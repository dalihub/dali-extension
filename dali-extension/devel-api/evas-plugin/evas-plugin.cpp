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

// INTERNAL INCLUDES
#include <dali-extension/internal/evas-plugin/evas-plugin-impl.h>

// CLASS HEADER
#include <dali-extension/devel-api/evas-plugin/evas-plugin.h>

namespace Dali
{

namespace Extension
{

EvasPlugin EvasPlugin::New( Evas_Object* parentEvasObject, uint16_t width, uint16_t height, bool isTranslucent )
{
  IntrusivePtr< Internal::EvasPlugin > impl = Internal::EvasPlugin::New( parentEvasObject, width, height, isTranslucent );

  EvasPlugin evasPlugin = EvasPlugin( impl.Get() );

  return evasPlugin;
}

EvasPlugin::EvasPlugin()
{
}

EvasPlugin::EvasPlugin( const EvasPlugin& evasPlugin )
: BaseHandle( evasPlugin )
{
}

EvasPlugin& EvasPlugin::operator=( const EvasPlugin& evasPlugin )
{
  if( *this != evasPlugin )
  {
    BaseHandle::operator=( evasPlugin );
  }
  return *this;
}

EvasPlugin::~EvasPlugin()
{
}

void EvasPlugin::Run()
{
  Internal::GetImplementation( *this ).Run();
}

void EvasPlugin::Pause()
{
  Internal::GetImplementation( *this ).Pause();
}

void EvasPlugin::Resume()
{
  Internal::GetImplementation( *this ).Resume();
}

void EvasPlugin::Stop()
{
  Internal::GetImplementation( *this ).Stop();
}

Scene EvasPlugin::GetDefaultScene()
{
  return Internal::GetImplementation( *this ).GetDefaultScene();
}

Evas_Object* EvasPlugin::GetAccessEvasObject()
{
  return Internal::GetImplementation( *this ).GetAccessEvasObject();
}

Evas_Object* EvasPlugin::GetDaliEvasObject()
{
  return Internal::GetImplementation( *this ).GetDaliEvasObject();
}

EvasPlugin::EvasPluginSignalType& EvasPlugin::InitSignal()
{
  return Internal::GetImplementation( *this ).InitSignal();
}

EvasPlugin::EvasPluginSignalType& EvasPlugin::TerminateSignal()
{
  return Internal::GetImplementation( *this ).TerminateSignal();
}

EvasPlugin::EvasPluginSignalType& EvasPlugin::PauseSignal()
{
  return Internal::GetImplementation( *this ).PauseSignal();
}

EvasPlugin::EvasPluginSignalType& EvasPlugin::ResumeSignal()
{
  return Internal::GetImplementation( *this ).ResumeSignal();
}

EvasPlugin::EvasPluginSignalType& EvasPlugin::ResizeSignal()
{
  return Internal::GetImplementation( *this ).ResizeSignal();
}

EvasPlugin::EvasPluginSignalType& EvasPlugin::FocusedSignal()
{
  return Internal::GetImplementation( *this ).FocusedSignal();
}

EvasPlugin::EvasPluginSignalType& EvasPlugin::UnFocusedSignal()
{
  return Internal::GetImplementation( *this ).UnFocusedSignal();
}

EvasPlugin::EvasPlugin( Internal::EvasPlugin* evasPlugin )
: BaseHandle( evasPlugin )
{
}

}  // namespace Extension

}  // namespace Dali
