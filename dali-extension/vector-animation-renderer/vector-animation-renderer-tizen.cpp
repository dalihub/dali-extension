/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#if defined(DEBUG_ENABLED)
Debug::Filter* gVectorAnimationLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_VECTOR_ANIMATION");
#endif
} // unnamed namespace

class VectorAnimationRendererTizen::RenderingDataImpl : public VectorAnimationRenderer::RenderingData
{
public:
  NativeImageSourceQueuePtr mTargetSurface;
};

VectorAnimationRendererTizen::VectorAnimationRendererTizen()
: mRenderedTexture(),
  mPreviousTextures()
{
}

VectorAnimationRendererTizen::~VectorAnimationRendererTizen()
{
}

// Called by VectorAnimationTaskThread
bool VectorAnimationRendererTizen::Render(uint32_t frameNumber)
{
  std::shared_ptr<RenderingDataImpl> renderingDataImpl;

  bool resourceChanged = false;
  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    if(DALI_LIKELY(!mFinalized))
    {
      if(mPreparedRenderingData)
      {
        mPreviousRenderingData.push_back(mCurrentRenderingData);
        mCurrentRenderingData = std::move(mPreparedRenderingData);
        mPreparedRenderingData.reset();
        resourceChanged = true;
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

  if(resourceChanged)
  {
    mResourceReady = false;
  }

  if(mEnableFixedCache)
  {
    if(resourceChanged || mDecodedBuffers.size() < mTotalFrameNumber)
    {
      mDecodedBuffers.clear();
      mDecodedBuffers.resize(mTotalFrameNumber, std::make_pair<std::vector<uint8_t>, bool>(std::vector<uint8_t>(), false));
    }
  }

  if(!mVectorRenderer || !renderingDataImpl->mTargetSurface)
  {
    return false;
  }

  int canDequeue = renderingDataImpl->mTargetSurface->CanDequeueBuffer();
  if(!canDequeue)
  {
    // Ignore the previous image which is inserted to the queue.
    renderingDataImpl->mTargetSurface->IgnoreSourceImage();

    // Check again
    canDequeue = renderingDataImpl->mTargetSurface->CanDequeueBuffer();
    if(!canDequeue)
    {
      return false;
    }
  }

  NativeImageSourceQueue::BufferAccessType type;
  if(mEnableFixedCache && (frameNumber < mDecodedBuffers.size()) && (!mDecodedBuffers[frameNumber].second))
  {
    type = NativeImageSourceQueue::BufferAccessType::READ | NativeImageSourceQueue::BufferAccessType::WRITE;
  }
  else
  {
    type = NativeImageSourceQueue::BufferAccessType::WRITE;
  }

  uint32_t width, height, stride;

  uint8_t* buffer = renderingDataImpl->mTargetSurface->DequeueBuffer(width, height, stride, type);
  if(!buffer)
  {
    DALI_LOG_ERROR("DequeueBuffer failed [%p]\n", this);
    return false;
  }

  if(width != renderingDataImpl->mWidth || height != renderingDataImpl->mHeight || !buffer)
  {
    DALI_LOG_ERROR("VectorAnimationRendererTizen::Render: Invalid buffer! [%d, %d, %p] [%p]\n", width, height, buffer, this);
    renderingDataImpl->mTargetSurface->CancelDequeuedBuffer(buffer);
    return false;
  }

  if(mEnableFixedCache && (frameNumber < mDecodedBuffers.size()) && mDecodedBuffers[frameNumber].second)
  {
    const size_t bufferSize = mDecodedBuffers[frameNumber].first.size();
    memcpy(buffer, &mDecodedBuffers[frameNumber].first[0], bufferSize);
  }
  else
  {
    // Create Surface object
    rlottie::Surface surface = rlottie::Surface(reinterpret_cast<uint32_t*>(buffer), renderingDataImpl->mWidth, renderingDataImpl->mHeight, static_cast<size_t>(stride));

    // Render the frame
    mVectorRenderer->renderSync(frameNumber, surface);

    if(mEnableFixedCache && (frameNumber < mDecodedBuffers.size()))
    {
      const size_t         bufferSize = renderingDataImpl->mHeight * static_cast<size_t>(stride);
      std::vector<uint8_t> rasterizeBuffer(buffer, buffer + bufferSize);
      mDecodedBuffers[frameNumber].first  = std::move(rasterizeBuffer);
      mDecodedBuffers[frameNumber].second = true;
    }
  }

  renderingDataImpl->mTargetSurface->EnqueueBuffer(buffer);

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
  }
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

  renderingDataImpl->mTargetSurface->SetQueueUsageHint(Dali::NativeImageSourceQueue::QueueUsageType::ENQUEUE_DEQUEUE);
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
