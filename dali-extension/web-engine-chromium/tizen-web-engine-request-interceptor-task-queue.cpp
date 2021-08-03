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

#include "tizen-web-engine-request-interceptor-task-queue.h"

namespace Dali
{
namespace Plugin
{

std::atomic<TaskQueue*> TaskQueue::mInstance { nullptr };
std::mutex TaskQueue::mInstanceMutex;

TaskQueue* TaskQueue::GetInstance()
{
  if(mInstance == nullptr)
  {
    std::lock_guard<std::mutex> lock(mInstanceMutex);
    if(mInstance == nullptr)
    {
      mInstance = new TaskQueue();
    }
  }
  return mInstance;
}

TaskQueue::TaskQueue()
  : mIsThreadWaiting(true)
{
}

void TaskQueue::AddTask(TaskCallback task)
{
  std::unique_lock<std::mutex> lock(mMutex);
  mTaskQueue.push_back(task);
}

void TaskQueue::NotifyTaskReady()
{
  std::unique_lock<std::mutex> lock(mMutex);
  mIsThreadWaiting = false;

  // wake up the request interceptor thread
  mCondition.notify_all();
}

void TaskQueue::WaitAndRunTasks()
{
  // wait for tasks.
  std::unique_lock<std::mutex> lock(mMutex);
  while(mIsThreadWaiting)
  {
    mCondition.wait(lock);
  }
  mIsThreadWaiting = true;

  // execute tasks.
  for(std::vector<TaskCallback>::iterator iter = mTaskQueue.begin(); iter != mTaskQueue.end(); iter++)
  {
    (*iter)();
  }
  mTaskQueue.clear();
}

} // namespace Plugin
} // namespace Dali
