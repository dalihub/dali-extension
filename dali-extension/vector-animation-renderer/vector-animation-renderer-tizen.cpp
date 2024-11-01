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
#include <dali-extension/vector-animation-renderer/vector-animation-renderer-tizen.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>
#include <dali/devel-api/common/hash.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/property-array.h>
#include <tbm_surface_internal.h>
#include <cstring> // for strlen()

// INTERNAL INCLUDES
#include <dali-extension/vector-animation-renderer/vector-animation-plugin-manager.h>

// The plugin factories
extern "C" DALI_EXPORT_API Dali::VectorAnimationRendererPlugin* CreateVectorAnimationRendererPlugin(void)
{
  return new Dali::Plugin::VectorAnimationRendererTizen;
}

namespace Dali
{
namespace Plugin
{
namespace
{
const char* const PIXEL_AREA_UNIFORM_NAME("pixelArea");
const Vector4     FULL_TEXTURE_RECT(0.f, 0.f, 1.f, 1.f);

#if defined(DEBUG_ENABLED)
Debug::Filter* gVectorAnimationLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_VECTOR_ANIMATION");
#endif
} // unnamed namespace

class VectorAnimationRendererTizen::RenderingDataImpl : public VectorAnimationRenderer::RenderingData
{
public:
  NativeImageSourceQueuePtr mTargetSurface;
  tbm_surface_queue_h       mTbmQueue;
};

VectorAnimationRendererTizen::VectorAnimationRendererTizen()
: mBuffers(),
  mRenderedTexture(),
  mPreviousTextures()
{
}

VectorAnimationRendererTizen::~VectorAnimationRendererTizen()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  ResetBuffers();
  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "this = %p\n", this);
}

// Called by VectorAnimationTaskThread
bool VectorAnimationRendererTizen::Render(uint32_t frameNumber)
{
  std::shared_ptr<RenderingDataImpl> renderingDataImpl;
  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    if(DALI_LIKELY(!mFinalized))
    {
      if(mPreparedRenderingData)
      {
        mPreviousRenderingData.push_back(mCurrentRenderingData);
        mCurrentRenderingData = std::move(mPreparedRenderingData);
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
  if(DALI_UNLIKELY(mFinalized))
  {
    // Fast-out if finalized.
    return false;
  }

  if(mEnableFixedCache)
  {
    if(mDecodedBuffers.size() < mTotalFrameNumber)
    {
      mDecodedBuffers.clear();
      mDecodedBuffers.resize(mTotalFrameNumber, std::make_pair<std::vector<uint8_t>, bool>(std::vector<uint8_t>(), false));
    }
  }

  if(!renderingDataImpl->mTbmQueue || !mVectorRenderer || !renderingDataImpl->mTargetSurface)
  {
    return false;
  }

  int canDequeue = tbm_surface_queue_can_dequeue(renderingDataImpl->mTbmQueue, 0);
  if(!canDequeue)
  {
    // Ignore the previous image which is inserted to the queue.
    renderingDataImpl->mTargetSurface->IgnoreSourceImage();

    // Check again
    canDequeue = tbm_surface_queue_can_dequeue(renderingDataImpl->mTbmQueue, 0);
    if(!canDequeue)
    {
      return false;
    }
  }

  tbm_surface_h tbmSurface;

  if(tbm_surface_queue_dequeue(renderingDataImpl->mTbmQueue, &tbmSurface) != TBM_SURFACE_QUEUE_ERROR_NONE)
  {
    DALI_LOG_ERROR("Failed to dequeue a tbm_surface [%p]\n", this);
    return false;
  }

  tbm_surface_info_s info;
  int                ret = TBM_SURFACE_ERROR_NONE;

  if(mEnableFixedCache && (frameNumber < mDecodedBuffers.size()) && (!mDecodedBuffers[frameNumber].second))
  {
    ret = tbm_surface_map(tbmSurface, TBM_SURF_OPTION_READ | TBM_SURF_OPTION_WRITE, &info);
  }
  else
  {
    ret = tbm_surface_map(tbmSurface, TBM_SURF_OPTION_WRITE, &info);
  }
  if(ret != TBM_SURFACE_ERROR_NONE)
  {
    DALI_LOG_ERROR("VectorAnimationRendererTizen::Render: tbm_surface_map is failed! [%d] [%p]\n", ret, this);
    tbm_surface_queue_cancel_dequeue(renderingDataImpl->mTbmQueue, tbmSurface);
    return false;
  }

  unsigned char* buffer = info.planes[0].ptr;
  if(info.width != renderingDataImpl->mWidth || info.height != renderingDataImpl->mHeight || !buffer)
  {
    DALI_LOG_ERROR("VectorAnimationRendererTizen::Render: Invalid tbm surface! [%d, %d, %p] [%p]\n", info.width, info.height, buffer, this);
    tbm_surface_unmap(tbmSurface);
    tbm_surface_queue_cancel_dequeue(renderingDataImpl->mTbmQueue, tbmSurface);
    return false;
  }

  rlottie::Surface surface;
  bool             existing = false;

  if(!mResourceReady)
  {
    // Need to reset buffer list
    ResetBuffers();
  }
  else
  {
    for(auto&& iter : mBuffers)
    {
      if(iter.first == tbmSurface)
      {
        // Find the buffer in the existing list
        existing = true;
        surface  = iter.second;
        break;
      }
    }
  }

  if(mEnableFixedCache && (frameNumber < mDecodedBuffers.size()) && mDecodedBuffers[frameNumber].second)
  {
    const int bufferSize = renderingDataImpl->mWidth * renderingDataImpl->mHeight * Dali::Pixel::GetBytesPerPixel(Dali::Pixel::RGBA8888);
    memcpy(buffer, &mDecodedBuffers[frameNumber].first[0], bufferSize);
  }
  else
  {
    if(!existing)
    {
      tbm_surface_internal_ref(tbmSurface);

      // Create Surface object
      surface = rlottie::Surface(reinterpret_cast<uint32_t*>(buffer), renderingDataImpl->mWidth, renderingDataImpl->mHeight, static_cast<size_t>(info.planes[0].stride));

      // Push the buffer
      mBuffers.push_back(SurfacePair(tbmSurface, surface));
    }

    if(mEnableFixedCache && (frameNumber < mDecodedBuffers.size()))
    {
      const uint32_t       bufferSize = renderingDataImpl->mWidth * renderingDataImpl->mHeight * Dali::Pixel::GetBytesPerPixel(Dali::Pixel::RGBA8888);
      std::vector<uint8_t> rasterizeBuffer(buffer, buffer + bufferSize);
      mDecodedBuffers[frameNumber].first  = std::move(rasterizeBuffer);
      mDecodedBuffers[frameNumber].second = true;
    }
  }

  // Render the frame
  mVectorRenderer->renderSync(frameNumber, surface);

  tbm_surface_unmap(tbmSurface);

  tbm_surface_queue_enqueue(renderingDataImpl->mTbmQueue, tbmSurface);

  if(!mResourceReady)
  {
    // Only move the ownership of the texture to the renderer when it is valid.
    if(renderingDataImpl->mTexture && mRenderedTexture != renderingDataImpl->mTexture)
    {
      mPreviousTextures.push_back(mRenderedTexture); // It is used to destroy the object in the main thread.

      mRenderedTexture = std::move(renderingDataImpl->mTexture);
    }

    // Don't need to keep reference of texture.
    renderingDataImpl->mTexture.Reset();

    mResourceReady          = true;
    mResourceReadyTriggered = true;

    VectorAnimationPluginManager::Get().TriggerEvent(*this);

    DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "Resource ready [current = %d] [%p]\n", frameNumber, this);
  }

  return true;
}

void VectorAnimationRendererTizen::RenderStopped()
{
  std::shared_ptr<RenderingDataImpl> renderingDataImpl;
  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    if(DALI_LIKELY(!mFinalized))
    {
      renderingDataImpl = std::static_pointer_cast<RenderingDataImpl>(mCurrentRenderingData);
    }
  }

  if(renderingDataImpl && renderingDataImpl->mTargetSurface)
  {
    // Animation is stopped. Free empty buffers
    renderingDataImpl->mTargetSurface->FreeReleasedBuffers();

    {
      Dali::Mutex::ScopedLock lock(mMutex);
      ResetBuffers();
    }
  }
}

