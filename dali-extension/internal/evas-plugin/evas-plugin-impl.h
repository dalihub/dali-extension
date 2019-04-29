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
#include <Elementary.h>
#include <Ecore_IMF_Evas.h>
#include <tbm_surface.h>

#include <dali/public-api/dali-core.h>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/common/stage.h>
#include <dali/public-api/signals/connection-tracker.h>
#include <dali/public-api/adaptor-framework/timer.h>

#include <dali/devel-api/adaptor-framework/singleton-service.h>
#include <dali/devel-api/adaptor-framework/clipboard.h>
#include <dali/devel-api/adaptor-framework/clipboard-event-notifier.h>
#include <dali/devel-api/adaptor-framework/accessibility-adaptor.h>


#include <dali/integration-api/debug.h>
#include <dali/integration-api/adaptors/adaptor.h>
#include <dali/integration-api/adaptors/native-render-surface.h>
#include <dali/integration-api/adaptors/native-render-surface-factory.h>
#include <dali/integration-api/adaptors/trigger-event-factory.h>
#ifdef DALI_EVASPLUGIN_USE_IMF_MANAGER
#include <dali/integration-api/adaptors/imf-manager-impl.h>
#endif

#include <dali-toolkit/public-api/accessibility-manager/accessibility-manager.h>


#ifdef ENABLE_TTRACE
#include <ttrace.h>
#endif


// INTERNAL INCLUDES
#include "evas-event-handler.h"
#include "ecore-wl-event-handler.h"
#include "evas-plugin-event-interface.h"
#include "evas-plugin-visibility-interface.h"

#include <dali-extension/devel-api/evas-plugin/evas-plugin.h>

