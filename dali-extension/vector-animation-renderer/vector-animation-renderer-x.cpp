/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
: mUploadPixelBufferRequired(false)
{
}

VectorAnimationRendererX::~VectorAnimationRendererX()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  ResetBuffers();
  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "this = %p\n", this);
}

// Called by VectorAnimationTaskThread
bool VectorAnimationRendererX::Render(uint32_t frameNumber)
{
  std::shared_ptr<RenderingDataImpl> renderingDataImpl;
  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    if(DALI_LIKELY(!mFinalized))
    {
      if(mPreparedRenderingData)
      {
        mPreviousRenderingData.push_back(mCurrentRenderingData);
        mCurrentRenderingData = mPreparedRenderingData;
        mPreparedRenderingData.reset();
        mResourceReady = false;
      }
      renderingDataImpl = std::static_pointer_cast<RenderingDataImpl>(mCurrentRenderingData);
    }
  }

  if(!renderingDataImpl)
  {
    return false;
  }

  Dali::Mutex::ScopedLock lock(mMutex);
  if(!mVectorRenderer || !renderingDataImpl->mPixelBuffer)
  {
    return false;
  }

  mVectorRenderer->renderSync(frameNumber, renderingDataImpl->mLottieSurface);

  mUploadPixelBufferRequired = true;

  if(!mResourceReady)
  {
    mResourceReady          = true;
    mResourceReadyTriggered = true;
  }

  VectorAnimationPluginManager::Get().TriggerEvent(*this);

  return true;
}

void VectorAnimationRendererX::RenderStopped()
{
}

// This Method is called inside mMutex
void VectorAnimationRendererX::ResetBuffers()
{
}

// This Method is called inside mMutex
void VectorAnimationRendererX::OnFinalize()
{
}

// This Method is called inside mMutex
void VectorAnimationRendererX::OnNotify()
{
  // Skip Upload operation if we don't need.
  if(!mUploadPixelBufferRequired)
  {
    return;
  }

  std::shared_ptr<RenderingDataImpl> renderingDataImpl;
  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    if(DALI_LIKELY(!mFinalized))
    {
      renderingDataImpl = std::static_pointer_cast<RenderingDataImpl>(mCurrentRenderingData);
    }
  }

  if(renderingDataImpl && renderingDataImpl->mPixelBuffer && renderingDataImpl->mTexture)
  {
    PixelData pixelData = renderingDataImpl->mPixelBuffer.CreatePixelData();
    renderingDataImpl->mTexture.Upload(pixelData);

    mUploadPixelBufferRequired = false;
  }
}

// This Method is called inside mRenderingDataMutex
void VectorAnimationRendererX::PrepareTarget(std::shared_ptr<RenderingData> renderingData)
{
  std::shared_ptr<RenderingDataImpl> renderingDataImpl = std::static_pointer_cast<RenderingDataImpl>(renderingData);
  renderingDataImpl->mTexture                          = Texture::New(Dali::TextureType::TEXTURE_2D, Dali::Pixel::BGRA8888, renderingDataImpl->mWidth, renderingDataImpl->mHeight);
  renderingDataImpl->mPixelBuffer                      = Dali::Devel::PixelBuffer::New(renderingDataImpl->mWidth, renderingDataImpl->mHeight, Dali::Pixel::BGRA8888);
  renderingDataImpl->mLottieSurface                    = rlottie::Surface(reinterpret_cast<uint32_t*>(renderingDataImpl->mPixelBuffer.GetBuffer()), renderingDataImpl->mWidth, renderingDataImpl->mHeight, static_cast<size_t>(renderingDataImpl->mPixelBuffer.GetStride() * 4));
}

bool VectorAnimationRendererX::IsTargetPrepared()
{
  std::shared_ptr<RenderingDataImpl> renderingDataImpl;
  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    if(DALI_LIKELY(!mFinalized))
    {
      renderingDataImpl = std::static_pointer_cast<RenderingDataImpl>(mPreparedRenderingData ? mPreparedRenderingData : mCurrentRenderingData);
    }
  }
  return (renderingDataImpl) ? !!renderingDataImpl->mPixelBuffer : false;
}

// This Method is called inside mMutex
bool VectorAnimationRendererX::IsRenderReady()
{
  std::shared_ptr<RenderingDataImpl> renderingDataImpl;
  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    if(DALI_LIKELY(!mFinalized))
    {
      renderingDataImpl = std::static_pointer_cast<RenderingDataImpl>(mCurrentRenderingData);
    }
  }
  return mResourceReady && ((renderingDataImpl) ? !!renderingDataImpl->mTexture : false);
}

// This Method is called inside mMutex
Dali::Texture VectorAnimationRendererX::GetTargetTexture()
{
  std::shared_ptr<RenderingDataImpl> renderingDataImpl;
  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    if(DALI_LIKELY(!mFinalized))
    {
      renderingDataImpl = std::static_pointer_cast<RenderingDataImpl>(mCurrentRenderingData);
    }
  }
  return (renderingDataImpl) ? renderingDataImpl->mTexture : Texture();
}

// This Method is called inside mRenderingDataMutex
std::shared_ptr<VectorAnimationRenderer::RenderingData> VectorAnimationRendererX::CreateRenderingData()
{
  return std::make_shared<RenderingDataImpl>();
}

} // namespace Plugin

} // namespace Dali
