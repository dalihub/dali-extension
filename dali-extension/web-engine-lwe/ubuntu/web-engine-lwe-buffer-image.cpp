/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "web-engine-lwe-buffer-image.h"

#include <GLES2/gl2.h>
#include <utility>

namespace DALI_NAMESPACE
{
namespace Plugin
{
WebEngineLweBufferImage::WebEngineLweBufferImage(uint32_t width, uint32_t height)
: mWidth(width),
  mHeight(height)
{
}

void WebEngineLweBufferImage::setFrame(std::vector<uint8_t>&& pixels)
{
  if(pixels.size() != static_cast<size_t>(mWidth) * mHeight * 4u)
  {
    return;
  }
  std::lock_guard<std::mutex> lock(mMutex);
  mPixels = std::move(pixels);
  mChanged = true;
}

bool WebEngineLweBufferImage::CreateResource()
{
  std::lock_guard<std::mutex> lock(mMutex);
  mTexture = 0u;
  mChanged = true;
  return true;
}

void WebEngineLweBufferImage::DestroyResource()
{
  std::lock_guard<std::mutex> lock(mMutex);
  mTexture = 0u;
  mChanged = true;
}

uint32_t WebEngineLweBufferImage::TargetTexture()
{
  // DALi has bound its texture here. It owns the texture name and its lifetime.
  // Keep the latest CPU frame for resource recreation after context loss.
  std::lock_guard<std::mutex> lock(mMutex);
  GLint texture = 0;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture);
  const bool allocated = mTexture == static_cast<uint32_t>(texture) && texture != 0;
  if((!mChanged && allocated) || mPixels.empty())
  {
    return GL_NO_ERROR;
  }

  GLint alignment = 4;
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  if(allocated)
  {
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mWidth, mHeight, GL_RGBA, GL_UNSIGNED_BYTE, mPixels.data());
  }
  else
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, mPixels.data());
  }
  const GLenum error = glGetError();
  glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
  if(error == GL_NO_ERROR)
  {
    mTexture = static_cast<uint32_t>(texture);
    mChanged = false;
  }
  return error;
}

Dali::NativeImageInterface::PrepareTextureResult WebEngineLweBufferImage::PrepareTexture()
{
  std::lock_guard<std::mutex> lock(mMutex);
  return mChanged ? IMAGE_CHANGED : NO_ERROR;
}

uint32_t WebEngineLweBufferImage::GetWidth() const
{
  return mWidth;
}

uint32_t WebEngineLweBufferImage::GetHeight() const
{
  return mHeight;
}

bool WebEngineLweBufferImage::RequiresBlending() const
{
  return true;
}

int WebEngineLweBufferImage::GetTextureTarget() const
{
  return GL_TEXTURE_2D;
}

bool WebEngineLweBufferImage::ApplyNativeFragmentShader(Dali::String&, int)
{
  return false;
}

const char* WebEngineLweBufferImage::GetCustomSamplerTypename() const
{
  return nullptr;
}

Dali::Any WebEngineLweBufferImage::GetNativeImageHandle() const
{
  return {};
}

bool WebEngineLweBufferImage::SourceChanged() const
{
  std::lock_guard<std::mutex> lock(mMutex);
  return mChanged;
}

Dali::Rect<uint32_t> WebEngineLweBufferImage::GetUpdatedArea()
{
  return {0u, 0u, mWidth, mHeight};
}

void WebEngineLweBufferImage::PostRender()
{
}
} // namespace Plugin
} // namespace DALI_NAMESPACE
