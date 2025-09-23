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
#include <dali-extension/internal/rive-animation-view/rive-animation-thread.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/adaptor-framework/thread-settings.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>
#include <thread>

namespace Dali
{
namespace Extension
{
namespace Internal
{
namespace
{
constexpr auto DEFAULT_NUMBER_OF_RASTERIZE_THREADS = size_t{4u};
constexpr auto NUMBER_OF_RASTERIZE_THREADS_ENV     = "DALI_RIVE_RASTERIZE_THREADS";

size_t GetNumberOfThreads(const char* environmentVariable, size_t defaultValue)
{
  using Dali::EnvironmentVariable::GetEnvironmentVariable;
  auto           numberString          = GetEnvironmentVariable(environmentVariable);
  auto           numberOfThreads       = numberString ? std::strtoul(numberString, nullptr, 10) : 0;
  constexpr auto MAX_NUMBER_OF_THREADS = 100u;
  DALI_ASSERT_DEBUG(numberOfThreads < MAX_NUMBER_OF_THREADS);
  return (numberOfThreads > 0 && numberOfThreads < MAX_NUMBER_OF_THREADS) ? numberOfThreads : defaultValue;
}

#if defined(DEBUG_ENABLED)
Debug::Filter* gRiveAnimationLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_RIVE_ANIMATION");
#endif

} // unnamed namespace

RiveAnimationThread::RiveAnimationThread()
: mAnimationTasks(),
  mCompletedTasks(),
  mWorkingTasks(),
  mRasterizers(GetNumberOfThreads(NUMBER_OF_RASTERIZE_THREADS_ENV, DEFAULT_NUMBER_OF_RASTERIZE_THREADS), [&]()
{ return RasterizeHelper(*this); }),
  mSleepThread(MakeCallback(this, &RiveAnimationThread::OnAwakeFromSleep)),
  mConditionalWait(),
  mNeedToSleep(false),
  mDestroyThread(false),
  mLogFactory(Dali::Adaptor::Get().GetLogFactory())
{
  mSleepThread.Start();
}

RiveAnimationThread::~RiveAnimationThread()
{
  // Stop the thread
  {
    ConditionalWait::ScopedLock lock(mConditionalWait);
    mDestroyThread = true;
    mNeedToSleep   = false;
    mConditionalWait.Notify(lock);
  }

  DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationThread::~RiveAnimationThread: Join [%p]\n", this);

  Join();
}

void RiveAnimationThread::AddTask(RiveAnimationTaskPtr task)
{
  ConditionalWait::ScopedLock lock(mConditionalWait);

  if(mAnimationTasks.end() == std::find(mAnimationTasks.begin(), mAnimationTasks.end(), task))
  {
    auto currentTime = task->CalculateNextFrameTime(true); // Rasterize as soon as possible

    bool inserted = false;
    for(auto iter = mAnimationTasks.begin(); iter != mAnimationTasks.end(); ++iter)
    {
      auto nextTime = (*iter)->GetNextFrameTime();
      if(nextTime > currentTime)
      {
        mAnimationTasks.insert(iter, task);
        inserted = true;
        break;
      }
    }

    if(!inserted)
    {
      mAnimationTasks.push_back(task);
    }

    mNeedToSleep = false;
    // wake up the animation thread
    mConditionalWait.Notify(lock);
  }
}

void RiveAnimationThread::OnTaskCompleted(RiveAnimationTaskPtr task, bool keepAnimation)
{
  if(!mDestroyThread)
  {
    ConditionalWait::ScopedLock lock(mConditionalWait);
    bool                        needRasterize = false;

    auto workingTask = std::find(mWorkingTasks.begin(), mWorkingTasks.end(), task);
    if(workingTask != mWorkingTasks.end())
    {
      mWorkingTasks.erase(workingTask);
    }

    // Check pending task
    if(mAnimationTasks.end() != std::find(mAnimationTasks.begin(), mAnimationTasks.end(), task))
    {
      needRasterize = true;
    }

    if(keepAnimation)
    {
      if(mCompletedTasks.end() == std::find(mCompletedTasks.begin(), mCompletedTasks.end(), task))
      {
        mCompletedTasks.push_back(task);
        needRasterize = true;
      }
    }

    if(needRasterize)
    {
      mNeedToSleep = false;
      // wake up the animation thread
      mConditionalWait.Notify(lock);
    }
  }
}

void RiveAnimationThread::OnAwakeFromSleep()
{
  if(!mDestroyThread)
  {
    mNeedToSleep = false;
    // wake up the animation thread
    mConditionalWait.Notify();
  }
}

void RiveAnimationThread::Run()
{
  SetThreadName("RiveAnimationThread");
  mLogFactory.InstallLogFunction();

  while(!mDestroyThread)
  {
    Rasterize();
  }
}

void RiveAnimationThread::Rasterize()
{
  // Lock while popping task out from the queue
  ConditionalWait::ScopedLock lock(mConditionalWait);

  // conditional wait
  if(mNeedToSleep)
  {
    mConditionalWait.Wait(lock);
  }

  mNeedToSleep = true;

  // Process completed tasks
  for(auto&& task : mCompletedTasks)
  {
    if(mAnimationTasks.end() == std::find(mAnimationTasks.begin(), mAnimationTasks.end(), task))
    {
      // Should use the frame rate of the animation file
      auto nextFrameTime = task->CalculateNextFrameTime(false);

      bool inserted = false;
      for(auto iter = mAnimationTasks.begin(); iter != mAnimationTasks.end(); ++iter)
      {
        auto time = (*iter)->GetNextFrameTime();
        if(time > nextFrameTime)
        {
          mAnimationTasks.insert(iter, task);
          inserted = true;
          break;
        }
      }

      if(!inserted)
      {
        mAnimationTasks.push_back(task);
      }
    }
  }
  mCompletedTasks.clear();

  // pop out the next task from the queue
  for(auto it = mAnimationTasks.begin(); it != mAnimationTasks.end();)
  {
    RiveAnimationTaskPtr nextTask = *it;

    auto currentTime   = std::chrono::system_clock::now();
    auto nextFrameTime = nextTask->GetNextFrameTime();

#if defined(DEBUG_ENABLED)
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(nextFrameTime - currentTime);

    DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationThread::Rasterize: [next time = %lld]\n", duration.count());
#endif

    if(nextFrameTime <= currentTime)
    {
      // If the task is not in the working list
      if(std::find(mWorkingTasks.begin(), mWorkingTasks.end(), nextTask) == mWorkingTasks.end())
      {
        it = mAnimationTasks.erase(it);

        // Add it to the working list
        mWorkingTasks.push_back(nextTask);

        auto rasterizerHelperIt = mRasterizers.GetNext();
        DALI_ASSERT_ALWAYS(rasterizerHelperIt != mRasterizers.End());

        rasterizerHelperIt->Rasterize(nextTask);
      }
      else
      {
        it++;
      }
    }
    else
    {
      mSleepThread.SleepUntil(nextFrameTime);
      break;
    }
  }
}

RiveAnimationThread::RasterizeHelper::RasterizeHelper(RiveAnimationThread& animationThread)
: RasterizeHelper(std::unique_ptr<RiveRasterizeThread>(new RiveRasterizeThread()), animationThread)
{
}

RiveAnimationThread::RasterizeHelper::RasterizeHelper(RasterizeHelper&& rhs)
: RasterizeHelper(std::move(rhs.mRasterizer), rhs.mAnimationThread)
{
}

RiveAnimationThread::RasterizeHelper::RasterizeHelper(std::unique_ptr<RiveRasterizeThread> rasterizer, RiveAnimationThread& animationThread)
: mRasterizer(std::move(rasterizer)),
  mAnimationThread(animationThread)
{
  mRasterizer->SetCompletedCallback(MakeCallback(&mAnimationThread, &RiveAnimationThread::OnTaskCompleted));
}

void RiveAnimationThread::RasterizeHelper::Rasterize(RiveAnimationTaskPtr task)
{
  if(task)
  {
    mRasterizer->AddTask(task);
  }
}

RiveAnimationThread::SleepThread::SleepThread(CallbackBase* callback)
: mConditionalWait(),
  mAwakeCallback(std::unique_ptr<CallbackBase>(callback)),
  mSleepTimePoint(),
  mLogFactory(Dali::Adaptor::Get().GetLogFactory()),
  mNeedToSleep(false),
  mDestroyThread(false)
{
}

RiveAnimationThread::SleepThread::~SleepThread()
{
  // Stop the thread
  {
    ConditionalWait::ScopedLock lock(mConditionalWait);
    mDestroyThread = true;
    mConditionalWait.Notify(lock);
  }

  Join();
}

void RiveAnimationThread::SleepThread::SleepUntil(std::chrono::time_point<std::chrono::system_clock> timeToSleepUntil)
{
  ConditionalWait::ScopedLock lock(mConditionalWait);
  mSleepTimePoint = timeToSleepUntil;
  mNeedToSleep    = true;
  mConditionalWait.Notify(lock);
}

void RiveAnimationThread::SleepThread::Run()
{
  SetThreadName("RiveSleepThread");
  mLogFactory.InstallLogFunction();

  while(!mDestroyThread)
  {
    bool                                               needToSleep;
    std::chrono::time_point<std::chrono::system_clock> sleepTimePoint;

    {
      ConditionalWait::ScopedLock lock(mConditionalWait);

      needToSleep    = mNeedToSleep;
      sleepTimePoint = mSleepTimePoint;

      mNeedToSleep = false;
    }

    if(needToSleep)
    {
#if defined(DEBUG_ENABLED)
      auto sleepDuration = std::chrono::duration_cast<std::chrono::milliseconds>(mSleepTimePoint - std::chrono::system_clock::now());

      DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationThread::SleepThread::Run: [sleep duration = %lld]\n", sleepDuration.count());
#endif

      std::this_thread::sleep_until(sleepTimePoint);

      if(mAwakeCallback)
      {
        CallbackBase::Execute(*mAwakeCallback);
      }
    }

    {
      ConditionalWait::ScopedLock lock(mConditionalWait);
      if(!mDestroyThread && !mNeedToSleep)
      {
        mConditionalWait.Wait(lock);
      }
    }
  }
}

} // namespace Internal

} // namespace Extension

} // namespace Dali
