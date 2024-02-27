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
  mRenderingDataImpl[0] = std::make_shared<RenderingDataImpl>();
  mRenderingDataImpl[1] = std::make_shared<RenderingDataImpl>();
  mRenderingData[0]     = mRenderingDataImpl[0];
  mRenderingData[1]     = mRenderingDataImpl[1];
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
  if(mEnableFixedCache)
  {
    if(mDecodedBuffers.size() < mTotalFrameNumber)
    {
      mDecodedBuffers.clear();
      mDecodedBuffers.resize(mTotalFrameNumber, std::make_pair<std::vector<uint8_t>, bool>(std::vector<uint8_t>(), false));
    }
  }

  if(!mCurrentRenderingData->mTbmQueue || !mVectorRenderer || !mCurrentRenderingData->mTargetSurface)
  {
    return false;
  }

  int canDequeue = tbm_surface_queue_can_dequeue(mCurrentRenderingData->mTbmQueue, 0);
  if(!canDequeue)
  {
    // Ignore the previous image which is inserted to the queue.
    mCurrentRenderingData->mTargetSurface->IgnoreSourceImage();

    // Check again
    canDequeue = tbm_surface_queue_can_dequeue(mCurrentRenderingData->mTbmQueue, 0);
    if(!canDequeue)
    {
      return false;
    }
  }

  tbm_surface_h tbmSurface;

  if(tbm_surface_queue_dequeue(mCurrentRenderingData->mTbmQueue, &tbmSurface) != TBM_SURFACE_QUEUE_ERROR_NONE)
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
    tbm_surface_queue_cancel_dequeue(mCurrentRenderingData->mTbmQueue, tbmSurface);
    return false;
  }

  unsigned char* buffer = info.planes[0].ptr;
  if(info.width != mCurrentRenderingData->mWidth || info.height != mCurrentRenderingData->mHeight || !buffer)
  {
    DALI_LOG_ERROR("VectorAnimationRendererTizen::Render: Invalid tbm surface! [%d, %d, %p] [%p]\n", info.width, info.height, buffer, this);
    tbm_surface_unmap(tbmSurface);
    tbm_surface_queue_cancel_dequeue(mCurrentRenderingData->mTbmQueue, tbmSurface);
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
    const int bufferSize = mCurrentRenderingData->mWidth * mCurrentRenderingData->mHeight * Dali::Pixel::GetBytesPerPixel(Dali::Pixel::RGBA8888);
    memcpy(buffer, &mDecodedBuffers[frameNumber].first[0], bufferSize);
  }
  else
  {
    if(!existing)
    {
      tbm_surface_internal_ref(tbmSurface);

      // Create Surface object
      surface = rlottie::Surface(reinterpret_cast<uint32_t*>(buffer), mCurrentRenderingData->mWidth, mCurrentRenderingData->mHeight, static_cast<size_t>(info.planes[0].stride));

      // Push the buffer
      mBuffers.push_back(SurfacePair(tbmSurface, surface));
    }

    if(mEnableFixedCache && (frameNumber < mDecodedBuffers.size()))
    {
      const uint32_t       bufferSize = mCurrentRenderingData->mWidth * mCurrentRenderingData->mHeight * Dali::Pixel::GetBytesPerPixel(Dali::Pixel::RGBA8888);
      std::vector<uint8_t> rasterizeBuffer(buffer, buffer + bufferSize);
      mDecodedBuffers[frameNumber].first  = std::move(rasterizeBuffer);
      mDecodedBuffers[frameNumber].second = true;
    }
  }

  // Render the frame
  mVectorRenderer->renderSync(frameNumber, surface);

  tbm_surface_unmap(tbmSurface);

  tbm_surface_queue_enqueue(mCurrentRenderingData->mTbmQueue, tbmSurface);

  if(!mResourceReady)
  {
    mPreviousTextures.push_back(mRenderedTexture); // It is used to destroy the object in the main thread.

    mRenderedTexture        = mCurrentRenderingData->mTexture;
    mResourceReady          = true;
    mResourceReadyTriggered = true;

    VectorAnimationPluginManager::Get().TriggerEvent(*this);

    DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "Resource ready [current = %d] [%p]\n", frameNumber, this);
  }

  return true;
}

