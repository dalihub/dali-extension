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
#include <dali-extension/internal/rive-animation-view/animation-renderer/rive-animation-renderer-manager.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>

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

RiveAnimationRendererManager& RiveAnimationRendererManager::Get()
{
  static RiveAnimationRendererManager animationManager;
  return animationManager;
}

RiveAnimationRendererManager::RiveAnimationRendererManager()
: mEventHandlers(),
  mTriggeredHandlers(),
  mMutex(),
  mEventTrigger()
{
}

RiveAnimationRendererManager::~RiveAnimationRendererManager()
{
  DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationRendererManager::~RiveAnimationRendererManager: this = %p\n", this);
}

void RiveAnimationRendererManager::AddEventHandler(RiveAnimationRendererEventHandler& handler)
{
  if(mEventHandlers.end() == std::find(mEventHandlers.begin(), mEventHandlers.end(), &handler))
  {
    if(mEventHandlers.empty())
    {
      Adaptor::Get().RegisterProcessor(*this);
    }

    mEventHandlers.push_back(&handler);

    {
      Dali::Mutex::ScopedLock lock(mMutex);

      if(!mEventTrigger)
      {
        mEventTrigger = std::unique_ptr<EventThreadCallback>(new EventThreadCallback(MakeCallback(this, &RiveAnimationRendererManager::OnEventTriggered)));
      }
    }
  }
}

void RiveAnimationRendererManager::RemoveEventHandler(RiveAnimationRendererEventHandler& handler)
{
  auto iter = std::find(mEventHandlers.begin(), mEventHandlers.end(), &handler);
  if(iter != mEventHandlers.end())
  {
    mEventHandlers.erase(iter);
  }

  bool releaseEventTrigger = false;

  if(mEventHandlers.empty())
  {
    if(Adaptor::IsAvailable())
    {
      Adaptor::Get().UnregisterProcessor(*this);
    }

    releaseEventTrigger = true;
  }

  {
    Dali::Mutex::ScopedLock lock(mMutex);

    auto triggeredHandler = std::find(mTriggeredHandlers.begin(), mTriggeredHandlers.end(), &handler);
    if(triggeredHandler != mTriggeredHandlers.end())
    {
      mTriggeredHandlers.erase(triggeredHandler);
    }

    if(releaseEventTrigger)
    {
      mEventTrigger.reset();
    }
  }
}

void RiveAnimationRendererManager::TriggerEvent(RiveAnimationRendererEventHandler& handler)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mTriggeredHandlers.end() == std::find(mTriggeredHandlers.begin(), mTriggeredHandlers.end(), &handler))
  {
    mTriggeredHandlers.push_back(&handler);

    if(mEventTrigger)
    {
      mEventTrigger->Trigger();
    }
  }
}

void RiveAnimationRendererManager::Process(bool postProcessor)
{
  OnEventTriggered();
}

// This function is called in the main thread.
void RiveAnimationRendererManager::OnEventTriggered()
{
  std::vector<RiveAnimationRendererEventHandler*> handlers;

  {
    Dali::Mutex::ScopedLock lock(mMutex);

    // Copy the list to the local variable and clear
    handlers = mTriggeredHandlers;
    mTriggeredHandlers.clear();
  }

  for(auto&& iter : handlers)
  {
    // Check if it is valid
    auto handler = std::find(mEventHandlers.begin(), mEventHandlers.end(), iter);
    if(handler != mEventHandlers.end())
    {
      iter->NotifyEvent();
    }
  }
}

} // namespace Internal

} // namespace Extension

} // namespace Dali
