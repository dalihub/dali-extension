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
#include "evas-plugin-impl.h"

namespace Dali
{

namespace Extension
{

namespace Internal
{

namespace
{
const char* IMAGE_EVAS_OBJECT_NAME = "dali-evas-plugin";
const char* ELM_OBJECT_STYLE = "transparent";
const char* ELM_OBJECT_CONTAINER_PART_NAME = "elm.swallow.content";
}

struct EvasPlugin::Impl
{
  Impl(EvasPlugin* evasPlugin, Evas_Object* parentEvasObject, int width, int height, bool transparent)
  : mEvasPlugin(evasPlugin)
  , mEcoreEvas(NULL)
  , mImageEvasObject(NULL)
  , mDaliAccessEvasObject(NULL)
  , mDaliEvasObject(NULL)
  {
    Evas* evas = evas_object_evas_get(parentEvasObject);
    mEcoreEvas = ecore_evas_ecore_evas_get(evas);

    // Create the image evas object
    mImageEvasObject = evas_object_image_filled_add(evas);
    evas_object_name_set(mImageEvasObject, IMAGE_EVAS_OBJECT_NAME);
    evas_object_image_content_hint_set(mImageEvasObject, EVAS_IMAGE_CONTENT_HINT_DYNAMIC);
    evas_object_image_alpha_set(mImageEvasObject, transparent ? EINA_TRUE : EINA_FALSE);
    evas_object_image_size_set(mImageEvasObject, width, height);
    evas_object_size_hint_weight_set(mImageEvasObject, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(mImageEvasObject, EVAS_HINT_FILL, EVAS_HINT_FILL);

    // Register the elm access to image evas object
    mDaliAccessEvasObject = elm_access_object_register(mImageEvasObject, parentEvasObject);

    // Create a button and set style as "focus", if does not want to show the focus, then "transparent"
    mDaliEvasObject = elm_button_add(parentEvasObject);

    // Don't need to show the focus boundary here
    elm_object_style_set(mDaliEvasObject, ELM_OBJECT_STYLE);

    // Set the image evas object to focus object, but event should not be propagated
    elm_object_part_content_set(mDaliEvasObject, ELM_OBJECT_CONTAINER_PART_NAME, mImageEvasObject);
    evas_object_propagate_events_set(mImageEvasObject, EINA_FALSE);

    // Set the evas object you want to make focusable as the content of the swallow part
    evas_object_size_hint_weight_set(mDaliEvasObject, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(mDaliEvasObject, EVAS_HINT_FILL, EVAS_HINT_FILL);

    evas_object_move(mDaliEvasObject, 0, 0);
    evas_object_resize(mDaliEvasObject, width, height);
    evas_object_show(mDaliEvasObject);
  }

  ~Impl()
  {
    // Delete the elm focus evas object
    evas_object_del(mDaliEvasObject);
    mDaliEvasObject = NULL;

    // Unregister elm_access_object
    elm_access_object_unregister(mImageEvasObject);
    mDaliAccessEvasObject = NULL;

    // Delete the image evas object
    evas_object_del(mImageEvasObject);
    mImageEvasObject = NULL;
  }

  Ecore_Wl2_Window* GetNativeWindow()
  {
    return ecore_evas_wayland2_window_get( mEcoreEvas );
  }

  PositionSize GetGeometry()
  {
    PositionSize geometry;
    evas_object_geometry_get(mImageEvasObject, &geometry.x, &geometry.y, &geometry.width, &geometry.height);
    return geometry;
  }

  void SetFocus()
  {
    evas_object_focus_set(mImageEvasObject, EINA_TRUE);
  }

  void BindTBMSurface( tbm_surface_h surface )
  {
#ifdef ENABLE_TTRACE
    traceBegin( TTRACE_TAG_GRAPHICS, "EvasNativeSurfaceSet[%d]", pixmap );
#endif

    Evas_Native_Surface nativeSurface;
    nativeSurface.type = EVAS_NATIVE_SURFACE_TBM;
    nativeSurface.version = EVAS_NATIVE_SURFACE_VERSION;
    nativeSurface.data.tbm.buffer = surface;
    nativeSurface.data.tbm.rot = 0;
    nativeSurface.data.tbm.ratio = 0;
    nativeSurface.data.tbm.flip = 0;

    evas_object_image_native_surface_set( mImageEvasObject, &nativeSurface );

#ifdef ENABLE_TTRACE
    traceEnd( TTRACE_TAG_GRAPHICS );
#endif
  }

  void RequestRender()
  {
    evas_object_image_pixels_dirty_set( mImageEvasObject, EINA_TRUE );
    ecore_evas_manual_render( mEcoreEvas );
  }

  EvasPlugin* mEvasPlugin;
  Ecore_Evas* mEcoreEvas;
  Evas_Object* mImageEvasObject;
  Evas_Object* mDaliAccessEvasObject;
  Evas_Object* mDaliEvasObject;
};

EvasPluginPtr EvasPlugin::New(Evas_Object* parentEvasObject, int width, int height, bool transparent)
{
  EvasPluginPtr evasPlugin = new EvasPlugin(parentEvasObject, width, height, transparent);
  return evasPlugin;
}

EvasPlugin::EvasPlugin(Evas_Object* parentEvasObject, int width, int height, bool transparent)
: mState(READY)
, mTBMRenderSurface(NULL)
, mAdaptor(NULL)
, mRenderNotification(NULL)
, mEvasEventHandler(NULL)
, mEcoreWlEventHandler(NULL)
, mIsFocus(false)
, mTransparent(transparent)
, mImpl(NULL)
{
  DALI_ASSERT_ALWAYS( parentEvasObject && "No parent object for the evas plugin." );

  mImpl = new Impl(this, parentEvasObject, width, height, transparent);

#ifdef DALI_PREVENT_TIZEN_40_FEATURE
  // Create the singleton service
  mSingletonService = SingletonService::New();

  // Create the pixmap render surface
  mTBMRenderSurface = CreateNativeSurface( PositionSize( 0, 0, width, height ), transparent );

  // Initialize dali
  Any nativeWindow = mImpl->GetNativeWindow();
  mAdaptor = &Adaptor::New(nativeWindow, *mTBMRenderSurface, Configuration::APPLICATION_DOES_NOT_HANDLE_CONTEXT_LOSS);
#endif

#ifdef DALI_EVASPLUGIN_USE_IMF_MANAGER
  ImfManager imfManager = ImfManager::Get();
  if(imfManager)
  {
    imfManager.ActivatedSignal().Connect(this, &EvasPlugin::OnImfActivated);
  }
#endif

#ifdef DALI_PREVENT_TIZEN_40_FEATURE
  TriggerEventFactory triggerEventFactory;
  mRenderNotification = triggerEventFactory.CreateTriggerEvent(MakeCallback(this, &EvasPlugin::OnPostRender),
                                                               TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER);

  mTBMRenderSurface->SetRenderNotification(mRenderNotification);
#endif
}

EvasPlugin::~EvasPlugin()
{
#ifdef DALI_PREVENT_TIZEN_40_FEATURE
  mAdaptor->Stop();

  if (mEcoreWlEventHandler)
  {
    delete mEcoreWlEventHandler;
    mEcoreWlEventHandler = NULL;
  }

  if (mEvasEventHandler)
  {
    delete mEvasEventHandler;
    mEvasEventHandler = NULL;
  }

  delete mRenderNotification;
  mRenderNotification = NULL;

  // the singleton service should be unregistered before adaptor deletion
  mSingletonService.UnregisterAll();

  delete mAdaptor;
  mAdaptor = NULL;

  delete mTBMRenderSurface;
  mTBMRenderSurface = NULL;

  delete mImpl;
  mImpl = NULL;
#endif
}

void EvasPlugin::Run()
{
#ifdef DALI_PREVENT_TIZEN_40_FEATURE
  if(READY == mState)
  {
    if (!mEvasEventHandler)
    {
      mEvasEventHandler = new Extension::Internal::EvasEventHandler(mImpl->mImageEvasObject, mImpl->mDaliAccessEvasObject, mImpl->mDaliEvasObject, *this);
    }

    // Start the adaptor
    mAdaptor->Start();

    mInitSignal.Emit();

    mAdaptor->NotifySceneCreated();

    mState = RUNNING;

    DALI_LOG_RELEASE_INFO("EvasPlugin::Run");
  }
#endif
}

void EvasPlugin::Pause()
{
  if(mState == RUNNING)
  {
    mState = SUSPENDED;

    mAdaptor->Pause();

    mPauseSignal.Emit();

    DALI_LOG_RELEASE_INFO("EvasPlugin::Pause");
  }
}

void EvasPlugin::Resume()
{
  if(mState == SUSPENDED)
  {
    mAdaptor->Resume();

    mResumeSignal.Emit();

    mState = RUNNING;

    DALI_LOG_RELEASE_INFO("EvasPlugin::Resume");
  }
}

void EvasPlugin::Stop()
{
#ifdef DALI_PREVENT_TIZEN_40_FEATURE
  if(mState != STOPPED)
  {
    // Stop the adaptor
    mAdaptor->Stop();
    mState = STOPPED;

    mTerminateSignal.Emit();

    DALI_LOG_RELEASE_INFO("EvasPlugin::Stop");
  }
#endif
}

Evas_Object* EvasPlugin::GetDaliAccessEvasObject()
{
  return mImpl->mDaliAccessEvasObject;
}

Evas_Object* EvasPlugin::GetDaliEvasObject()
{
  return mImpl->mDaliEvasObject;
}

void EvasPlugin::ResizeSurface()
{
#if 0
  // Remember old surface
  NativeRenderSurface* oldSurface = mTBMRenderSurface;
  TriggerEventInterface* oldTriggerEvent = mRenderNotification;

  PositionSize geometry = mImpl->GetGeometry();

  mAdaptor->SurfaceSizeChanged( geometry );

  // emit resized signal to application
  mResizeSignal.Emit();

  mTBMRenderSurface = CreateNativeSurface( PositionSize( 0, 0, geometry.width, geometry.height ), NATIVE_SURFACE_NAME, mTransparent );
  mTBMRenderSurface->SetRenderNotification( mRenderNotification );

  // Ask the replace the surface inside dali
  Any nativeWindow = mImpl->GetNativeWindow();

  mAdaptor->ReplaceSurface( nativeWindow, *mTBMRenderSurface ); // this method is synchronous => guarantee until rendering next frame

  TriggerEventFactory triggerEventFactory;
  mRenderNotification = triggerEventFactory.CreateTriggerEvent( MakeCallback( this, &EvasPlugin::OnPostRender ),
                                                                TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER );

  mTBMRenderSurface->SetRenderNotification( mRenderNotification );
  delete oldTriggerEvent;

  // Bind offscreen surface to the evas object
  mTBMRenderSurface->WaitUntilSurfaceReplaced();
  mImpl->BindTBMSurface( AnyCast<tbm_surface_h>( mTBMRenderSurface->GetDrawable() ) );
  mAdaptor->ReleaseSurfaceLock();

  // It's now safe to delete the old surface
  delete oldSurface;
#endif
}

#ifdef DALI_EVASPLUGIN_USE_IMF_MANAGER
void EvasPlugin::OnImfActivated(ImfManager& imfManager)
{
  mImpl->SetFocus();
}
#endif

void EvasPlugin::OnPostRender()
{
  // Bind offscreen surface to the evas object
  mImpl->BindTBMSurface(AnyCast<tbm_surface_h>(mTBMRenderSurface->GetDrawable()));
  mAdaptor->ReleaseSurfaceLock();

  mImpl->RequestRender();
}

void EvasPlugin::OnEvasObjectTouchEvent(TouchPoint& touchPoint, unsigned long timeStamp)
{
  mAdaptor->FeedTouchPoint(touchPoint, timeStamp);
}

void EvasPlugin::OnEvasObjectWheelEvent(WheelEvent& wheelEvent)
{
  mAdaptor->FeedWheelEvent( wheelEvent );
}

void EvasPlugin::OnEvasObjectKeyEvent(KeyEvent& keyEvent)
{
  mAdaptor->FeedKeyEvent( keyEvent );
}

void EvasPlugin::OnEvasObjectMove(const Rect<int>& geometry)
{
}

void EvasPlugin::OnEvasObjectResize(const Rect<int>& geometry)
{
  DALI_LOG_RELEASE_INFO("EvasPlugin::OnEvasObjectResize (%d x %d)", geometry.width, geometry.height);

  if (geometry.width <= 1 || geometry.height <= 1)
  {
    // skip meaningless resize signal
    return;
  }

  Vector2 size = Stage::GetCurrent().GetSize();
  if(size.width == geometry.width && size.height == geometry.height)
  {
    // skip meaningless resize signal
    return;
  }

  ResizeSurface();
}

void EvasPlugin::OnEvasObjectFocusIn()
{
  if(!mIsFocus)
  {
    if (!mEcoreWlEventHandler)
    {
      mEcoreWlEventHandler = new Extension::Internal::EcoreWlEventHandler(mImpl->GetNativeWindow(), *this);
    }

#ifdef DALI_EVASPLUGIN_USE_IMF_MANAGER
    ImfManager imfManager = ImfManager::Get();
    if(imfManager)
    {
      // TODO : Move to Impl
      Ecore_Wl_Window* window = mImpl->GetNativeWindow();
      Ecore_IMF_Context* imfContext = Dali::Internal::Adaptor::ImfManager::GetImplementation(imfManager).GetContext();
      ecore_imf_context_client_window_set( imfContext, reinterpret_cast<void*>( window ) );

      if(imfManager.RestoreAfterFocusLost())
      {
        imfManager.Activate();
      }
    }
#endif

    mFocusedSignal.Emit();

    mIsFocus = true;

    DALI_LOG_RELEASE_INFO("EvasPlugin::OnEvasObjectFocusIn");
  }
}

void EvasPlugin::OnEvasObjectFocusOut()
{
  if(mIsFocus)
  {
    mIsFocus = false;

    if (mEcoreWlEventHandler)
    {
      delete mEcoreWlEventHandler;
      mEcoreWlEventHandler = NULL;
    }

#ifdef DALI_EVASPLUGIN_USE_IMF_MANAGER
    ImfManager imfManager = ImfManager::Get();
    if(imfManager && imfManager.RestoreAfterFocusLost())
    {
      imfManager.Deactivate();
    }
#endif

    Clipboard::Get().HideClipboard();

    mUnFocusedSignal.Emit();

    DALI_LOG_RELEASE_INFO("EvasPlugin::OnEvasObjectFocusOut");
  }
}

void EvasPlugin::OnEvasPostRender()
{
}

bool EvasPlugin::OnElmAccessActionHighlight(AccessActionInfo& accessActionInfo)
{
  return this->OnElmAccessibilityActionEvent(accessActionInfo);
}

bool EvasPlugin::OnElmAccessActionUnhighlight(AccessActionInfo& accessActionInfo)
{
  return this->OnElmAccessibilityActionEvent(accessActionInfo);
}

bool EvasPlugin::OnElmAccessActionHighlightNext(AccessActionInfo& accessActionInfo)
{
  return this->OnElmAccessibilityActionEvent(accessActionInfo);
}

bool EvasPlugin::OnElmAccessActionHighlightPrev(AccessActionInfo& accessActionInfo)
{
  return this->OnElmAccessibilityActionEvent(accessActionInfo);
}

bool EvasPlugin::OnElmAccessActionActivate(AccessActionInfo& accessActionInfo)
{
  return this->OnElmAccessibilityActionEvent(accessActionInfo);
}

bool EvasPlugin::OnElmAccessActionUp(AccessActionInfo& accessActionInfo)
{
  return this->OnElmAccessibilityActionEvent(accessActionInfo);
}

bool EvasPlugin::OnElmAccessActionDown(AccessActionInfo& accessActionInfo)
{
  return this->OnElmAccessibilityActionEvent(accessActionInfo);
}

bool EvasPlugin::OnElmAccessActionScroll(AccessActionInfo& accessActionInfo)
{
  bool ret = false;

  Evas_Coord rel_x, rel_y;
  Evas_Coord obj_x,  obj_y, obj_w, obj_h;
  Evas_Object* eo = this->GetDaliAccessEvasObject();

  if(eo)
  {
    evas_object_geometry_get(eo, &obj_x,  &obj_y, &obj_w, &obj_h);

    rel_x = accessActionInfo.x - obj_x;
    rel_y = accessActionInfo.y - obj_y;

    ret = this->OnElmAccessibilityActionEvent(accessActionInfo, rel_x, rel_y);
  }

  return ret;
}

bool EvasPlugin::OnElmAccessActionBack(AccessActionInfo& accessActionInfo)
{
  return this->OnElmAccessibilityActionEvent(accessActionInfo);
}

bool EvasPlugin::OnElmAccessActionRead(AccessActionInfo& accessActionInfo)
{
  return this->OnElmAccessibilityActionEvent(accessActionInfo);
}

void EvasPlugin::OnEcoreWlVisibility(bool visibility)
{
  if (!visibility)
  {
    mAdaptor->ReleaseSurfaceLock();
  }
}

bool EvasPlugin::OnElmAccessibilityActionEvent(AccessActionInfo& accessActionInfo, int x, int y)
{
  bool ret = false;

  if( NULL == mAdaptor)
  {
    return ret;
  }

  Dali::AccessibilityAdaptor accessibilityAdaptor = Dali::AccessibilityAdaptor::Get();
  if( accessibilityAdaptor )
  {
    int touchType = accessActionInfo.mouseType;
    int touchX = x >= 0 ? x : accessActionInfo.x;
    int touchY = y >= 0 ? y : accessActionInfo.y;

    switch(accessActionInfo.actionBy)
    {
      case Dali::Extension::Internal::ACCESS_ACTION_HIGHLIGHT:
      case Dali::Extension::Internal::ACCESS_ACTION_READ:
      {
        ret = accessibilityAdaptor.HandleActionReadEvent((unsigned int)x, (unsigned int)y, true);
      }
      break;

      case Dali::Extension::Internal::ACCESS_ACTION_HIGHLIGHT_PREV:
      {
        // if accessActionInfo.highlight_end is true, need to handle end_of_list sound feedback
        ret = accessibilityAdaptor.HandleActionPreviousEvent(accessActionInfo.highlightCycle);
        if(!ret)
        {
          // when focus moving was failed, clear the focus
          accessibilityAdaptor.HandleActionClearFocusEvent();
        }
      }
      break;

      case Dali::Extension::Internal::ACCESS_ACTION_HIGHLIGHT_NEXT:
      {
        // if accessActionInfo.highlight_cycle is true, need to handle end_of_list sound feedback
        ret = accessibilityAdaptor.HandleActionNextEvent(accessActionInfo.highlightCycle);
        if(!ret)
        {
          // when focus moving was failed, clear the focus
          accessibilityAdaptor.HandleActionClearFocusEvent();
        }
      }
      break;

      case Dali::Extension::Internal::ACCESS_ACTION_ACTIVATE:
      {
        ret = accessibilityAdaptor.HandleActionActivateEvent();
      }
      break;

      case Dali::Extension::Internal::ACCESS_ACTION_UNHIGHLIGHT:
      {
        ret = accessibilityAdaptor.HandleActionClearFocusEvent();
      }
      break;

      case Dali::Extension::Internal::ACCESS_ACTION_SCROLL:
      {
        TouchPoint::State state(TouchPoint::Down);

        if (touchType == 0)
        {
          state = TouchPoint::Down; // mouse down
        }
        else if (touchType == 1)
        {
          state = TouchPoint::Motion; // mouse move
        }
        else if (touchType == 2)
        {
          state = TouchPoint::Up; // mouse up
        }
        else
        {
          state = TouchPoint::Interrupted; // error
        }

        // Send touch event to accessibility manager.
        TouchPoint point( 0, state, (float)touchX, (float)touchY );
        ret = accessibilityAdaptor.HandleActionScrollEvent(point, accessActionInfo.timeStamp);
      }
      break;

      case Dali::Extension::Internal::ACCESS_ACTION_UP:
      {
        ret = accessibilityAdaptor.HandleActionUpEvent();
      }
      break;

      case Dali::Extension::Internal::ACCESS_ACTION_DOWN:
      {
        ret = accessibilityAdaptor.HandleActionDownEvent();
      }
      break;

        case Dali::Extension::Internal::ACCESS_ACTION_BACK:
      default:
      {
        DALI_LOG_WARNING("[%s:%d]\n", __FUNCTION__, __LINE__);
      }

      break;
    }
  }
  else
  {
    DALI_LOG_WARNING("[%s:%d]\n", __FUNCTION__, __LINE__);
  }

  DALI_LOG_INFO(gEvasPluginLogFilter, Debug::General, "[%s:%d] [action : %d] focus manager returns %s\n", __FUNCTION__, __LINE__, (int)(actionType), ret?"TRUE":"FALSE");

  return ret;
}

} // namespace Internal

} // namespace Extension

} // namespace Dali
