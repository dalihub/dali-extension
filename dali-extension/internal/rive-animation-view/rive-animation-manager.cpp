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
#include <dali-extension/internal/rive-animation-view/rive-animation-manager.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali-extension/internal/rive-animation-view/rive-animation-thread.h>

namespace Dali
{
namespace Extension
{
namespace Internal
{
namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gRiveAnimationLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_RIVE_ANIMATION");
#endif

} // unnamed namespace

std::unique_ptr<RiveAnimationManager> RiveAnimationManager::mInstance = nullptr;
std::once_flag                        RiveAnimationManager::mOnceFlag;

RiveAnimationManager& RiveAnimationManager::GetInstance()
{
  std::call_once(mOnceFlag, []() {
    mInstance.reset(new RiveAnimationManager);
  });
  return *(mInstance.get());
}

RiveAnimationManager::RiveAnimationManager()
: mEventCallbacks(),
  mLifecycleObservers(),
  mRiveAnimationThread(nullptr),
  mProcessorRegistered(false)
{
}

RiveAnimationManager::~RiveAnimationManager()
{
  mEventCallbacks.clear();

  if(mProcessorRegistered)
  {
    Adaptor::Get().UnregisterProcessor(*this);
  }

  for(auto observer : mLifecycleObservers)
  {
    observer->RiveAnimationManagerDestroyed();
  }
}

void RiveAnimationManager::AddObserver(RiveAnimationManager::LifecycleObserver& observer)
{
  DALI_ASSERT_DEBUG(mLifecycleObservers.end() == std::find(mLifecycleObservers.begin(), mLifecycleObservers.end(), &observer));
  mLifecycleObservers.push_back(&observer);
}

void RiveAnimationManager::RemoveObserver(RiveAnimationManager::LifecycleObserver& observer)
{
  auto iterator = std::find(mLifecycleObservers.begin(), mLifecycleObservers.end(), &observer);
  if(iterator != mLifecycleObservers.end())
  {
    mLifecycleObservers.erase(iterator);
  }
}

RiveAnimationThread& RiveAnimationManager::GetRiveAnimationThread()
{
  if(!mRiveAnimationThread)
  {
    mRiveAnimationThread = std::unique_ptr<RiveAnimationThread>(new RiveAnimationThread());
    mRiveAnimationThread->Start();
  }
  return *mRiveAnimationThread;
}

void RiveAnimationManager::RegisterEventCallback(CallbackBase* callback)
{
  mEventCallbacks.emplace_back(std::unique_ptr<Dali::CallbackBase>(callback));

  if(!mProcessorRegistered)
  {
    Adaptor::Get().RegisterProcessor(*this);
    mProcessorRegistered = true;
  }
}

void RiveAnimationManager::UnregisterEventCallback(CallbackBase* callback)
{
  auto iter = std::find_if(mEventCallbacks.begin(),
                           mEventCallbacks.end(),
                           [callback](const std::unique_ptr<CallbackBase>& element) {
                             return element.get() == callback;
                           });
  if(iter != mEventCallbacks.end())
  {
    mEventCallbacks.erase(iter);

    if(mEventCallbacks.empty())
    {
      if(Adaptor::IsAvailable())
      {
        Adaptor::Get().UnregisterProcessor(*this);
        mProcessorRegistered = false;
      }
    }
  }
}

void RiveAnimationManager::Process(bool postProcessor)
{
  for(auto&& iter : mEventCallbacks)
  {
    CallbackBase::Execute(*iter);
  }
  mEventCallbacks.clear();

  Adaptor::Get().UnregisterProcessor(*this);
  mProcessorRegistered = false;
}

} // namespace Internal

} // namespace Extension

} // namespace Dali
