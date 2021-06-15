#ifndef DALI_EXTENSION_INTERNAL_RIVE_RASTERIZE_THREAD_H
#define DALI_EXTENSION_INTERNAL_RIVE_RASTERIZE_THREAD_H

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
#include <dali/devel-api/threading/conditional-wait.h>
#include <dali/devel-api/threading/thread.h>
#include <dali/integration-api/adaptor-framework/log-factory-interface.h>
#include <memory>
#include <vector>

// INTERNAL INCLUDES
#include <dali-extension/internal/rive-animation-view/rive-animation-task.h>

namespace Dali
{
namespace Extension
{
namespace Internal
{
/**
 * The worker thread for rive image rasterization.
 */
class RiveRasterizeThread : public Thread
{
public:
  /**
   * @brief Constructor.
   */
  RiveRasterizeThread();

  /**
   * @brief Destructor.
   */
  ~RiveRasterizeThread() override;

  /**
   * The callback is called from the rasterize thread after the rasterization is completed.
   * @param[in] callBack  The function to call.
   */
  void SetCompletedCallback(CallbackBase* callback);

  /**
   * Add a task to rasterize.
   *
   * @param[in] task The task to rasterize
   */
  void AddTask(RiveAnimationTaskPtr task);

protected:
  /**
   * @brief The entry function of the worker thread.
   *        It rasterizes the rive image.
   */
  void Run() override;

private:
  /**
   * Rasterizes the tasks.
   */
  void Rasterize();

private:
  // Undefined
  RiveRasterizeThread(const RiveRasterizeThread& thread) = delete;

  // Undefined
  RiveRasterizeThread& operator=(const RiveRasterizeThread& thread) = delete;

private:
  std::vector<RiveAnimationTaskPtr> mRasterizeTasks;
  ConditionalWait                   mConditionalWait;
  std::unique_ptr<CallbackBase>     mCompletedCallback;
  bool                              mDestroyThread; ///< Whether the thread be destroyed
  bool                              mIsThreadStarted;
  const Dali::LogFactoryInterface&  mLogFactory; ///< The log factory
};

} // namespace Internal

} // namespace Extension

} // namespace Dali

#endif // DALI_EXTENSION_INTERNAL_RIVE_RASTERIZE_THREAD_H
