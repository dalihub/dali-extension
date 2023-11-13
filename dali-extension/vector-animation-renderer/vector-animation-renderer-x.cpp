/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include <dali-extension/vector-animation-renderer/vector-animation-renderer-x.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/hash.h>
#include <dali/devel-api/rendering/texture-devel.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/property-array.h>

#include <cstring> // for strlen()

// INTERNAL INCLUDES
#include <dali-extension/vector-animation-renderer/vector-animation-plugin-manager.h>

// The plugin factories
extern "C" DALI_EXPORT_API Dali::VectorAnimationRendererPlugin* CreateVectorAnimationRendererPlugin(void)
{
  return new Dali::Plugin::VectorAnimationRendererX;
}

namespace Dali
{
namespace Plugin
{
namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gVectorAnimationLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_VECTOR_ANIMATION");
#endif
} // unnamed namespace

VectorAnimationRendererX::VectorAnimationRendererX()
: mLottieSurface(),
  mRenderCallback(std::unique_ptr<EventThreadCallback>(new EventThreadCallback(MakeCallback(this, &VectorAnimationRendererX::OnLottieRendered))))
{
}

VectorAnimationRendererX::~VectorAnimationRendererX()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  ResetBuffers();
  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "this = %p\n", this);
}

bool VectorAnimationRendererX::Render(uint32_t frameNumber)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(!mVectorRenderer || !mPixelBuffer)
  {
    return false;
  }

  mVectorRenderer->renderSync(frameNumber, mLottieSurface);
  mRenderCallback->Trigger();

  if(!mResourceReady)
  {
    mResourceReady          = true;
    mResourceReadyTriggered = true;

    VectorAnimationPluginManager::Get().TriggerEvent(*this);
  }

  return true;
}

void VectorAnimationRendererX::RenderStopped()
{
}


void VectorAnimationRendererX::SetShader()
{
  if(mShaderChanged || !mTexture)
  {
    return;
  }

  // Not implemented

  mShaderChanged = true;
}

void VectorAnimationRendererX::ResetBuffers()
{
  if(mPixelBuffer)
  {
    uint32_t bufferSize = mPixelBuffer.GetWidth() * mPixelBuffer.GetHeight() * Dali::Pixel::GetBytesPerPixel(mPixelBuffer.GetPixelFormat());
    memset(mPixelBuffer.GetBuffer(), 0, bufferSize);
  }
}

void VectorAnimationRendererX::OnFinalize()
{
}

void VectorAnimationRendererX::OnLottieRendered()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mPixelBuffer && mTexture)
  {
    PixelData pixelData = mPixelBuffer.CreatePixelData();
    mTexture.Upload(pixelData);
  }
}

void VectorAnimationRendererX::PrepareTarget()
{
  mTexture = Texture::New(Dali::TextureType::TEXTURE_2D, Dali::Pixel::BGRA8888, mWidth, mHeight);

  mPixelBuffer   = Dali::Devel::PixelBuffer::New(mWidth, mHeight, Dali::Pixel::BGRA8888);
  mLottieSurface = rlottie::Surface(reinterpret_cast<uint32_t*>(mPixelBuffer.GetBuffer()), mWidth, mHeight, static_cast<size_t>(mPixelBuffer.GetStride() * 4));
}

bool VectorAnimationRendererX::IsTargetPrepared()
{
  return !!mPixelBuffer;
}

bool VectorAnimationRendererX::IsRenderReady()
{
  return mResourceReady && mTexture;
}

Dali::Texture VectorAnimationRendererX::GetTargetTexture()
{
  return mTexture;
}

} // namespace Plugin

} // namespace Dali