namespace Dali
{
class TriggerEventInterface;
class NativeRenderSurface;

namespace Extension
{

namespace Internal
{

typedef Rect<int> PositionSize;

class EvasPlugin;
typedef IntrusivePtr<EvasPlugin> EvasPluginPtr;

/**
 * Implementation of the EvasPlugin class.
 */
class EvasPlugin : public BaseObject,
                   public ConnectionTracker,
                   public Extension::Internal::EvasPluginEventInterface,
                   public Extension::Internal::EvasPluginVisibilityInterface
{
public:

  typedef Dali::Extension::EvasPlugin::EvasPluginSignalType EvasPluginSignalType;

  /**
   * @brief Create a new evas plugin
   * @param[in] parentEvasObject A pointer of the parent evas object
   * @param[in] width The width of Dali view port
   * @param[in] height The height of Dali view port
   * @param[in] transparent Whether the evas object is transparent or not
   */
  static EvasPluginPtr New(Evas_Object* parentEvasObject, int width, int height, bool transparent);

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

private:

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnEvasObjectTouchEvent
   */
  virtual void OnEvasObjectTouchEvent(TouchPoint& touchPoint, unsigned long timeStamp);

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnEvasObjectWheelEvent
   */
  virtual void OnEvasObjectWheelEvent(WheelEvent& wheelEvent);

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnEvasObjectKeyEvent
   */
  virtual void OnEvasObjectKeyEvent(KeyEvent& keyEvent);

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnEvasObjectMove
   */
  virtual void OnEvasObjectMove(const Rect<int>& geometry);

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnEvasObjectResize
   */
  virtual void OnEvasObjectResize(const Rect<int>& geometry);

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnEvasObjectFocusIn
   */
  virtual void OnEvasObjectFocusIn();

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnEvasObjectFocusOut
   */
  virtual void OnEvasObjectFocusOut();

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnEvasRenderPost
   */
  virtual void OnEvasPostRender();

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnElmAccessActionHighlight
   */
  virtual bool OnElmAccessActionHighlight(Dali::Extension::Internal::AccessActionInfo& accessActionInfo);

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnElmAccessActionUnhighlight
   */
  virtual bool OnElmAccessActionUnhighlight(Dali::Extension::Internal::AccessActionInfo& accessActionInfo);

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnElmAccessActionHighlightNext
   */
  virtual bool OnElmAccessActionHighlightNext(Dali::Extension::Internal::AccessActionInfo& accessActionInfo);

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnElmAccessActionHighlightPrev
   */
  virtual bool OnElmAccessActionHighlightPrev(Dali::Extension::Internal::AccessActionInfo& accessActionInfo);

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnElmAccessActionActivate
   */
  virtual bool OnElmAccessActionActivate(Dali::Extension::Internal::AccessActionInfo& accessActionInfo);

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnElmAccessActionUp
   */
  virtual bool OnElmAccessActionUp(Dali::Extension::Internal::AccessActionInfo& accessActionInfo);

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnElmAccessActionDown
   */
  virtual bool OnElmAccessActionDown(Dali::Extension::Internal::AccessActionInfo& accessActionInfo);

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnElmAccessActionScroll
   */
  virtual bool OnElmAccessActionScroll(Dali::Extension::Internal::AccessActionInfo& accessActionInfo);

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface:OnElmAccessActionBack:
   */
  virtual bool OnElmAccessActionBack(Dali::Extension::Internal::AccessActionInfo& accessActionInfo);

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginEventInterface::OnElmAccessActionRead
   */
  virtual bool OnElmAccessActionRead(Dali::Extension::Internal::AccessActionInfo& accessActionInfo);

  /**
   * @copydoc Dali::Extension::Internal::EvasPluginVisibilityInterface::OnElmEcoreWlVisibility
   */
  virtual void OnEcoreWlVisibility(bool visibility);

  /**
   * Resize the surface
   * To resize, create new surface with evas object's size and replace with it.
   */
  void ResizeSurface();

#ifdef DALI_EVASPLUGIN_USE_IMF_MANAGER
  /**
   * For ImfActivated signal
   * When the imf is activated, it will handle the focus
   * @param[in] imfManager imfManager instance
   */
  void OnImfActivated(ImfManager& imfManager);
#endif

  /**
   * This function is called after drawing by dali.
   */
  void OnPostRender();

  /**
   * Called when the accessibility action event dispatched from elm_access.
   * @param[in] actionType elm accessibility action type structure
   * @param[in] x x position for action, it could be unnecessary
   * @param[in] y y position for action, it could be unnecessary
   * @return True if the event was handled
   */
  bool OnElmAccessibilityActionEvent(AccessActionInfo& actionInfo, int x = -1, int y = -1);

private:
  /**
   * Private constructor
   * @param[in] parentEvasObject A pointer of the parent evas object
   * @param[in] width The width of Dali view port
   * @param[in] height The height of Dali view port
   * @param[in] transparent Whether the evas object is transparent or not
   */
  EvasPlugin(Evas_Object* parentEvasObject, int width, int height, bool transparent);

  /**
   * Destructor
   */
  virtual ~EvasPlugin();

  // Undefined
  EvasPlugin(const EvasPlugin&);
  EvasPlugin& operator=(EvasPlugin&);

private:
  enum State
  {
    READY,
    RUNNING,
    SUSPENDED,
    STOPPED,
  };

  State mState;
  NativeRenderSurface* mTBMRenderSurface;
  Adaptor* mAdaptor;
  TriggerEventInterface* mRenderNotification;
  Extension::Internal::EvasEventHandler* mEvasEventHandler;
  Extension::Internal::EcoreWlEventHandler* mEcoreWlEventHandler;

  bool mIsFocus:1;        ///< Flag
  bool mTransparent:1;    ///< Whether

  SingletonService mSingletonService;

  // Signals
  EvasPluginSignalType mInitSignal;
  EvasPluginSignalType mTerminateSignal;
  EvasPluginSignalType mPauseSignal;
  EvasPluginSignalType mResumeSignal;
  EvasPluginSignalType mResizeSignal;
  EvasPluginSignalType mFocusedSignal;
  EvasPluginSignalType mUnFocusedSignal;

  struct Impl;
  Impl* mImpl;
};

inline EvasPlugin& GetImplementation(Extension::EvasPlugin& evasPlugin)
{
  DALI_ASSERT_ALWAYS(evasPlugin && "evasPluing handle is empty");

  BaseObject& handle = evasPlugin.GetBaseObject();

  return static_cast<EvasPlugin&>(handle);
}

inline const EvasPlugin& GetImplementation(const Extension::EvasPlugin& evasPlugin)
{
  DALI_ASSERT_ALWAYS(evasPlugin && "evasPlugin handle is empty");

  const BaseObject& handle = evasPlugin.GetBaseObject();

  return static_cast<const EvasPlugin&>(handle);
}

} // namespace Internal

} // namespace Extension

} // namespace Dali

#endif // __DALI_EXTENSION_INTERNAL_EVAS_PLUGIN_H__
