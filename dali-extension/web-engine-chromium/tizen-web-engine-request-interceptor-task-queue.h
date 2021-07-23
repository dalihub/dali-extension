#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_REQUEST_INTERCEPTOR_TASK_QUEUE_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_REQUEST_INTERCEPTOR_TASK_QUEUE_H

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
 *
 */

// EXTERNAL INCLUDES
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <vector>

namespace Dali
{
namespace Plugin
{

/**
 * @brief A class TaskQueue for running tasks. It is thread-safe.
 */
class TaskQueue
{
public:
  static TaskQueue* GetInstance();

  /**
   * @brief Task callback.
   */
  using TaskCallback = std::function<bool(void)>;

  /**
   * @brief Add task.
   */
  void AddTask(TaskCallback taskQueue);

  /**
   * @brief Notify that tasks are ready.
   */
  void NotifyTaskReady();

  /**
   * @brief Wait for and run tasks.
   */
  void WaitAndRunTasks();

private:
  TaskQueue();

  static std::atomic<TaskQueue*> mInstance;
  static std::mutex              mInstanceMutex;

  bool                      mIsThreadWaiting;
  std::vector<TaskCallback> mTaskQueue;
  std::mutex                mMutex;
  std::condition_variable   mCondition;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_REQUEST_INTERCEPTOR_TASK_QUEUE_H
