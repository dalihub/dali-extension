#ifndef DALI_EXTENSION_INTERNAL_RIVE_ANIMATION_TASK_H
#define DALI_EXTENSION_INTERNAL_RIVE_ANIMATION_TASK_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
 */

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/event-thread-callback.h>
#include <dali/devel-api/adaptor-framework/vector-animation-renderer.h>
#include <dali/devel-api/threading/conditional-wait.h>
#include <dali/public-api/object/property-array.h>
#include <chrono>
#include <memory>

// INTERNAL INCLUDES
#include <dali-extension/devel-api/rive-animation-view/rive-animation-view.h>

namespace Dali
{
namespace Extension
{
namespace Internal
{
class RiveAnimationThread;
class RiveAnimationTask;
using RiveAnimationTaskPtr = IntrusivePtr<RiveAnimationTask>;

/**
 * The task of the rive animation.
 */
class RiveAnimationTask : public RefObject
{
public:
  using UploadCompletedSignalType = Dali::VectorAnimationRenderer::UploadCompletedSignalType;

  using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

  /**
   * Flags for re-sending data to the rive animation thread
   */
  enum ResendFlags
  {
    RESEND_PLAY_RANGE    = 1 << 0,
    RESEND_LOOP_COUNT    = 1 << 1,
    RESEND_STOP_BEHAVIOR = 1 << 2,
    RESEND_LOOPING_MODE  = 1 << 3,
    RESEND_CURRENT_FRAME = 1 << 4,
    RESEND_SIZE          = 1 << 5,
    RESEND_PLAY_STATE    = 1 << 6
  };

  /**
   * @brief Structure used to pass parameters to the rive animation task
   */
  struct AnimationData
  {
    AnimationData()
    : resendFlag(0),
      playState(),
      width(0),
      height(0)
    {
    }

    AnimationData& operator=(const AnimationData& rhs)
    {
      resendFlag |= rhs.resendFlag; // OR resend flag
      playState    = rhs.playState;
      width        = rhs.width;
      height       = rhs.height;
      return *this;
    }

    uint32_t                                   resendFlag;
    Extension::RiveAnimationView::PlayState    playState;
    uint32_t                                   width;
    uint32_t                                   height;
  };

  /**
   * @brief Constructor.
   */
  RiveAnimationTask();

  /**
   * @brief Destructor.
   */
  ~RiveAnimationTask() override;

  /**
   * @brief Finalizes the task.
   */
  void Finalize();

  /**
   * @brief Loads the animation file.
   *
   * @param[in] url The url of the rive animation file
   * @return True if loading success, false otherwise.
   */
  bool Load(const std::string& url);

  /**
   * @brief Sets the renderer used to display the result image.
   *
   * @param[in] renderer The renderer used to display the result image
   */
  void SetRenderer(Renderer renderer);

  /**
   * @brief Sets data to specify animation playback.
   * @param[in] data The animation data
   */
  void SetAnimationData(const AnimationData& data);

  /**
   * @brief This callback is called after the animation is finished.
   * @param[in] callback The animation finished callback
   */
  void SetAnimationFinishedCallback(EventThreadCallback* callback);

  /**
   * @brief Gets the default size of the file,.
   * @return The default size of the file
   */
  void GetDefaultSize(uint32_t& width, uint32_t& height) const;

  /**
   * @brief Connect to this signal to be notified when the texture upload is completed.
   * @return The signal to connect to.
   */
  UploadCompletedSignalType& UploadCompletedSignal();

  /**
   * @brief Rasterizes the current frame.
   * @return true if the animation is running, false otherwise.
   */
  bool Rasterize();

  /**
   * @brief Calculates the time for the next frame rasterization.
   * @return The time for the next frame rasterization.
   */
  TimePoint CalculateNextFrameTime(bool renderNow);

  /**
   * @brief Gets the time for the next frame rasterization.
   * @return The time for the next frame rasterization.
   */
  TimePoint GetNextFrameTime();

private:
  /**
   * @brief Play the rive animation.
   */
  void PlayAnimation();

  /**
   * @brief Stop the rive animation.
   */
  void StopAnimation();

  /**
   * @brief Pause the rive animation.
   */
  void PauseAnimation();

  /**
   * @brief Sets the target image size.
   *
   * @param[in] width The target image width
   * @param[in] height The target image height
   */
  void SetSize(uint32_t width, uint32_t height);

  /**
   * @brief Applies the animation data set by the main thread.
   */
  void ApplyAnimationData();

  // Undefined
  RiveAnimationTask(const RiveAnimationTask& task) = delete;

  // Undefined
  RiveAnimationTask& operator=(const RiveAnimationTask& task) = delete;

private:
  enum class PlayState
  {
    STOPPING, ///< The animation is stopping
    STOPPED,  ///< The animation has stopped
    PLAYING,  ///< The animation is playing
    PAUSED    ///< The animation is paused
  };

  std::string                          mUrl;
  VectorAnimationRenderer              mVectorRenderer;
  AnimationData                        mAnimationData[2];
  RiveAnimationThread&                 mRiveAnimationThread;
  ConditionalWait                      mConditionalWait;
  std::unique_ptr<EventThreadCallback> mAnimationFinishedTrigger;
  PlayState                            mPlayState;
  TimePoint                            mNextFrameStartTime;
  int64_t                              mFrameDurationMicroSeconds;
  float                                mFrameRate;
  uint32_t                             mCurrentFrame;
  uint32_t                             mTotalFrame;
  uint32_t                             mStartFrame;
  uint32_t                             mEndFrame;
  uint32_t                             mDroppedFrames;
  uint32_t                             mWidth;
  uint32_t                             mHeight;
  uint32_t                             mAnimationDataIndex;
  bool                                 mUpdateFrameNumber;
  bool                                 mNeedAnimationFinishedTrigger;
  bool                                 mAnimationDataUpdated;
  bool                                 mDestroyTask;
};

} // namespace Internal

} // namespace Toolkit

} // namespace Dali

#endif // DALI_EXTENSION_INTERNAL_RIVE_ANIMATION_TASK_H
