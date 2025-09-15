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
#include "tizen-video-constraint-helper.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <tbm_surface_internal.h>
#include <chrono>

namespace Dali
{
namespace Plugin
{
VideoConstraintHelperPtr VideoConstraintHelper::New()
{
  VideoConstraintHelperPtr ptr = new VideoConstraintHelper();
  return ptr;
}

VideoConstraintHelper::VideoConstraintHelper()
: mPreviousFrameBufferNativeImageSourcePtr(nullptr),
  mCurrentFrameBufferNativeImageSourcePtr(nullptr),
  mPreviousFrameBufferSurface(nullptr),
  mCurrentFrameBufferSurface(nullptr),
  mIsFirstUpdate(true)
{
}

VideoConstraintHelper::~VideoConstraintHelper()
{
}

void VideoConstraintHelper::SetFrameInterpolationInterval(float intervalSeconds)
{
  Dali::Mutex::ScopedLock lock(mConstraintMutex);
  mInterpolationIntervalCandidate = intervalSeconds;
}

void VideoConstraintHelper::SetVideoFrameBufferNativeImageSource(Dali::NativeImageSourcePtr previousFrameBufferNativeImageSourcePtr, Dali::NativeImageSourcePtr currentFrameBufferNativeImageSourcePtr)
{
  mPreviousFrameBufferNativeImageSourcePtr = previousFrameBufferNativeImageSourcePtr;
  mCurrentFrameBufferNativeImageSourcePtr  = currentFrameBufferNativeImageSourcePtr;
}

void VideoConstraintHelper::SetVideoFrameBuffer(tbm_surface_h newVideoFrameBufferSurface)
{
  Dali::Mutex::ScopedLock lock(mConstraintMutex);
  if(!mPreviousFrameBufferNativeImageSourcePtr || !mCurrentFrameBufferNativeImageSourcePtr)
  {
    DALI_LOG_ERROR("NativeImageSources are not initialized.\n");
  }

  tbm_surface_h previousSurfaceTemp = mPreviousFrameBufferSurface;
  tbm_surface_h currentSurfaceTemp = mCurrentFrameBufferSurface;

  if(mIsFirstVideoFrame)
  {
    mPreviousFrameBufferSurface = newVideoFrameBufferSurface;
    mIsFirstVideoFrame          = false;
  }
  else
  {
    mPreviousFrameBufferSurface = mCurrentFrameBufferSurface;
  }
  mCurrentFrameBufferSurface = newVideoFrameBufferSurface;
  mInterpolationInterval = mInterpolationIntervalCandidate;

  if(mPreviousFrameBufferSurface != nullptr)
  {
    tbm_surface_internal_ref(mPreviousFrameBufferSurface);
  }

  if(mCurrentFrameBufferSurface != nullptr)
  {
    tbm_surface_internal_ref(mCurrentFrameBufferSurface);
  }

  if(previousSurfaceTemp != nullptr)
  {
    tbm_surface_internal_unref(previousSurfaceTemp);
  }

  if(currentSurfaceTemp != nullptr)
  {
    tbm_surface_internal_unref(currentSurfaceTemp);
  }
  mIsFrameReady = true;
}

void VideoConstraintHelper::ResetFirstFrameFlag()
{
  Dali::Mutex::ScopedLock lock(mConstraintMutex);
  mInterpolationFactor = 0.0f;
  mIsFirstVideoFrame   = true;
  mIsFirstUpdate       = true; // Reset timestep tracking as well
}

bool VideoConstraintHelper::UpdateVideoFrameBuffer()
{
  Dali::Mutex::ScopedLock lock(mConstraintMutex);
  if(!mIsFrameReady)
  {
    return false;
  }

  if(mPreviousFrameBufferNativeImageSourcePtr && mCurrentFrameBufferNativeImageSourcePtr)
  {
    mPreviousFrameBufferNativeImageSourcePtr->SetSource(mPreviousFrameBufferSurface);
    mCurrentFrameBufferNativeImageSourcePtr->SetSource(mCurrentFrameBufferSurface);
  }
  mInterpolationFactor = 0.0f;
  mIsFrameReady        = false;
  return true;
}

float VideoConstraintHelper::UpdateInterpolationFactor()
{
  Dali::Mutex::ScopedLock lock(mConstraintMutex);

  // Calculate real timestep
  float realTimeStep = 0.0f;
  auto  currentTime  = std::chrono::steady_clock::now();

  if(mIsFirstUpdate)
  {
    // First update, initialize timestamp
    mLastUpdateTime = currentTime;
    mIsFirstUpdate  = false;
    realTimeStep    = 0.0f; // No time elapsed on first call
  }
  else
  {
    // Calculate elapsed time in seconds
    auto elapsedTime = currentTime - mLastUpdateTime;
    realTimeStep     = std::chrono::duration<float>(elapsedTime).count();
  }

  // Update last timestamp
  mLastUpdateTime = currentTime;

  mInterpolationFactor += (realTimeStep / mInterpolationInterval);
  mInterpolationFactor = std::min(mInterpolationFactor, 1.0f);
  return mInterpolationFactor;
}

} // namespace Plugin
} // namespace Dali
