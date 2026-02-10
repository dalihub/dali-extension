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
  mEventTrigger(new EventThreadCallback(MakeCallback(this, &VectorAnimationPluginManager::OnEventTriggered))),
  mProcessorRegistered(false),
  mEventTriggered(false),
  mEventHandlerRemovedDuringEventProcessing(false)
{
  DALI_LOG_DEBUG_INFO("VectorAnimationPluginManager Trigger Id(%d)\n", mEventTrigger->GetId());
}

VectorAnimationPluginManager::~VectorAnimationPluginManager()
{
  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "this = %p\n", this);

  {
    Dali::Mutex::ScopedLock lock(mMutex);
    mEventTrigger.reset();
  }

  if(Adaptor::IsAvailable() && mProcessorRegistered)
  {
    Adaptor::Get().UnregisterProcessorOnce(*this);
    mProcessorRegistered = false;
  }
}

// This function is called in the main thread.
void VectorAnimationPluginManager::AddEventHandler(VectorAnimationEventHandler& handler)
{
  if(mEventHandlers.end() == mEventHandlers.find(&handler))
  {
    if(!mProcessorRegistered)
    {
      mProcessorRegistered = true;
      Adaptor::Get().RegisterProcessorOnce(*this);
    }

    mEventHandlers.insert(&handler);
  }
}

// This function is called in the main thread.
void VectorAnimationPluginManager::RemoveEventHandler(VectorAnimationEventHandler& handler)
{
  auto iter = mEventHandlers.find(&handler);
  if(iter != mEventHandlers.end())
  {
    bool releaseEventTrigger = false;

    mEventHandlers.erase(iter);

    // Mark removed flag now.
    // Note that it will be removed at the begin of event processing.
    mEventHandlerRemovedDuringEventProcessing = true;

    if(mEventHandlers.empty())
    {
      releaseEventTrigger = true;
    }

    {
      Dali::Mutex::ScopedLock lock(mMutex);

      if(releaseEventTrigger)
      {
        // There is no valid event handler now. We could remove whole triggered event handlers.
        mTriggeredHandlers.clear();
        mEventHandlerRemovedDuringEventProcessing = false;
      }
      else
      {
        auto triggeredHandler = mTriggeredHandlers.find(&handler);
        if(triggeredHandler != mTriggeredHandlers.end())
        {
          mTriggeredHandlers.erase(triggeredHandler);
        }
      }
    }
  }
}

// Called by VectorAnimationTaskThread
void VectorAnimationPluginManager::TriggerEvent(VectorAnimationEventHandler& handler)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  // Add triggered handler only if event trigger exist.
  // If event handler is null, mean there is no valid handler now. So we can ignore this trigger.
  if(DALI_LIKELY(mEventTrigger))
  {
    if(mTriggeredHandlers.end() == mTriggeredHandlers.find(&handler))
    {
      mTriggeredHandlers.insert({&handler, mTriggerOrderId++});

      if(!mEventTriggered)
      {
        mEventTrigger->Trigger();
        mEventTriggered = true;
      }
    }
  }
}

void VectorAnimationPluginManager::Process(bool postProcessor)
{
  mProcessorRegistered = false;
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
    movedTriggeredHandlers.swap(mTriggeredHandlers);

    mTriggerOrderId = 0u;
    mEventTriggered = false;
  }

  // Reorder event handler ordered by trigger request.
  // And also, check validation of event handler.
  for(auto&& iter : movedTriggeredHandlers)
  {
    if(mEventHandlers.end() != mEventHandlers.find(iter.first))
    {
      handlers[iter.second] = iter.first;
    }
  }

  // We check validation before move handlers. Reset removed flag now.
  mEventHandlerRemovedDuringEventProcessing = false;

  for(auto&& iter : handlers)
  {
    auto* handler = iter.second;

    // Check if it is valid
    // (If the event handler is removed during event processing, it is not valid. So we should not notify event)
    if(!mEventHandlerRemovedDuringEventProcessing || mEventHandlers.end() != mEventHandlers.find(handler))
    {
      handler->NotifyEvent();
    }
  }
}

} // namespace Plugin

} // namespace Dali
