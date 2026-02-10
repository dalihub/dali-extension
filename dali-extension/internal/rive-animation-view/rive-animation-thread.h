#ifndef DALI_EXTENSION_INTERNAL_RIVE_ANIMATION_THREAD_H
#define DALI_EXTENSION_INTERNAL_RIVE_ANIMATION_THREAD_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/threading/conditional-wait.h>
#include <dali/devel-api/threading/thread.h>
#include <dali/integration-api/adaptor-framework/log-factory-interface.h>
#include <dali/integration-api/adaptor-framework/round-robin-container-view.h>
#include <dali/public-api/signals/connection-tracker.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali-extension/internal/rive-animation-view/rive-animation-task.h>
#include <dali-extension/internal/rive-animation-view/rive-rasterize-thread.h>

namespace Dali
{
namespace Extension
{
namespace Internal
{
/**
 * The main animation thread for rive animations
 */
class RiveAnimationThread : public Thread
{
public:
  /**
   * @brief Constructor.
   */
  RiveAnimationThread();

  /**
   * @brief Destructor.
   */
  ~RiveAnimationThread() override;

  /**
   * Add a animation task into the rive animation thread, called by main thread.
   *
   * @param[in] task The task added to the thread.
   */
  void AddTask(RiveAnimationTaskPtr task);

  /**
   * @brief Called when the rasterization is completed from the rasterize thread.
   * @param task The completed task
   */
  void OnTaskCompleted(RiveAnimationTaskPtr task, bool stopped);

  /**
   * @brief Called when the sleep thread is awaken.
   */
  void OnAwakeFromSleep();

protected:
  /**
   * @brief The entry function of the animation thread.
   */
  void Run() override;

private:
  /**
   * Rasterizes the tasks.
   */
  void Rasterize();

private:
  /**
   * @brief Helper class to keep the relation between RiveRasterizeThread and corresponding container
   */
  class RasterizeHelper : public ConnectionTracker
  {
  public:
    /**
     * @brief Create an RasterizeHelper.
     *
     * @param[in] animationThread Reference to the RiveAnimationThread
     */
    RasterizeHelper(RiveAnimationThread& animationThread);

    /**
     * @brief Rasterizes the task.
     *
     * @param[in] task The task to rasterize.
     */
    void Rasterize(RiveAnimationTaskPtr task);

  public:
    RasterizeHelper(const RasterizeHelper&)            = delete;
    RasterizeHelper& operator=(const RasterizeHelper&) = delete;

    RasterizeHelper(RasterizeHelper&& rhs);
    RasterizeHelper& operator=(RasterizeHelper&& rhs) = delete;

  private:
    /**
     * @brief Main constructor that used by all other constructors
     */
    RasterizeHelper(std::unique_ptr<RiveRasterizeThread> rasterizer, RiveAnimationThread& animationThread);

  private:
    std::unique_ptr<RiveRasterizeThread> mRasterizer;
    RiveAnimationThread&                 mAnimationThread;
  };

  /**
   * @brief The thread to sleep until the next frame time.
   */
  class SleepThread : public Thread
  {
  public:
    /**
     * @brief Constructor.
     */
    SleepThread(CallbackBase* callback);

    /**
     * @brief Destructor.
     */
    ~SleepThread() override;

    /**
     * @brief Sleeps untile the specified time point.
     */
    void SleepUntil(std::chrono::time_point<std::chrono::system_clock> timeToSleepUntil);

  protected:
    /**
     * @brief The entry function of the animation thread.
     */
    void Run() override;

  private:
    SleepThread(const SleepThread& thread)            = delete;
    SleepThread& operator=(const SleepThread& thread) = delete;

  private:
    ConditionalWait                                    mConditionalWait;
    std::unique_ptr<CallbackBase>                      mAwakeCallback;
    std::chrono::time_point<std::chrono::system_clock> mSleepTimePoint;
    const Dali::LogFactoryInterface&                   mLogFactory;
    bool                                               mNeedToSleep;
    bool                                               mDestroyThread;
  };

private:
  // Undefined
  RiveAnimationThread(const RiveAnimationThread& thread) = delete;

  // Undefined
  RiveAnimationThread& operator=(const RiveAnimationThread& thread) = delete;

private:
  std::vector<RiveAnimationTaskPtr>              mAnimationTasks;
  std::vector<RiveAnimationTaskPtr>              mCompletedTasks;
  std::vector<RiveAnimationTaskPtr>              mWorkingTasks;
  Dali::RoundRobinContainerView<RasterizeHelper> mRasterizers;
  SleepThread                                    mSleepThread;
  ConditionalWait                                mConditionalWait;
  bool                                           mNeedToSleep;
  bool                                           mDestroyThread;
  const Dali::LogFactoryInterface&               mLogFactory;
};

} // namespace Internal

} // namespace Extension

} // namespace Dali

#endif // DALI_EXTENSION_INTERNAL_RIVE_ANIMATION_THREAD_H
