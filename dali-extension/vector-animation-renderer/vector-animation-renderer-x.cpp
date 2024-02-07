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

class VectorAnimationRendererX::RenderingDataImpl : public VectorAnimationRenderer::RenderingData
{
public:
  rlottie::Surface         mLottieSurface;
  Dali::Devel::PixelBuffer mPixelBuffer;
};

VectorAnimationRendererX::VectorAnimationRendererX()
: mRenderCallback(std::unique_ptr<EventThreadCallback>(new EventThreadCallback(MakeCallback(this, &VectorAnimationRendererX::OnLottieRendered))))
{
  mRenderingDataImpl[0] = std::make_shared<RenderingDataImpl>();
  mRenderingDataImpl[1] = std::make_shared<RenderingDataImpl>();
  mRenderingData[0] = mRenderingDataImpl[0];
  mRenderingData[1] = mRenderingDataImpl[1];
}

VectorAnimationRendererX::~VectorAnimationRendererX()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  ResetBuffers();
  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "this = %p\n", this);
}

bool VectorAnimationRendererX::Render(uint32_t frameNumber)
{
  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    mCurrentRenderingData = mRenderingDataImpl[mCurrentDataIndex];
    if(IsRenderingDataUpdated())
    {
      mResourceReady = false;
      mIsDataActivated = true;
    }
  }

  Dali::Mutex::ScopedLock lock(mMutex);
  if(!mVectorRenderer || !mCurrentRenderingData->mPixelBuffer)
  {
    return false;
  }

  mVectorRenderer->renderSync(frameNumber, mCurrentRenderingData->mLottieSurface);
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

void VectorAnimationRendererX::ResetBuffers()
{
  for(auto&& renderingData : mRenderingDataImpl)
  {
    if(renderingData->mPixelBuffer)
    {
      uint32_t bufferSize = renderingData->mPixelBuffer.GetWidth() * renderingData->mPixelBuffer.GetHeight() * Dali::Pixel::GetBytesPerPixel(renderingData->mPixelBuffer.GetPixelFormat());
      memset(renderingData->mPixelBuffer.GetBuffer(), 0, bufferSize);
    }
  }
}

void VectorAnimationRendererX::OnFinalize()
{
}

void VectorAnimationRendererX::OnLottieRendered()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mCurrentRenderingData->mPixelBuffer && mCurrentRenderingData->mTexture)
  {
    PixelData pixelData = mCurrentRenderingData->mPixelBuffer.CreatePixelData();
    mCurrentRenderingData->mTexture.Upload(pixelData);
  }
}

void VectorAnimationRendererX::OnNotify()
{
  // If RenderingData is updated, then clear previous renderingData
  if(IsRenderingDataUpdated())
  {
    std::shared_ptr<RenderingDataImpl> oldRenderingData = mRenderingDataImpl[1u - mCurrentDataIndex];
    SetRenderingDataUpdated(false);
  }
}

void VectorAnimationRendererX::PrepareTarget(uint32_t updatedDataIndex)
{
  mRenderingDataImpl[updatedDataIndex]->mTexture = Texture::New(Dali::TextureType::TEXTURE_2D, Dali::Pixel::BGRA8888, mRenderingDataImpl[updatedDataIndex]->mWidth, mRenderingDataImpl[updatedDataIndex]->mHeight);

  mRenderingDataImpl[updatedDataIndex]->mPixelBuffer   = Dali::Devel::PixelBuffer::New(mRenderingDataImpl[updatedDataIndex]->mWidth, mRenderingDataImpl[updatedDataIndex]->mHeight, Dali::Pixel::BGRA8888);
  mRenderingDataImpl[updatedDataIndex]->mLottieSurface = rlottie::Surface(reinterpret_cast<uint32_t*>(mRenderingDataImpl[updatedDataIndex]->mPixelBuffer.GetBuffer()), mRenderingDataImpl[updatedDataIndex]->mWidth, mRenderingDataImpl[updatedDataIndex]->mHeight, static_cast<size_t>(mRenderingDataImpl[updatedDataIndex]->mPixelBuffer.GetStride() * 4));
}

void VectorAnimationRendererX::SetShader(uint32_t updatedDataIndex)
{
  std::shared_ptr<RenderingDataImpl> renderingData = mRenderingDataImpl[updatedDataIndex];
  if(mShaderChanged || !renderingData->mTexture)
  {
    return;
  }

  // Not implemented

  mShaderChanged = true;
}

bool VectorAnimationRendererX::IsTargetPrepared()
{
  return (mCurrentRenderingData) ? !!mCurrentRenderingData->mPixelBuffer : false;
}

bool VectorAnimationRendererX::IsRenderReady()
{
  return mResourceReady && ((mCurrentRenderingData) ? !!mCurrentRenderingData->mTexture : false);
}

Dali::Texture VectorAnimationRendererX::GetTargetTexture()
{
  return (mCurrentRenderingData) ? mCurrentRenderingData->mTexture : Texture();
}

} // namespace Plugin

} // namespace Dali
