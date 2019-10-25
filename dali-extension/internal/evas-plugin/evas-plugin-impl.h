#ifndef __DALI_EXTENSION_INTERNAL_EVAS_PLUGIN_H__
#define __DALI_EXTENSION_INTERNAL_EVAS_PLUGIN_H__

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

#include <dali/devel-api/adaptor-framework/singleton-service.h>
#include <dali/integration-api/adaptor-framework/scene-holder-impl.h>
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/signals/connection-tracker.h>

// INTERNAL INCLUDES
#include <dali-extension/internal/evas-plugin/evas-event-interface.h>
#include <dali-extension/devel-api/evas-plugin/evas-plugin.h>

namespace Dali
{
class Adaptor;
class NativeRenderSurface;
class TriggerEventInterface;

namespace Extension
{

namespace Internal
{

class EvasPluginEventHandler;
class EvasWrapper;

/**
 * Implementation of the EvasPlugin class.
 */
class EvasPlugin : public Dali::Internal::Adaptor::SceneHolder,
                   public ConnectionTracker,
                   public Extension::Internal::EvasPluginEventInterface
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
   * @brief Gets the native handle.
   * @note When users call this function, it wraps the actual type used by the underlying system.
   * @return The native handle or an empty handle
   */
  Dali::Any GetNativeHandle() const override;

  /**
   * @brief Get the native render surface
   * @return The render surface
   */
  NativeRenderSurface* GetNativeRenderSurface() const;

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

  /*
   * @bried Initialize EvasPlugin
   */
  void Initialize();

private:

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::GetEvasWrapper
   */
  EvasWrapper* GetEvasWrapper() const override;

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnEvasObjectTouchEvent
   */
  void OnEvasObjectTouchEvent( Dali::Integration::Point& touchPoint, unsigned long timeStamp ) override;

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnEvasObjectWheelEvent
   */
  void OnEvasObjectWheelEvent( Dali::Integration::WheelEvent& wheelEvent ) override;

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnEvasObjectKeyEvent
   */
  void OnEvasObjectKeyEvent( Dali::Integration::KeyEvent& keyEvent ) override;

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnEvasObjectMove
   */
  void OnEvasObjectMove( const Rect<int>& geometry ) override;

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnEvasObjectResize
   */
  void OnEvasObjectResize( const Rect<int>& geometry ) override;

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnEvasObjectFocusIn
   */
  void OnEvasObjectFocusIn() override;

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnEvasObjectFocusOut
   */
  void OnEvasObjectFocusOut() override;

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnEvasRenderPost
   */
  void OnEvasPostRender() override;

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnElmAccessibilityActionEvent
   */
  bool OnElmAccessibilityActionEvent( AccessActionInfo& actionInfo ) override;

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnEcoreWl2VisibilityChange
   */
  void OnEcoreWl2VisibilityChange( bool visibility ) override;

  /**
   * @brief Resize the surface
   * @param[in] width The width value
   * @param[in] height The height value
   */
  void ResizeSurface( int width, int height );

  /**
   * This function is called after drawing by dali.
   */
  void OnPostRender();

  /**
   * @brief Shows the EvasPlugin if it is hidden.
   */
  void Show();

  /**
   * @brief Hides the EvasPlugin if it is showing.
   */
  void Hide();

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

private:

  enum State
  {
    READY,
    RUNNING,
    SUSPENDED,
    STOPPED,
  };

  static Adaptor*                           mAdaptor;
  static uint32_t                           mEvasPluginCount;
  static SingletonService                   mSingletonService;

  std::unique_ptr< EvasWrapper >            mEvasWrapper;
  std::unique_ptr< TriggerEventInterface >  mRenderNotification;
  std::unique_ptr< EvasPluginEventHandler > mEvasPluginEventHandler;

  EvasPluginSignalType                      mInitSignal;
  EvasPluginSignalType                      mTerminateSignal;
  EvasPluginSignalType                      mPauseSignal;
  EvasPluginSignalType                      mResumeSignal;
  EvasPluginSignalType                      mResizeSignal;
  EvasPluginSignalType                      mFocusedSignal;
  EvasPluginSignalType                      mUnFocusedSignal;

  State                                     mState;
  bool                                      mIsFocus;
  bool                                      mIsTranslucent;
};

inline EvasPlugin& GetImplementation( Extension::EvasPlugin& evasPlugin )
{
  DALI_ASSERT_ALWAYS( evasPlugin && "evasPluing handle is empty" );

  BaseObject& handle = evasPlugin.GetBaseObject();

  return static_cast<EvasPlugin&>( handle );
}

inline const EvasPlugin& GetImplementation( const Extension::EvasPlugin& evasPlugin )
{
  DALI_ASSERT_ALWAYS( evasPlugin && "evasPlugin handle is empty" );

  const BaseObject& handle = evasPlugin.GetBaseObject();

  return static_cast<const EvasPlugin&>( handle );
}

} // namespace Internal

} // namespace Extension

} // namespace Dali

#endif // __DALI_EXTENSION_INTERNAL_EVAS_PLUGIN_H__
