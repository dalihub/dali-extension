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
#include <dali-extension/vector-animation-renderer/vector-animation-plugin-manager.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Plugin
{
namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gVectorAnimationLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_VECTOR_ANIMATION");
#endif
} // unnamed namespace

VectorAnimationPluginManager& VectorAnimationPluginManager::Get()
{
  static VectorAnimationPluginManager animationManager;
  return animationManager;
}

VectorAnimationPluginManager::VectorAnimationPluginManager()
: mEventHandlers(),
  mTriggeredHandlers(),
  mMutex(),
  mEventTrigger()
{
}

VectorAnimationPluginManager::~VectorAnimationPluginManager()
{
  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "this = %p\n", this);
}

void VectorAnimationPluginManager::AddEventHandler(VectorAnimationEventHandler& handler)
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
        mEventTrigger = std::unique_ptr<EventThreadCallback>(new EventThreadCallback(MakeCallback(this, &VectorAnimationPluginManager::OnEventTriggered)));
      }
    }
  }
}

void VectorAnimationPluginManager::RemoveEventHandler(VectorAnimationEventHandler& handler)
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

void VectorAnimationPluginManager::TriggerEvent(VectorAnimationEventHandler& handler)
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

void VectorAnimationPluginManager::Process(bool postProcessor)
{
  OnEventTriggered();
}

// This function is called in the main thread.
void VectorAnimationPluginManager::OnEventTriggered()
{
  std::vector<VectorAnimationEventHandler*> handlers;

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

} // namespace Plugin

} // namespace Dali
