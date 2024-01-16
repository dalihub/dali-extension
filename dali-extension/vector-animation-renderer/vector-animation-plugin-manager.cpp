/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/common/map-wrapper.h>
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
  mTriggerOrderId(0u),
  mMutex(),
  mEventTrigger(),
  mEventTriggered(false)
{
}

VectorAnimationPluginManager::~VectorAnimationPluginManager()
{
  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "this = %p\n", this);
}

void VectorAnimationPluginManager::AddEventHandler(VectorAnimationEventHandler& handler)
{
  if(mEventHandlers.end() == mEventHandlers.find(&handler))
  {
    if(mEventHandlers.empty())
    {
      Adaptor::Get().RegisterProcessor(*this);
    }

    mEventHandlers.insert(&handler);

    {
      Dali::Mutex::ScopedLock lock(mMutex);

      if(!mEventTrigger)
      {
        mEventTrigger   = std::unique_ptr<EventThreadCallback>(new EventThreadCallback(MakeCallback(this, &VectorAnimationPluginManager::OnEventTriggered)));
        mEventTriggered = false;
      }
    }
  }
}

void VectorAnimationPluginManager::RemoveEventHandler(VectorAnimationEventHandler& handler)
{
  auto iter = mEventHandlers.find(&handler);
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

    auto triggeredHandler = mTriggeredHandlers.find(&handler);
    if(triggeredHandler != mTriggeredHandlers.end())
    {
      mTriggeredHandlers.erase(triggeredHandler);
    }

    if(releaseEventTrigger)
    {
      mEventTrigger.reset();
      mEventTriggered = false;
    }
  }
}

void VectorAnimationPluginManager::TriggerEvent(VectorAnimationEventHandler& handler)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mTriggeredHandlers.end() == mTriggeredHandlers.find(&handler))
  {
    mTriggeredHandlers.insert({&handler, mTriggerOrderId++});

    if(mEventTrigger && !mEventTriggered)
    {
      mEventTrigger->Trigger();
      mEventTriggered = true;
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
  std::map<uint32_t, VectorAnimationEventHandler*> handlers;

  decltype(mTriggeredHandlers) movedTriggeredHandlers;

  {
    Dali::Mutex::ScopedLock lock(mMutex);

    // Copy the list to the local variable and clear
    movedTriggeredHandlers = std::move(mTriggeredHandlers);
    mTriggeredHandlers.clear();
    mTriggeredHandlers.rehash(0u);

    mTriggerOrderId = 0u;
    mEventTriggered = false;
  }

  // Reorder event handler ordered by trigger request.
  for(auto&& iter : movedTriggeredHandlers)
  {
    handlers[iter.second] = iter.first;
  }
  movedTriggeredHandlers.clear();

  for(auto&& iter : handlers)
  {
    auto* handler = iter.second;

    // Check if it is valid
    if(mEventHandlers.end() != mEventHandlers.find(handler))
    {
      handler->NotifyEvent();
    }
  }
}

} // namespace Plugin

} // namespace Dali