// This Method is called inside mMutex
void VectorAnimationRendererTizen::ResetBuffers()
{
  for(auto&& iter : mBuffers)
  {
    tbm_surface_internal_unref(iter.first);
  }
  mBuffers.clear();
}

// This Method is called inside mMutex
void VectorAnimationRendererTizen::OnFinalize()
{
  mRenderedTexture.Reset();
  mPreviousTextures.clear();
}

// This Method is called inside mMutex
void VectorAnimationRendererTizen::OnNotify()
{
  // Reset the previous texture to destroy it in the main thread
  mPreviousTextures.clear();
}

// This Method is called inside mRenderingDataMutex
void VectorAnimationRendererTizen::PrepareTarget(std::shared_ptr<RenderingData> renderingData)
{
  std::shared_ptr<RenderingDataImpl> renderingDataImpl = std::static_pointer_cast<RenderingDataImpl>(renderingData);
  renderingDataImpl->mTargetSurface                    = NativeImageSourceQueue::New(renderingDataImpl->mWidth, renderingDataImpl->mHeight, NativeImageSourceQueue::ColorFormat::BGRA8888);
  renderingDataImpl->mTexture                          = Texture::New(*renderingDataImpl->mTargetSurface);
}

// This Method is called inside mRenderingDataMutex
void VectorAnimationRendererTizen::OnSetSize(std::shared_ptr<RenderingData> renderingData)
{
  std::shared_ptr<RenderingDataImpl> renderingDataImpl = std::static_pointer_cast<RenderingDataImpl>(renderingData);
  renderingDataImpl->mTbmQueue                         = AnyCast<tbm_surface_queue_h>(renderingDataImpl->mTargetSurface->GetNativeImageSourceQueue());
}

bool VectorAnimationRendererTizen::IsTargetPrepared()
{
  std::shared_ptr<RenderingDataImpl> renderingDataImpl;
  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    if(DALI_LIKELY(!mFinalized))
    {
      renderingDataImpl = std::static_pointer_cast<RenderingDataImpl>(mPreparedRenderingData ? mPreparedRenderingData : mCurrentRenderingData);
    }
  }
  return (renderingDataImpl) ? !!renderingDataImpl->mTargetSurface : false;
}

// This Method is called inside mMutex
bool VectorAnimationRendererTizen::IsRenderReady()
{
  return (mResourceReady && mRenderedTexture);
}

// This Method is called inside mMutex
Dali::Texture VectorAnimationRendererTizen::GetTargetTexture()
{
  return mRenderedTexture;
}

// This Method is called inside mRenderingDataMutex
std::shared_ptr<VectorAnimationRenderer::RenderingData> VectorAnimationRendererTizen::CreateRenderingData()
{
  return std::make_shared<RenderingDataImpl>();
}

} // namespace Plugin

} // namespace Dali
