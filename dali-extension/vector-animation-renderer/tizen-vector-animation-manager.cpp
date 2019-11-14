/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali-extension/vector-animation-renderer/tizen-vector-animation-manager.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>

namespace Dali
{

namespace Plugin
{

TizenVectorAnimationManager& TizenVectorAnimationManager::Get()
{
  static TizenVectorAnimationManager animationManager;
  return animationManager;
}

TizenVectorAnimationManager::TizenVectorAnimationManager()
: mEventHandlers(),
  mTriggeredHandlers(),
  mMutex(),
  mEventTrigger( new EventThreadCallback( MakeCallback( this, &TizenVectorAnimationManager::OnEventTriggered ) ) )
{
}

TizenVectorAnimationManager::~TizenVectorAnimationManager()
{
}

void TizenVectorAnimationManager::AddEventHandler( TizenVectorAnimationEventHandler& handler )
{
  if( mEventHandlers.end() == std::find( mEventHandlers.begin(), mEventHandlers.end(), &handler ) )
  {
    if( mEventHandlers.empty() )
    {
      Adaptor::Get().RegisterProcessor( *this );
    }

    mEventHandlers.push_back( &handler );
  }
}

void TizenVectorAnimationManager::RemoveEventHandler( TizenVectorAnimationEventHandler& handler )
{
  auto iter = std::find( mEventHandlers.begin(), mEventHandlers.end(), &handler );
  if( iter != mEventHandlers.end() )
  {
    mEventHandlers.erase( iter );

    if( mEventHandlers.empty() )
    {
      if( Adaptor::IsAvailable() )
      {
        Adaptor::Get().UnregisterProcessor( *this );
      }
    }
  }
}

void TizenVectorAnimationManager::TriggerEvent( TizenVectorAnimationEventHandler& handler )
{
  Dali::Mutex::ScopedLock lock( mMutex );

  if( mTriggeredHandlers.end() == std::find( mTriggeredHandlers.begin(), mTriggeredHandlers.end(), &handler ) )
  {
    mTriggeredHandlers.push_back( &handler );

    mEventTrigger->Trigger();
  }
}

void TizenVectorAnimationManager::Process()
{
  OnEventTriggered();
}

void TizenVectorAnimationManager::OnEventTriggered()
{
  Dali::Mutex::ScopedLock lock( mMutex );

  for( auto&& iter : mTriggeredHandlers )
  {
    iter->NotifyEvent();
  }

  mTriggeredHandlers.clear();
}

} // namespace Plugin

} // namespace Dali;
