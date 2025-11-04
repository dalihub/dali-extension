#ifndef __DALI_TIZEN_VIDEO_CONSTRAINT_HELPER_H__
#define __DALI_TIZEN_VIDEO_CONSTRAINT_HELPER_H__

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

// INTERNAL INCLUDES

// EXTERNAL INCLUDES
#include <dali/devel-api/threading/mutex.h>
#include <dali/public-api/adaptor-framework/native-image-source.h>
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/object/ref-object.h>
#include <tbm_surface.h>
#include <chrono>

namespace Dali
{
namespace Plugin
{
class VideoConstraintHelper;
typedef Dali::IntrusivePtr<VideoConstraintHelper> VideoConstraintHelperPtr;

class VideoConstraintHelper : public Dali::RefObject
{
public:
  /**
   * @brief Creates a new VideoConstraintHelper instance.
   * @return A new VideoConstraintHelper pointer
   */
  static VideoConstraintHelperPtr New();

  /**
   * @brief Sets the frame interpolation interval for smooth video playback.
   *
   * The interpolation factor will progress from 0.0 to 1.0 over this duration.
   * This interval is applied after the next call to SetVideoFrameBuffer.
   *
   * @param[in] intervalSeconds The interpolation interval in seconds
   */
  void SetFrameInterpolationInterval(float intervalSeconds);

  /**
   * @brief Sets the native image sources for video frame buffers.
   *
   * This method sets the native image sources that will be used as frame buffers
   * for video interpolation between previous and current frames.
   *
   * @param[in] previousFrameBufferNativeImageSourcePtr Native image source for previous frame buffer
   * @param[in] currentFrameBufferNativeImageSourcePtr Native image source for current frame buffer
   */
  void SetVideoFrameBufferNativeImageSource(Dali::NativeImageSourcePtr previousFrameBufferNativeImageSourcePtr, Dali::NativeImageSourcePtr currentFrameBufferNativeImageSourcePtr);

  /**
   * @brief Sets the video frame buffer surface for rendering.
   *
   * This method sets the TBM surface that contains the current video frame data
   * for video rendering and interpolation.
   *
   * @param[in] newVideoFrameBufferSurface The TBM surface containing video frame data
   */
  void SetVideoFrameBuffer(tbm_surface_h newVideoFrameBufferSurface);

  /**
   * @brief Resets the first frame flag to indicate a new video sequence.
   *
   * This method should be called when starting a new video or when the video
   * playback is restarted to reset the interpolation state.
   */
  void ResetFirstFrameFlag();

  /**
   * @brief Updates the video frame buffer for rendering.
   *
   * This method updates the video frame buffer with the latest frame data
   * and prepares it for rendering with interpolation.
   *
   * @return True if the video frame buffer was successfully updated, false otherwise
   */
  bool UpdateVideoFrameBuffer();

  /**
   * @brief Updates the interpolation factor using real timestep calculation.
   *
   * This method calculates the actual time elapsed since the last update and
   * uses it to progress the interpolation factor from 0.0 to 1.0 over the
   * configured interpolation interval.
   *
   * @return The current interpolation factor (0.0 to 1.0)
   */
  float UpdateInterpolationFactor();

private:
  VideoConstraintHelper();

  virtual ~VideoConstraintHelper();

private:
  Dali::Mutex mConstraintMutex;

  Dali::NativeImageSourcePtr mPreviousFrameBufferNativeImageSourcePtr;
  Dali::NativeImageSourcePtr mCurrentFrameBufferNativeImageSourcePtr;
  tbm_surface_h              mPreviousFrameBufferSurface;
  tbm_surface_h              mCurrentFrameBufferSurface;

  float mInterpolationIntervalCandidate{0.0f};
  float mInterpolationInterval{0.0f};
  float mInterpolationFactor{0.0f};
  bool  mIsFirstVideoFrame{true};
  bool  mIsFrameReady{false};

  // For real timestep calculation
  std::chrono::steady_clock::time_point mLastUpdateTime;
  bool                                  mIsFirstUpdate{true};
};

} // namespace Plugin
} // namespace Dali

#endif // __DALI_TIZEN_VIDEO_CONSTRAINT_HELPER_H__
