#ifndef DALI_EXTENSION_INTERNAL_EVAS_PLUGIN_H
#define DALI_EXTENSION_INTERNAL_EVAS_PLUGIN_H

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

// EXTERNAL INCLUDES
#include <memory>
#include <Ecore_IMF_Evas.h>

#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/signals/connection-tracker.h>

// INTERNAL INCLUDES
#include <dali-extension/devel-api/evas-plugin/evas-plugin.h>
#include <dali-extension/devel-api/evas-plugin/scene.h>

namespace Dali
{
class Adaptor;

namespace Extension
{

namespace Internal
{

/**
 * Implementation of the EvasPlugin class.
 */
class EvasPlugin : public BaseObject,
                   public ConnectionTracker
{
public:

  typedef Dali::Extension::EvasPlugin::EvasPluginSignalType EvasPluginSignalType;

  /**
   * @brief Create a new evas plugin
   * @param[in] parentEvasObject A pointer of the parent evas object
   * @param[in] width The width of Dali view port
   * @param[in] height The height of Dali view port
   * @param[in] isTranslucent Whether the evas object is translucent or not
   */
  static IntrusivePtr<EvasPlugin> New( Evas_Object* parentEvasObject, int width, int height, bool isTranslucent );

public:

  /**
   * @copydoc Dali::Extension::EvasPlugin::Start()
   */
  void Run();

  /**
   * @copydoc Dali::Extension::EvasPlugin::Pause()
   */
  void Pause();

  /**
   * @copydoc Dali::Extension::EvasPlugin::Resume()
   */
  void Resume();

  /**
   * @copydoc Dali::Extension::EvasPlugin::Stop()
   */
  void Stop();

  /**
   * @copydoc Dali::Extension::EvasPlugin::GetDefaultScene()
   */
  Extension::Scene GetDefaultScene();

  /**
   * @copydoc Dali::Extension::EvasPlugin::GetAccessEvasObject()
   */
  Evas_Object* GetAccessEvasObject();

  /**
   * @copydoc Dali::Extension::EvasPlugin::GetDaliEvasObject()
   */
  Evas_Object* GetDaliEvasObject();

  /**
   * @copydoc Dali::Extension::EvasPlugin::InitSignal()
   */
  EvasPluginSignalType& InitSignal()
  {
    return mInitSignal;
  }

  /**
   * @copydoc Dali::Extension::EvasPlugin::TerminateSignal()
   */
  EvasPluginSignalType& TerminateSignal()
  {
    return mTerminateSignal;
  }

  /**
   * @copydoc Dali::Extension::EvasPlugin::PauseSignal()
   */
  EvasPluginSignalType& PauseSignal()
  {
   return mPauseSignal;
  }

  /**
   * @copydoc Dali::Extension::EvasPlugin::ResumeSignal()
   */
  EvasPluginSignalType& ResumeSignal()
  {
    return mResumeSignal;
  }

  /**
   * @copydoc Dali::Extension::EvasPlugin::ResizeSignal()
   */
  EvasPluginSignalType& ResizeSignal()
  {
    return mResizeSignal;
  }

  /**
   * @copydoc Dali::Extension::EvasPlugin::FocusedSignal()
   */
  EvasPluginSignalType& FocusedSignal()
  {
    return mFocusedSignal;
  }

  /**
   * @copydoc Dali::Extension::EvasPlugin::UnFocusedSignal()
   */
  EvasPluginSignalType& UnFocusedSignal()
  {
    return mUnFocusedSignal;
  }

public:
  /**
   * All methods here are for the internal use
   */

  enum State
  {
    READY,
    RUNNING,
    SUSPENDED,
    STOPPED,
  };

  /**
   * @brief This is for internal use to get notified before InitSignal() emits
   */
  EvasPluginSignalType& PreInitSignal()
  {
    return mPreInitSignal;
  }

  /**
   * @brief Get EvasPlugin's current state
   *
   * @return The state
   */
  const State GetState() const
  {
    return mState;
  }

  /*
   * @bried Get currently working Adaptor. It can be null.
   */
  Adaptor* GetAdaptor();

private:
  /**
   * Private constructor
   * @param[in] parentEvasObject A pointer of the parent evas object
   * @param[in] width The width of Dali view port
   * @param[in] height The height of Dali view port
   * @param[in] isTranslucent Whether the evas object is translucent or not
   */
  EvasPlugin( Evas_Object* parentEvasObject, int width, int height, bool isTranslucent );

  /**
   * Destructor
   */
  virtual ~EvasPlugin();

  // Undefined
  EvasPlugin( const EvasPlugin& );
  EvasPlugin& operator=( EvasPlugin& );

  /**
   * This callback is for supporting legacy API EvasPlugin::ResizeSignal
   */
  void OnDefaultSceneResized( Extension::Scene defaultScene, int width, int height );

  /**
   * This callback is for supporting legacy API EvasPlugin::FocusedSignal, EvasPlugin::UnFocusedSignal
   */
  void OnDefaultSceneFocusChanged( Extension::Scene defaultScene, bool focused );

private:

  Adaptor*                                  mAdaptor;
  Extension::Scene                          mDefaultScene;

  EvasPluginSignalType                      mPreInitSignal;
  EvasPluginSignalType                      mInitSignal;
  EvasPluginSignalType                      mTerminateSignal;
  EvasPluginSignalType                      mPauseSignal;
  EvasPluginSignalType                      mResumeSignal;
  EvasPluginSignalType                      mResizeSignal;    // Connect to DefaultScene.ResizedSignal
  EvasPluginSignalType                      mFocusedSignal;   // Connect to DefaultScene.FocusChangedSignal
  EvasPluginSignalType                      mUnFocusedSignal; // Connect to DefaultScene.FocusChangedSignal

  State                                     mState;
};

inline EvasPlugin& GetImplementation( Extension::EvasPlugin& evasPlugin )
{
  DALI_ASSERT_ALWAYS( evasPlugin && "EvasPlugin handle is empty" );

  BaseObject& handle = evasPlugin.GetBaseObject();

  return static_cast<EvasPlugin&>( handle );
}

inline const EvasPlugin& GetImplementation( const Extension::EvasPlugin& evasPlugin )
{
  DALI_ASSERT_ALWAYS( evasPlugin && "EvasPlugin handle is empty" );

  const BaseObject& handle = evasPlugin.GetBaseObject();

  return static_cast<const EvasPlugin&>( handle );
}

} // namespace Internal

} // namespace Extension

} // namespace Dali

#endif // DALI_EXTENSION_INTERNAL_EVAS_PLUGIN_H
