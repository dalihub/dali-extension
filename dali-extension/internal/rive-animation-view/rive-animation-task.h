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
#include <dali/devel-api/threading/conditional-wait.h>
#include <dali/public-api/object/property-array.h>
#include <chrono>
#include <memory>

// INTERNAL INCLUDES
#include <dali-extension/devel-api/rive-animation-view/rive-animation-view.h>
#include <dali-extension/internal/rive-animation-view/animation-renderer/rive-animation-renderer.h>

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
  using UploadCompletedSignalType = RiveAnimationRenderer::UploadCompletedSignalType;

  using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

  /**
   * Flags for re-sending data to the rive animation thread
   */
  enum ResendFlags
  {
    RESEND_PLAY_RANGE        = 1 << 0,
    RESEND_LOOP_COUNT        = 1 << 1,
    RESEND_STOP_BEHAVIOR     = 1 << 2,
    RESEND_LOOPING_MODE      = 1 << 3,
    RESEND_CURRENT_FRAME     = 1 << 4,
    RESEND_SIZE              = 1 << 5,
    RESEND_PLAY_STATE        = 1 << 6,
    RESEND_ENABLE_ANIMATION  = 1 << 7,
    RESEND_FILL_COLOR        = 1 << 8,
    RESEND_STROKE_COLOR      = 1 << 9,
    RESEND_OPACITY           = 1 << 10,
    RESEND_SCALE             = 1 << 11,
    RESEND_ROTATION          = 1 << 12,
    RESEND_POSITION          = 1 << 13,
  };

  /**
   * @brief Structure used to pass parameters to the rive animation task
   */
  struct AnimationData
  {
    AnimationData()
    : resendFlag(0),
      width(0),
      height(0),
      playState()
    {
    }

    AnimationData& operator=(const AnimationData& rhs)
    {
      resendFlag     |= rhs.resendFlag; // OR resend flag
      width           = rhs.width;
      height          = rhs.height;
      playState       = rhs.playState;
      animations.resize(rhs.animations.size());
      std::copy(rhs.animations.begin(), rhs.animations.end(), animations.begin());
      fillColors.resize(rhs.fillColors.size());
      std::copy(rhs.fillColors.begin(), rhs.fillColors.end(), fillColors.begin());
      strokeColors.resize(rhs.strokeColors.size());
      std::copy(rhs.strokeColors.begin(), rhs.strokeColors.end(), strokeColors.begin());
      opacities.resize(rhs.opacities.size());
      std::copy(rhs.opacities.begin(), rhs.opacities.end(), opacities.begin());
      scales.resize(rhs.scales.size());
      std::copy(rhs.scales.begin(), rhs.scales.end(), scales.begin());
      rotations.resize(rhs.rotations.size());
      std::copy(rhs.rotations.begin(), rhs.rotations.end(), rotations.begin());
      positions.resize(rhs.positions.size());
      std::copy(rhs.positions.begin(), rhs.positions.end(), positions.begin());

      return *this;
    }

    uint32_t                                      resendFlag;
    uint32_t                                      width;
    uint32_t                                      height;
    Extension::RiveAnimationView::PlayState       playState;
    std::vector<std::pair<std::string, bool>>     animations;
    std::vector<std::pair<std::string, Vector4>>  fillColors;
    std::vector<std::pair<std::string, Vector4>>  strokeColors;
    std::vector<std::pair<std::string, float>>    opacities;
    std::vector<std::pair<std::string, Vector2>>  scales;
    std::vector<std::pair<std::string, Degree>>   rotations;
    std::vector<std::pair<std::string, Vector2>>  positions;
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
   * @brief Enables the animation state of given rive animation.
   *
   * @param[in] animationName The animation name
   * @param[in] enable The state of animation
   */
  void EnableAnimation(const std::string& animationName, bool enable);

  /**
   * @brief Sets the shape fill color of given fill name.
   *
   * @param[in] fillName The fill name
   * @param[in] color The rgba color
   */
  void SetShapeFillColor(const std::string& fillName, Vector4 color);

  /**
   * @brief Sets the shape stroke color of given stroke name.
   *
   * @param[in] strokeName The stroke name
   * @param[in] color The rgba color
   */
  void SetShapeStrokeColor(const std::string& strokeName, Vector4 color);

  /**
   * @brief Sets the opacity of given node.
   *
   * @param[in] nodeName The node name
   * @param[in] opacity The opacity of given node
   */
  void SetNodeOpacity(const std::string& nodeName, float opacity);

  /**
   * @brief Sets the scale of given node.
   *
   * @param[in] nodeName The node name
   * @param[in] scale The scale of given node
   */
  void SetNodeScale(const std::string& nodeName, Vector2 scale);

  /**
   * @brief Sets the rotation of given node.
   *
   * @param[in] nodeName The node name
   * @param[in] degree The degree of given node
   */
  void SetNodeRotation(const std::string& nodeName, Degree degree);

  /**
   * @brief Sets the position of given node.
   *
   * @param[in] nodeName The node name
   * @param[in] position The position of given node
   */
  void SetNodePosition(const std::string& nodeName, Vector2 position);

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
  RiveAnimationRendererPtr             mVectorRenderer;
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
