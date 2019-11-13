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
#include <dali/devel-api/adaptor-framework/clipboard.h>
#include <dali/devel-api/adaptor-framework/clipboard-event-notifier.h>
#include <dali/devel-api/adaptor-framework/accessibility-adaptor.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/native-render-surface.h>
#include <dali/integration-api/adaptor-framework/native-render-surface-factory.h>
#include <dali/integration-api/adaptor-framework/trigger-event-factory.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/common/stage.h>
#include <dali-toolkit/public-api/accessibility-manager/accessibility-manager.h>
#include <dali-toolkit/devel-api/focus-manager/keyinput-focus-manager.h>

// INTERNAL INCLUDES
#include <dali-extension/internal/evas-plugin/evas-wrapper.h>
#include <dali-extension/internal/evas-plugin/evas-event-handler.h>

// CLASS HEADER
#include <dali-extension/internal/evas-plugin/evas-plugin-impl.h>

namespace Dali
{

namespace Extension
{

namespace Internal
{

// Initialize static members

Adaptor* EvasPlugin::mAdaptor = nullptr;

uint32_t EvasPlugin::mEvasPluginCount = 0;

SingletonService EvasPlugin::mSingletonService = SingletonService();


IntrusivePtr< EvasPlugin > EvasPlugin::New( Evas_Object* parentEvasObject, int width, int height, bool isTranslucent )
{
  IntrusivePtr< EvasPlugin > evasPlugin = new EvasPlugin( parentEvasObject, width, height, isTranslucent );
  return evasPlugin;
}

EvasPlugin::EvasPlugin( Evas_Object* parentEvasObject, int width, int height, bool isTranslucent )
: mEvasWrapper( new EvasWrapper( parentEvasObject, width, height, isTranslucent ) ),
  mRenderNotification(),
  mEvasPluginEventHandler(),
  mState( READY ),
  mIsFocus( false ),
  mIsTranslucent( isTranslucent )
{
  DALI_ASSERT_ALWAYS( parentEvasObject && "No parent object for the evas plugin." );

  // Increase plugin count.
  mEvasPluginCount++;

  // Create surface
  mSurface = std::unique_ptr< RenderSurfaceInterface >( CreateNativeSurface( PositionSize( 0, 0, width, height ), isTranslucent ) );
}

void EvasPlugin::Initialize()
{
  NativeRenderSurface* surface =  static_cast<NativeRenderSurface*>( mSurface.get() );

  Dali::Integration::SceneHolder sceneHolderHandler = Dali::Extension::EvasPlugin( this );

  if( !mAdaptor )
  {
    // Create the singleton service
    mSingletonService = SingletonService::New();

    // Create an adaptor or add new scene holder to the adaptor
    mAdaptor = &Adaptor::New( sceneHolderHandler, *surface, Configuration::APPLICATION_DOES_NOT_HANDLE_CONTEXT_LOSS );
  }
  else
  {
    mAdaptor->AddWindow( sceneHolderHandler, "", "", mIsTranslucent );
  }

  // Connect callback to be notified when the surface is rendered
  TriggerEventFactory triggerEventFactory;

  mRenderNotification = std::unique_ptr< TriggerEventInterface >( triggerEventFactory.CreateTriggerEvent( MakeCallback( this, &EvasPlugin::OnPostRender ), TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER ) );

  surface->SetRenderNotification( mRenderNotification.get() );
}

EvasPlugin::~EvasPlugin()
{
  mEvasPluginCount--;

  if( !mEvasPluginCount )
  {
    delete mAdaptor;

    mSingletonService.UnregisterAll();
  }
}

void EvasPlugin::Run()
{
  if( READY == mState )
  {
    if( !mEvasPluginEventHandler )
    {
      mEvasPluginEventHandler = std::unique_ptr< EvasPluginEventHandler >( new EvasPluginEventHandler( *this ) );
    }

    // Start the adaptor
    mAdaptor->Start();

    mInitSignal.Emit();

    mAdaptor->NotifySceneCreated();

    mState = RUNNING;
  }
}

void EvasPlugin::Pause()
{
  if( mState == RUNNING )
  {
    mState = SUSPENDED;

    Hide();

    mPauseSignal.Emit();
  }
}

void EvasPlugin::Resume()
{
  if( mState == SUSPENDED )
  {
    Show();

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

Dali::Any EvasPlugin::GetNativeHandle() const
{
  return mEvasWrapper->GetNativeWindow();
}

NativeRenderSurface* EvasPlugin::GetNativeRenderSurface() const
{
  return dynamic_cast< NativeRenderSurface* >( mSurface.get() );
}

Evas_Object* EvasPlugin::GetAccessEvasObject()
{
  return mEvasWrapper->GetAccessibilityTarget();
}

Evas_Object* EvasPlugin::GetDaliEvasObject()
{
  return mEvasWrapper->GetFocusTarget();
}

void EvasPlugin::ResizeSurface( int width, int height )
{
  if( !mSurface || !mAdaptor || width <= 1 || height <= 1 )
  {
    return;
  }

  PositionSize currentSize = mSurface->GetPositionSize();
  if( currentSize.width == width && currentSize.height == height )
  {
    return;
  }

  mSurface->MoveResize( PositionSize( 0, 0, width, height ) );

  SurfaceResized();

  Adaptor::SurfaceSize newSize( width, height );

  // When surface size is updated, inform adaptor of resizing and emit ResizeSignal
  mAdaptor->SurfaceResizePrepare( mSurface.get(), newSize );

  mResizeSignal.Emit();

  mAdaptor->SurfaceResizeComplete( mSurface.get(), newSize );
}

void EvasPlugin::OnPostRender()
{
  // Bind offscreen surface to the evas object
  NativeRenderSurface* surface = GetNativeRenderSurface();

  DALI_ASSERT_DEBUG( surface && "Surface is null in EvasPlugin" );

  tbm_surface_h tbmSurface = AnyCast<tbm_surface_h>( surface->GetDrawable() );

  if( !tbmSurface )
  {
    return;
  }

  mEvasWrapper->BindTBMSurface( tbmSurface );

  mEvasWrapper->RequestRender();

  surface->ReleaseLock();
}

EvasWrapper* EvasPlugin::GetEvasWrapper() const
{
  return mEvasWrapper.get();
}

void EvasPlugin::OnEvasObjectTouchEvent( Dali::Integration::Point& touchPoint, unsigned long timeStamp )
{
  FeedTouchPoint( touchPoint, timeStamp );
}

void EvasPlugin::OnEvasObjectWheelEvent( Dali::Integration::WheelEvent& wheelEvent )
{
  FeedWheelEvent( wheelEvent );
}

void EvasPlugin::OnEvasObjectKeyEvent( Dali::Integration::KeyEvent& keyEvent )
{
  FeedKeyEvent( keyEvent );
}

void EvasPlugin::OnEvasObjectMove( const Rect<int>& geometry )
{
}

void EvasPlugin::OnEvasObjectResize( const Rect<int>& geometry )
{
  ResizeSurface( geometry.width, geometry.height );
}

void EvasPlugin::OnEvasObjectFocusIn()
{
  if( !mIsFocus )
  {
    mFocusedSignal.Emit();

    mIsFocus = true;
  }
}

void EvasPlugin::OnEvasObjectFocusOut()
{
  if( mIsFocus )
  {
    mIsFocus = false;

    Toolkit::KeyInputFocusManager focusManager = Toolkit::KeyInputFocusManager::Get();
    Toolkit::Control currentFocused = focusManager.GetCurrentFocusControl();
    if( currentFocused )
    {
      focusManager.RemoveFocus( currentFocused );
    }

    Clipboard::Get().HideClipboard();

    mUnFocusedSignal.Emit();
  }
}

void EvasPlugin::OnEvasPostRender()
{
}

bool EvasPlugin::OnElmAccessibilityActionEvent( AccessActionInfo& accessActionInfo )
{
  bool ret = false;

  if( mAdaptor == nullptr )
  {
    return ret;
  }

  Dali::AccessibilityAdaptor accessibilityAdaptor = Dali::AccessibilityAdaptor::Get();
  if( accessibilityAdaptor )
  {
    switch( accessActionInfo.actionBy )
    {
      case Dali::Extension::Internal::ACCESS_ACTION_HIGHLIGHT:
      case Dali::Extension::Internal::ACCESS_ACTION_READ:
      {
        ret = accessibilityAdaptor.HandleActionReadEvent( (unsigned int)accessActionInfo.x, (unsigned int)accessActionInfo.y, true );
      }
      break;

      case Dali::Extension::Internal::ACCESS_ACTION_HIGHLIGHT_PREV:
      {
        // if accessActionInfo.highlight_end is true, need to handle end_of_list sound feedback
        ret = accessibilityAdaptor.HandleActionPreviousEvent( accessActionInfo.highlightCycle );
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
        ret = accessibilityAdaptor.HandleActionNextEvent( accessActionInfo.highlightCycle );
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
        Evas_Object* eo = mEvasWrapper->GetAccessibilityTarget();

        if( eo )
        {
          int touchType = accessActionInfo.mouseType;

          TouchPoint::State state( TouchPoint::Down );

          if( touchType == 0 )
          {
            state = TouchPoint::Down; // mouse down
          }
          else if( touchType == 1 )
          {
            state = TouchPoint::Motion; // mouse move
          }
          else if( touchType == 2 )
          {
            state = TouchPoint::Up; // mouse up
          }
          else
          {
            state = TouchPoint::Interrupted; // error
          }

          // Send touch event to accessibility manager.
          Evas_Coord rel_x, rel_y, obj_x,  obj_y, obj_w, obj_h;

          evas_object_geometry_get( eo, &obj_x,  &obj_y, &obj_w, &obj_h );

          rel_x = accessActionInfo.x - obj_x;
          rel_y = accessActionInfo.y - obj_y;

          TouchPoint point( 0, state, (float)rel_x, (float)rel_y );

          ret = accessibilityAdaptor.HandleActionScrollEvent( point, accessActionInfo.timeStamp );
        }
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
        DALI_LOG_WARNING( "[%s:%d]\n", __FUNCTION__, __LINE__ );
      }

      break;
    }
  }
  else
  {
    DALI_LOG_WARNING( "[%s:%d]\n", __FUNCTION__, __LINE__ );
  }

  return ret;
}

void EvasPlugin::OnEcoreWl2VisibilityChange( bool visibility )
{
  DALI_LOG_RELEASE_INFO( "EvasPlugin::OnEcoreWl2VisibilityChange( %s )", visibility ? "T" : "F" );

  if( visibility )
  {
    Show();
  }
  else
  {
    Hide();

    mAdaptor->ReleaseSurfaceLock();
  }
}

void EvasPlugin::Show()
{
  if( !mVisible )
  {
    mVisible = true;

    mAdaptor->OnWindowShown();
  }
}

void EvasPlugin::Hide()
{
  if( mVisible )
  {
    mVisible = false;

    mAdaptor->OnWindowHidden();
  }
}

} // namespace Internal

} // namespace Extension

} // namespace Dali
