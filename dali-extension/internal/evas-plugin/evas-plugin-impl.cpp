/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/native-render-surface.h>

// INTERNAL INCLUDES
#include <dali-extension/internal/evas-plugin/scene-impl.h>

// CLASS HEADER
#include <dali-extension/internal/evas-plugin/evas-plugin-impl.h>

namespace Dali
{

namespace Extension
{

namespace Internal
{

IntrusivePtr< EvasPlugin > EvasPlugin::New( Evas_Object* parentEvasObject, uint16_t width, uint16_t height, bool isTranslucent )
{
  IntrusivePtr< EvasPlugin > evasPlugin = new EvasPlugin( parentEvasObject, width, height, isTranslucent );
  return evasPlugin;
}

EvasPlugin::EvasPlugin( Evas_Object* parentEvasObject, uint16_t width, uint16_t height, bool isTranslucent )
: mState( READY )
{
  DALI_ASSERT_ALWAYS( parentEvasObject && "No parent object for the EvasPlugin." );

  // Generate a default scene
  IntrusivePtr< Internal::Scene > scenePtr = Internal::Scene::New( parentEvasObject, width, height, isTranslucent );

  mDefaultScene = Extension::Scene( scenePtr.Get() );

  // Generate DALi adaptor
  NativeRenderSurface* surface = static_cast<NativeRenderSurface*>( scenePtr->GetSurface() );

  mAdaptor = &Adaptor::New( Dali::Integration::SceneHolder( scenePtr.Get() ), *surface, Configuration::APPLICATION_DOES_NOT_HANDLE_CONTEXT_LOSS );

  // Initialize default scene
  scenePtr->Initialize( this, true );

  scenePtr->ResizedSignal().Connect( this, &EvasPlugin::OnDefaultSceneResized );

  scenePtr->FocusChangedSignal().Connect( this, &EvasPlugin::OnDefaultSceneFocusChanged );
}

EvasPlugin::~EvasPlugin()
{
  delete mAdaptor;
  mAdaptor = nullptr;
}

Adaptor* EvasPlugin::GetAdaptor()
{
  return mAdaptor;
}

void EvasPlugin::Run()
{
  if( READY == mState )
  {
    // Start the adaptor
    mAdaptor->Start();

    mState = RUNNING;

    mPreInitSignal.Emit();

    mInitSignal.Emit();

    mAdaptor->NotifySceneCreated();
  }
}

void EvasPlugin::Pause()
{
  if( mState == RUNNING )
  {
    mState = SUSPENDED;

    mAdaptor->Pause();

    mPauseSignal.Emit();
  }
}

void EvasPlugin::Resume()
{
  if( mState == SUSPENDED )
  {
    mAdaptor->Resume();

    mResumeSignal.Emit();

    mState = RUNNING;
  }
}

void EvasPlugin::Stop()
{
  if( mState != STOPPED )
  {
    // Stop the adaptor
    mAdaptor->Stop();
    mState = STOPPED;

    mTerminateSignal.Emit();
  }
}

Extension::Scene EvasPlugin::GetDefaultScene()
{
  return mDefaultScene;
}

Evas_Object* EvasPlugin::GetAccessEvasObject()
{
  return mDefaultScene.GetAccessEvasObject();
}

Evas_Object* EvasPlugin::GetDaliEvasObject()
{
  return mDefaultScene.GetDaliEvasObject();
}

void EvasPlugin::OnDefaultSceneResized( Extension::Scene defaultScene, uint16_t width, uint16_t height )
{
  mResizeSignal.Emit();
}

void EvasPlugin::OnDefaultSceneFocusChanged( Extension::Scene defaultScene, bool focused )
{
  if( focused )
  {
    mFocusedSignal.Emit();
  }
  else
  {
    mUnFocusedSignal.Emit();
  }
}

} // namespace Internal

} // namespace Extension

} // namespace Dali