void VectorAnimationRendererTizen::RenderStopped()
{
  if(mCurrentRenderingData->mTargetSurface)
  {
    // Animation is stopped. Free empty buffers
    mCurrentRenderingData->mTargetSurface->FreeReleasedBuffers();

    {
      Dali::Mutex::ScopedLock lock(mMutex);
      ResetBuffers();
    }
  }
}

void VectorAnimationRendererTizen::ResetBuffers()
{
  for(auto&& iter : mBuffers)
  {
    tbm_surface_internal_unref(iter.first);
  }
  mBuffers.clear();
}

void VectorAnimationRendererTizen::OnFinalize()
{
  mRenderedTexture.Reset();
  mPreviousTextures.clear();

  for(auto&& renderingData : mRenderingDataImpl)
  {
    renderingData->mTargetSurface = nullptr;
    renderingData->mTbmQueue      = nullptr;
  }
}

void VectorAnimationRendererTizen::OnNotify()
{
  // Reset the previous texture to destroy it in the main thread
  mPreviousTextures.clear();

  // If RenderingData is updated, then clear previous renderingData
  if(IsRenderingDataUpdated())
  {
    std::shared_ptr<RenderingDataImpl> oldRenderingData = mRenderingDataImpl[1u - mCurrentDataIndex];
    oldRenderingData->mTargetSurface                    = nullptr;
    oldRenderingData->mTbmQueue                         = nullptr;
    SetRenderingDataUpdated(false);
  }
}

void VectorAnimationRendererTizen::PrepareTarget(uint32_t updatedDataIndex)
{
  mRenderingDataImpl[updatedDataIndex]->mTargetSurface = NativeImageSourceQueue::New(mRenderingDataImpl[updatedDataIndex]->mWidth, mRenderingDataImpl[updatedDataIndex]->mHeight, NativeImageSourceQueue::ColorFormat::RGBA8888);
  mRenderingDataImpl[updatedDataIndex]->mTexture       = Texture::New(*mRenderingDataImpl[updatedDataIndex]->mTargetSurface);
}

void VectorAnimationRendererTizen::SetShader(uint32_t updatedDataIndex)
{
  if(mShaderChanged)
  {
    return;
  }

  Shader shader = mRenderer.GetShader();

  std::string fragmentShader;
  std::string vertexShader;

  // Get the current fragment shader source
  Property::Value program = shader.GetProperty(Shader::Property::PROGRAM);
  Property::Map*  map     = program.GetMap();
  if(map)
  {
    Property::Value* fragment = map->Find("fragment");
    if(fragment)
    {
      fragmentShader = fragment->Get<std::string>();
    }

    Property::Value* vertex = map->Find("vertex");
    if(vertex)
    {
      vertexShader = vertex->Get<std::string>();
    }
  }

  // Get custom fragment shader prefix
  mRenderingDataImpl[updatedDataIndex]->mTargetSurface->ApplyNativeFragmentShader(fragmentShader);

  // Set the modified shader again
  Shader newShader = Shader::New(vertexShader, fragmentShader);
  newShader.RegisterProperty(PIXEL_AREA_UNIFORM_NAME, FULL_TEXTURE_RECT);

  mRenderer.SetShader(newShader);

  mShaderChanged = true;
}

void VectorAnimationRendererTizen::OnSetSize(uint32_t updatedDataIndex)
{
  std::shared_ptr<RenderingDataImpl> renderingData = mRenderingDataImpl[updatedDataIndex];
  renderingData->mTbmQueue                         = AnyCast<tbm_surface_queue_h>(renderingData->mTargetSurface->GetNativeImageSourceQueue());
}

bool VectorAnimationRendererTizen::IsTargetPrepared()
{
  return (mCurrentRenderingData) ? !!mCurrentRenderingData->mTargetSurface : false;
}

bool VectorAnimationRendererTizen::IsRenderReady()
{
  return (mResourceReady && mRenderedTexture);
}

Dali::Texture VectorAnimationRendererTizen::GetTargetTexture()
{
  return mRenderedTexture;
}

} // namespace Plugin

} // namespace Dali
