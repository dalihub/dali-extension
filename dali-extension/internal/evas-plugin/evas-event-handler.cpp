/*
 * Copyright ( c ) 2020 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 ( the "License" );
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
#include <dali/integration-api/debug.h>
#include <dali/devel-api/adaptor-framework/key-devel.h>
#include <dali/public-api/events/device.h>

// INTERNAL INCLUDES
#include <dali-extension/internal/evas-plugin/evas-event-interface.h>
#include <dali-extension/internal/evas-plugin/evas-wrapper.h>

// CLASS HEADER
#include <dali-extension/internal/evas-plugin/evas-event-handler.h>

namespace Dali
{

namespace Extension
{

namespace Internal
{

namespace
{

const int TOUCH_DEVICE_ID = 0;

const char* EVAS_OBJECT_FOCUSED_EVENT_NAME = "focused";
const char* EVAS_OBJECT_UNFOCUSED_EVENT_NAME = "unfocused";

/**
 * Copied from x server
 * @brief Retrieve the current milliseconds.
 *
 * @return the current milliseconds.
 */
unsigned int GetCurrentMilliSeconds( void )
{
  struct timeval tv;

  struct timespec tp;
  static clockid_t clockid;

  if( !clockid )
  {
#ifdef CLOCK_MONOTONIC_COARSE
    if( clock_getres( CLOCK_MONOTONIC_COARSE, &tp ) == 0 && ( tp.tv_nsec / 1000 ) <= 1000 && clock_gettime( CLOCK_MONOTONIC_COARSE, &tp ) == 0 )
    {
      clockid = CLOCK_MONOTONIC_COARSE;
    }
    else
#endif
    if( clock_gettime( CLOCK_MONOTONIC, &tp ) == 0 )
    {
      clockid = CLOCK_MONOTONIC;
    }
    else
    {
      clockid = ~0L;
    }
  }
  if( clockid != ~0L && clock_gettime( clockid, &tp ) == 0 )
  {
    return ( tp.tv_sec * 1000 ) + ( tp.tv_nsec / 1000000L );
  }

  gettimeofday( &tv, NULL );
  return ( tv.tv_sec * 1000 ) + ( tv.tv_usec / 1000 );
}

/**
 * @brief Retrieve the dali screen touch point.
 *
 * @param[in] canvasX is the x of canvas position
 * @param[in] canvasY is the y of canvas position
 * @param[in] evasObject is rendered by dali
 * @param[out] screenX is the x of local position of evasObject
 * @param[out] screenY is the y of local position of evasObject
 */
void GetScreenPosition( Evas_Coord canvasX, Evas_Coord canvasY, Evas_Object* evasObject, Evas_Coord &screenX, Evas_Coord &screenY )
{
  Evas_Coord objX = 0, objY = 0, objW = 0, objH = 0;
  evas_object_geometry_get( evasObject, &objX, &objY, &objW, &objH );

  screenX = canvasX - objX;
  screenY = canvasY - objY;
}

void ConvertActionInfo( Elm_Access_Action_Info* elmActionInfo, Dali::Extension::Internal::AccessActionInfo&  daliActionInfo )
{
  daliActionInfo.x = elmActionInfo->x;
  daliActionInfo.y = elmActionInfo->y;
  daliActionInfo.mouseType = elmActionInfo->mouse_type;

  switch( elmActionInfo->action_type )
  {
    case ELM_ACCESS_ACTION_HIGHLIGHT:
    {
      daliActionInfo.actionType = ACCESS_ACTION_HIGHLIGHT;
    }
    break;

    case ELM_ACCESS_ACTION_READ:
    {
      daliActionInfo.actionType = ACCESS_ACTION_READ;
    }
    break;

    case ELM_ACCESS_ACTION_HIGHLIGHT_PREV:
    {
      daliActionInfo.actionType = ACCESS_ACTION_HIGHLIGHT_PREV;
    }
    break;

    case ELM_ACCESS_ACTION_HIGHLIGHT_NEXT:
    {
      daliActionInfo.actionType = ACCESS_ACTION_HIGHLIGHT_NEXT;
    }
    break;

    case ELM_ACCESS_ACTION_ACTIVATE:
    {
      daliActionInfo.actionType = ACCESS_ACTION_ACTIVATE;
    }
    break;

    case ELM_ACCESS_ACTION_UNHIGHLIGHT:
    {
      daliActionInfo.actionType = ACCESS_ACTION_UNHIGHLIGHT;
    }
    break;

    case ELM_ACCESS_ACTION_SCROLL:
    {
      daliActionInfo.actionType = ACCESS_ACTION_SCROLL;
    }
    break;

    case ELM_ACCESS_ACTION_UP:
    {
      daliActionInfo.actionType = ACCESS_ACTION_UP;
    }
    break;

    case ELM_ACCESS_ACTION_DOWN:
    {
      daliActionInfo.actionType = ACCESS_ACTION_DOWN;
    }
    break;

    case ELM_ACCESS_ACTION_BACK:
    {
      daliActionInfo.actionType = ACCESS_ACTION_BACK;
    }
    break;

    default:

    break;
  }

  switch( elmActionInfo->action_by )
  {
    case ELM_ACCESS_ACTION_HIGHLIGHT:
    {
      daliActionInfo.actionBy = ACCESS_ACTION_HIGHLIGHT;
    }
    break;

    case ELM_ACCESS_ACTION_READ:
    {
      daliActionInfo.actionBy = ACCESS_ACTION_READ;
    }
    break;

    case ELM_ACCESS_ACTION_HIGHLIGHT_PREV:
    {
      daliActionInfo.actionBy = ACCESS_ACTION_HIGHLIGHT_PREV;
    }
    break;

    case ELM_ACCESS_ACTION_HIGHLIGHT_NEXT:
    {
      daliActionInfo.actionBy = ACCESS_ACTION_HIGHLIGHT_NEXT;
    }
    break;

    case ELM_ACCESS_ACTION_ACTIVATE:
    {
      daliActionInfo.actionBy = ACCESS_ACTION_ACTIVATE;
    }
    break;

    case ELM_ACCESS_ACTION_UNHIGHLIGHT:
    {
      daliActionInfo.actionBy = ACCESS_ACTION_UNHIGHLIGHT;
    }
    break;

    case ELM_ACCESS_ACTION_SCROLL:
    {
      daliActionInfo.actionBy = ACCESS_ACTION_SCROLL;
    }
    break;

    case ELM_ACCESS_ACTION_UP:
    {
      daliActionInfo.actionBy = ACCESS_ACTION_UP;
    }
    break;

    case ELM_ACCESS_ACTION_DOWN:
    {
      daliActionInfo.actionBy = ACCESS_ACTION_DOWN;
    }
    break;

    case ELM_ACCESS_ACTION_BACK:
    {
      daliActionInfo.actionBy = ACCESS_ACTION_BACK;
    }
    break;

    default:

    break;
  }

  daliActionInfo.highlightCycle = elmActionInfo->highlight_cycle;
  daliActionInfo.timeStamp = GetCurrentMilliSeconds();
}

template<typename Type>
void FillIntegrationKeyEvent( Type keyEvent, Integration::KeyEvent& result )
{
  // KeyName
  if( keyEvent->keyname )
  {
    result.keyName = keyEvent->keyname;
  }

  // LogicalKey
  if( keyEvent->key )
  {
    result.logicalKey = keyEvent->key;
  }

  // KeyString
  if( keyEvent->string )
  {
    result.keyString = keyEvent->string;
  }

  // KeyCode
  if( keyEvent->keyname && !strncmp( keyEvent->keyname, "Keycode-", 8 ) )
  {
    result.keyCode = atoi( keyEvent->keyname + 8 );
  }
  else
  {
    int keycode = Dali::DevelKey::GetDaliKeyCode( keyEvent->keyname );
    result.keyCode = ( keycode == Dali::DALI_KEY_INVALID ) ? 0 : keycode;
  }

  // Modifier
  result.keyModifier = 0;
  if( evas_key_modifier_is_set( keyEvent->modifiers, "Shift" ) )
  {
    result.keyModifier |= ECORE_EVENT_MODIFIER_SHIFT;
  }
  if( evas_key_modifier_is_set( keyEvent->modifiers, "Alt" ) )
  {
    result.keyModifier |= ECORE_EVENT_MODIFIER_ALT;
  }
  if( evas_key_modifier_is_set( keyEvent->modifiers, "AltGr" ) )
  {
    result.keyModifier |= ECORE_EVENT_MODIFIER_ALTGR;
  }
  if( evas_key_modifier_is_set( keyEvent->modifiers, "Control" ) )
  {
    result.keyModifier |= ECORE_EVENT_MODIFIER_CTRL;
  }
  if( evas_key_modifier_is_set( keyEvent->modifiers, "Win" ) ||
      evas_key_modifier_is_set( keyEvent->modifiers, "Super" ) ||
      evas_key_modifier_is_set( keyEvent->modifiers, "Hyper" ) )
  {
    result.keyModifier |= ECORE_EVENT_MODIFIER_WIN;
  }

  // Time
  result.time = keyEvent->timestamp;

  // Compose
  if( keyEvent->compose )
  {
    result.compose = keyEvent->compose;
  }

  // DeviceName
  const char* ecoreDeviceName = ecore_device_name_get( keyEvent->dev );
  if( ecoreDeviceName )
  {
    result.deviceName = ecoreDeviceName;
  }

  // DeviceClass
  Ecore_Device_Class ecoreDeviceClass = ecore_device_class_get( keyEvent->dev );
  switch( ecoreDeviceClass )
  {
    case ECORE_DEVICE_CLASS_SEAT:
    {
      result.deviceClass = Device::Class::USER;
      break;
    }
    case ECORE_DEVICE_CLASS_KEYBOARD:
    {
      result.deviceClass = Device::Class::KEYBOARD;
      break;
    }
    case ECORE_DEVICE_CLASS_MOUSE:
    {
      result.deviceClass = Device::Class::MOUSE;
      break;
    }
    case ECORE_DEVICE_CLASS_TOUCH:
    {
      result.deviceClass = Device::Class::TOUCH;
      break;
    }
    case ECORE_DEVICE_CLASS_PEN:
    {
      result.deviceClass = Device::Class::PEN;
      break;
    }
    case ECORE_DEVICE_CLASS_POINTER:
    {
      result.deviceClass = Device::Class::POINTER;
      break;
    }
    case ECORE_DEVICE_CLASS_GAMEPAD:
    {
      result.deviceClass = Device::Class::GAMEPAD;
      break;
    }
    default:
    {
      result.deviceClass = Device::Class::NONE;
      break;
    }
  }

  // DeviceSubClass
  Ecore_Device_Subclass ecoreDeviceSubclass = ecore_device_subclass_get( keyEvent->dev );

  switch( ecoreDeviceSubclass )
  {
    case ECORE_DEVICE_SUBCLASS_FINGER:
    {
      result.deviceSubclass = Device::Subclass::FINGER;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_FINGERNAIL:
    {
      result.deviceSubclass = Device::Subclass::FINGERNAIL;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_KNUCKLE:
    {
      result.deviceSubclass = Device::Subclass::KNUCKLE;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_PALM:
    {
      result.deviceSubclass = Device::Subclass::PALM;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_HAND_SIZE:
    {
      result.deviceSubclass = Device::Subclass::HAND_SIDE;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_HAND_FLAT:
    {
      result.deviceSubclass = Device::Subclass::HAND_FLAT;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_PEN_TIP:
    {
      result.deviceSubclass = Device::Subclass::PEN_TIP;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_TRACKPAD:
    {
      result.deviceSubclass = Device::Subclass::TRACKPAD;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_TRACKPOINT:
    {
      result.deviceSubclass = Device::Subclass::TRACKPOINT;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_TRACKBALL:
    {
      result.deviceSubclass = Device::Subclass::TRACKBALL;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_REMOCON:
    {
      result.deviceSubclass = Device::Subclass::REMOCON;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_VIRTUAL_KEYBOARD:
    {
      result.deviceSubclass = Device::Subclass::VIRTUAL_KEYBOARD;
      break;
    }
    default:
    {
      result.deviceSubclass = Device::Subclass::NONE;
      break;
    }
  }
}

} // anonymous namespace

EvasEventHandler::EvasEventHandler( EvasEventInterface& evasEventInterface )
: mEvasEventInterface( evasEventInterface )
{
  EvasWrapper* evasWrapper = mEvasEventInterface.GetEvasWrapper();
  Evas_Object* renderTarget = evasWrapper->GetRenderTarget();
  Evas_Object* accessibilityTarget = evasWrapper->GetAccessibilityTarget();
  Evas_Object* focusTarget = evasWrapper->GetFocusTarget();
  Evas* renderTargetAsEvas = evas_object_evas_get( renderTarget );

  // Register the evas event callbacks
  evas_object_event_callback_add( renderTarget, EVAS_CALLBACK_MOUSE_DOWN,  OnEvasObjectMouseDown,      &evasEventInterface );
  evas_object_event_callback_add( renderTarget, EVAS_CALLBACK_MOUSE_UP,    OnEvasObjectMouseUp,        &evasEventInterface );
  evas_object_event_callback_add( renderTarget, EVAS_CALLBACK_MOUSE_MOVE,  OnEvasObjectMouseMove,      &evasEventInterface );
  evas_object_event_callback_add( renderTarget, EVAS_CALLBACK_MOUSE_WHEEL, OnEvasObjectMouseWheel,     &evasEventInterface );
  evas_object_event_callback_add( renderTarget, EVAS_CALLBACK_MULTI_DOWN,  OnEvasObjectMultiTouchDown, &evasEventInterface );
  evas_object_event_callback_add( renderTarget, EVAS_CALLBACK_MULTI_UP,    OnEvasObjectMultiTouchUp,   &evasEventInterface );
  evas_object_event_callback_add( renderTarget, EVAS_CALLBACK_MULTI_MOVE,  OnEvasObjectMultiTouchMove, &evasEventInterface );
  evas_object_event_callback_add( renderTarget, EVAS_CALLBACK_KEY_DOWN,    OnEvasObjectKeyDown,        &evasEventInterface );
  evas_object_event_callback_add( renderTarget, EVAS_CALLBACK_KEY_UP,      OnEvasObjectKeyUp,          &evasEventInterface );

  // Register the evas geometry callbacks
  evas_object_event_callback_add( renderTarget, EVAS_CALLBACK_MOVE,   OnEvasObjectMove,   &evasEventInterface );
  evas_object_event_callback_add( renderTarget, EVAS_CALLBACK_RESIZE, OnEvasObjectResize, &evasEventInterface );

  // Register the evas visibility callbacks
  evas_object_event_callback_add( renderTarget, EVAS_CALLBACK_SHOW, OnEvasObjectShow, &evasEventInterface );
  evas_object_event_callback_add( renderTarget, EVAS_CALLBACK_HIDE, OnEvasObjectHide, &evasEventInterface );

  // Register the evas focus callbacks
  evas_object_event_callback_add( renderTarget, EVAS_CALLBACK_FOCUS_IN,  OnEvasObjectFocusIn,  this );
  evas_object_event_callback_add( renderTarget, EVAS_CALLBACK_FOCUS_OUT, OnEvasObjectFocusOut, this );

  evas_event_callback_add( renderTargetAsEvas, EVAS_CALLBACK_CANVAS_FOCUS_IN,  OnEvasFocusIn,  &evasEventInterface );
  evas_event_callback_add( renderTargetAsEvas, EVAS_CALLBACK_CANVAS_FOCUS_OUT, OnEvasFocusOut, &evasEventInterface );

  // Register the elm access action callbacks and these callbacks are disconnected when mElmAccessEvasObject is unregistred
  elm_access_action_cb_set( accessibilityTarget, ELM_ACCESS_ACTION_HIGHLIGHT,      OnElmAccessActionHighlight,      &evasEventInterface );
  elm_access_action_cb_set( accessibilityTarget, ELM_ACCESS_ACTION_UNHIGHLIGHT,    OnElmAccessActionUnhighlight,    &evasEventInterface );
  elm_access_action_cb_set( accessibilityTarget, ELM_ACCESS_ACTION_HIGHLIGHT_NEXT, OnElmAccessActionHighlightNext,  &evasEventInterface );
  elm_access_action_cb_set( accessibilityTarget, ELM_ACCESS_ACTION_HIGHLIGHT_PREV, OnElmAccessActionHighlightPrev,  &evasEventInterface );
  elm_access_action_cb_set( accessibilityTarget, ELM_ACCESS_ACTION_ACTIVATE,       OnElmAccessActionActivate,       &evasEventInterface );
  elm_access_action_cb_set( accessibilityTarget, ELM_ACCESS_ACTION_UP,             OnElmAccessActionUp,             &evasEventInterface );
  elm_access_action_cb_set( accessibilityTarget, ELM_ACCESS_ACTION_DOWN,           OnElmAccessActionDown,           &evasEventInterface );
  elm_access_action_cb_set( accessibilityTarget, ELM_ACCESS_ACTION_SCROLL,         OnElmAccessActionScroll,         &evasEventInterface );
  elm_access_action_cb_set( accessibilityTarget, ELM_ACCESS_ACTION_BACK,           OnElmAccessActionBack,           &evasEventInterface );
  elm_access_action_cb_set( accessibilityTarget, ELM_ACCESS_ACTION_READ,           OnElmAccessActionRead,           &evasEventInterface );

  // Register the elm focus callbacks
  evas_object_smart_callback_add( focusTarget, EVAS_OBJECT_FOCUSED_EVENT_NAME,   OnEvasObjectSmartFocused,   this );
  evas_object_smart_callback_add( focusTarget, EVAS_OBJECT_UNFOCUSED_EVENT_NAME, OnEvasObjectSmartUnfocused, this );

  EnableEcoreWl2Events();
}

EvasEventHandler::~EvasEventHandler()
{
  EvasWrapper* evasWrapper = mEvasEventInterface.GetEvasWrapper();
  Evas_Object* renderTarget = evasWrapper->GetRenderTarget();
  Evas_Object* focusTarget = evasWrapper->GetFocusTarget();
  Evas* renderTargetAsEvas = evas_object_evas_get( renderTarget );

  // Unregister the evas event callbacks.
  evas_object_event_callback_del( renderTarget, EVAS_CALLBACK_MOUSE_DOWN,  OnEvasObjectMouseDown );
  evas_object_event_callback_del( renderTarget, EVAS_CALLBACK_MOUSE_UP,    OnEvasObjectMouseUp );
  evas_object_event_callback_del( renderTarget, EVAS_CALLBACK_MOUSE_MOVE,  OnEvasObjectMouseMove );
  evas_object_event_callback_del( renderTarget, EVAS_CALLBACK_MOUSE_WHEEL, OnEvasObjectMouseWheel );
  evas_object_event_callback_del( renderTarget, EVAS_CALLBACK_MULTI_DOWN,  OnEvasObjectMultiTouchDown );
  evas_object_event_callback_del( renderTarget, EVAS_CALLBACK_MULTI_UP,    OnEvasObjectMultiTouchUp );
  evas_object_event_callback_del( renderTarget, EVAS_CALLBACK_MULTI_MOVE,  OnEvasObjectMultiTouchMove );
  evas_object_event_callback_del( renderTarget, EVAS_CALLBACK_KEY_DOWN,    OnEvasObjectKeyDown );
  evas_object_event_callback_del( renderTarget, EVAS_CALLBACK_KEY_UP,      OnEvasObjectKeyUp );

  // Unregister the evas geometry callbacks.
  evas_object_event_callback_del( renderTarget, EVAS_CALLBACK_MOVE,   OnEvasObjectMove );
  evas_object_event_callback_del( renderTarget, EVAS_CALLBACK_RESIZE, OnEvasObjectResize );

  // Unregister the evas visibility callbacks
  evas_object_event_callback_del( renderTarget, EVAS_CALLBACK_SHOW, OnEvasObjectShow );
  evas_object_event_callback_del( renderTarget, EVAS_CALLBACK_HIDE, OnEvasObjectHide );

  // Unregister the evas focus callbacks
  evas_object_event_callback_del( renderTarget, EVAS_CALLBACK_FOCUS_IN,  OnEvasObjectFocusIn );
  evas_object_event_callback_del( renderTarget, EVAS_CALLBACK_FOCUS_OUT, OnEvasObjectFocusOut );

  evas_event_callback_del( renderTargetAsEvas, EVAS_CALLBACK_CANVAS_FOCUS_IN,  OnEvasFocusIn );
  evas_event_callback_del( renderTargetAsEvas, EVAS_CALLBACK_CANVAS_FOCUS_OUT, OnEvasFocusOut );

  // Unregister the elm focus callbacks
  evas_object_smart_callback_del( focusTarget, EVAS_OBJECT_FOCUSED_EVENT_NAME,   OnEvasObjectSmartFocused );
  evas_object_smart_callback_del( focusTarget, EVAS_OBJECT_UNFOCUSED_EVENT_NAME, OnEvasObjectSmartUnfocused );

  DisableEcoreWl2Events();
}

void EvasEventHandler::EnableEcoreWl2Events()
{
  if( !mEcoreEventHandlers.size() )
  {
    // Register Window visibility change events
    mEcoreEventHandlers.push_back( ecore_event_handler_add( ECORE_WL2_EVENT_WINDOW_VISIBILITY_CHANGE, OnEcoreWl2EventWindowVisibilityChange, &mEvasEventInterface ) );
  }
}

void EvasEventHandler::DisableEcoreWl2Events()
{
  if( mEcoreEventHandlers.size() )
  {
    // Unregister Window events
    for( std::vector<Ecore_Event_Handler*>::iterator iter = mEcoreEventHandlers.begin(), endIter = mEcoreEventHandlers.end(); iter != endIter; ++iter )
    {
      ecore_event_handler_del( *iter );
    }
    mEcoreEventHandlers.clear();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Event callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////

void EvasEventHandler::OnEvasObjectMouseDown( void *data, Evas* evas, Evas_Object* evasObject, void* event )
{
  Evas_Event_Mouse_Down* eventMouseDown = static_cast<Evas_Event_Mouse_Down*>( event );

  Evas_Coord screenX = 0.0f, screenY = 0.0f;
  GetScreenPosition( eventMouseDown->canvas.x, eventMouseDown->canvas.y, evasObject, screenX, screenY );

  Dali::Integration::Point point( TouchPoint( TOUCH_DEVICE_ID, TouchPoint::Down, screenX, screenY ) );
  unsigned long timeStamp = eventMouseDown->timestamp;
  if( timeStamp < 1 )
  {
    timeStamp = GetCurrentMilliSeconds();
  }

  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  eventInterface->OnEvasObjectTouchEvent( point, timeStamp );
}

void EvasEventHandler::OnEvasObjectMouseUp( void *data, Evas* evas, Evas_Object* evasObject, void* event )
{
  Evas_Event_Mouse_Up* eventMouseUp = static_cast<Evas_Event_Mouse_Up*>( event );

  Evas_Coord screenX = 0.0f, screenY = 0.0f;
  GetScreenPosition( eventMouseUp->canvas.x, eventMouseUp->canvas.y, evasObject, screenX, screenY );

  Dali::Integration::Point point( TouchPoint( TOUCH_DEVICE_ID, TouchPoint::Up, screenX, screenY ) );
  unsigned long timeStamp = eventMouseUp->timestamp;
  if( timeStamp < 1 )
  {
    timeStamp = GetCurrentMilliSeconds();
  }

  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  eventInterface->OnEvasObjectTouchEvent( point, timeStamp );
}

void EvasEventHandler::OnEvasObjectMouseMove( void *data, Evas* evas, Evas_Object* evasObject, void* event )
{
  Evas_Event_Mouse_Move* eventMouseMove = static_cast<Evas_Event_Mouse_Move*>( event );

  Evas_Coord screenX = 0.0f, screenY = 0.0f;
  GetScreenPosition( eventMouseMove->cur.canvas.x, eventMouseMove->cur.canvas.y, evasObject, screenX, screenY );

  Dali::Integration::Point point( TouchPoint( TOUCH_DEVICE_ID, TouchPoint::Motion, screenX, screenY ) );
  unsigned long timeStamp = eventMouseMove->timestamp;
  if( timeStamp < 1 )
  {
    timeStamp = GetCurrentMilliSeconds();
  }

  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  eventInterface->OnEvasObjectTouchEvent( point, timeStamp );
}

void EvasEventHandler::OnEvasObjectMouseWheel( void *data, Evas* evas, Evas_Object* evasObject, void* event )
{
  Evas_Event_Mouse_Wheel* eventMouseWheel = static_cast<Evas_Event_Mouse_Wheel*>( event );

  Evas_Coord screenX = 0.0f, screenY = 0.0f;
  GetScreenPosition( eventMouseWheel->canvas.x, eventMouseWheel->canvas.y, evasObject, screenX, screenY );

  int direction = eventMouseWheel->direction;
  unsigned int modifiers = -1;  // TODO: Need to check evas modifier
  Vector2 point = Vector2( screenX, screenY );
  int z = eventMouseWheel->z;
  unsigned int timeStamp = eventMouseWheel->timestamp;
  if( timeStamp < 1 )
  {
    timeStamp = GetCurrentMilliSeconds();
  }

  Dali::Integration::WheelEvent wheelEvent( Dali::Integration::WheelEvent::MOUSE_WHEEL, direction, modifiers, point, z, timeStamp );

  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  eventInterface->OnEvasObjectWheelEvent( wheelEvent );
}

void EvasEventHandler::OnEvasObjectMultiTouchDown( void *data, Evas* evas, Evas_Object* evasObject, void* event )
{
  Evas_Event_Multi_Down* eventMultiDown = static_cast<Evas_Event_Multi_Down*>( event );

  Evas_Coord screenX = 0.0f, screenY = 0.0f;
  GetScreenPosition( eventMultiDown->canvas.x, eventMultiDown->canvas.y, evasObject, screenX, screenY );

  Dali::Integration::Point point( TouchPoint( eventMultiDown->device, TouchPoint::Down, screenX, screenY ) );
  unsigned long timeStamp = eventMultiDown->timestamp;
  if( timeStamp < 1 )
  {
    timeStamp = GetCurrentMilliSeconds();
  }

  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  eventInterface->OnEvasObjectTouchEvent( point, timeStamp );
}

void EvasEventHandler::OnEvasObjectMultiTouchUp( void *data, Evas* evas, Evas_Object* evasObject, void* event )
{
  Evas_Event_Multi_Up* eventMultiUp = static_cast<Evas_Event_Multi_Up*>( event );

  Evas_Coord screenX = 0.0f, screenY = 0.0f;
  GetScreenPosition( eventMultiUp->canvas.x, eventMultiUp->canvas.y, evasObject, screenX, screenY );

  Dali::Integration::Point point( TouchPoint( eventMultiUp->device, TouchPoint::Up, screenX, screenY ) );
  unsigned long timeStamp = eventMultiUp->timestamp;
  if( timeStamp < 1 )
  {
    timeStamp = GetCurrentMilliSeconds();
  }

  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  eventInterface->OnEvasObjectTouchEvent( point, timeStamp );
}

void EvasEventHandler::OnEvasObjectMultiTouchMove( void *data, Evas* evas, Evas_Object* evasObject, void* event )
{
  Evas_Event_Multi_Move* eventMultiMove = static_cast<Evas_Event_Multi_Move*>( event );

  Evas_Coord screenX = 0.0f, screenY = 0.0f;
  GetScreenPosition( eventMultiMove->cur.canvas.x, eventMultiMove->cur.canvas.y, evasObject, screenX, screenY );

  Dali::Integration::Point point( TouchPoint( eventMultiMove->device, TouchPoint::Motion, screenX, screenY ) );
  unsigned long timeStamp = eventMultiMove->timestamp;
  if( timeStamp < 1 )
  {
    timeStamp = GetCurrentMilliSeconds();
  }

  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  eventInterface->OnEvasObjectTouchEvent( point, timeStamp );
}

void EvasEventHandler::OnEvasObjectKeyDown( void *data, Evas* evas, Evas_Object* evasObject, void* event )
{
  Evas_Event_Key_Down* keyEvent = static_cast<Evas_Event_Key_Down*>( event );

  // Create KeyEvent
  Integration::KeyEvent integKeyEvent;

  integKeyEvent.state = Integration::KeyEvent::Down;

  FillIntegrationKeyEvent( keyEvent, integKeyEvent );

  // Feed to EvasPlugin
  ( static_cast<EvasEventInterface*>( data ) )->OnEvasObjectKeyEvent( integKeyEvent );
}

void EvasEventHandler::OnEvasObjectKeyUp( void *data, Evas* evas, Evas_Object* evasObject, void* event )
{
  Evas_Event_Key_Up* keyEvent = static_cast<Evas_Event_Key_Up*>( event );

  // Create KeyEvent
  Integration::KeyEvent integKeyEvent;

  integKeyEvent.state = Integration::KeyEvent::Up;

  FillIntegrationKeyEvent( keyEvent, integKeyEvent );

  // Feed to EvasPlugin
  ( static_cast<EvasEventInterface*>( data ) )->OnEvasObjectKeyEvent( integKeyEvent );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Geometry callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////

void EvasEventHandler::OnEvasObjectMove( void *data, Evas* evas, Evas_Object* evasObject, void* event )
{
  Rect<int> geometry;
  evas_object_geometry_get( evasObject, &geometry.x, &geometry.y, &geometry.width, &geometry.height );

  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  eventInterface->OnEvasObjectMove( geometry );
}

void EvasEventHandler::OnEvasObjectResize( void *data, Evas* evas, Evas_Object* evasObject, void* event )
{
  Rect<int> geometry;
  evas_object_geometry_get( evasObject, &geometry.x, &geometry.y, &geometry.width, &geometry.height );

  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  eventInterface->OnEvasObjectResize( geometry );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Visibility callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////

void EvasEventHandler::OnEvasObjectShow( void *data, Evas* evas, Evas_Object* evasObject, void* event )
{
  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  eventInterface->OnEvasObjectVisiblityChanged( true );
}

void EvasEventHandler::OnEvasObjectHide( void *data, Evas* evas, Evas_Object* evasObject, void* event )
{
  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  eventInterface->OnEvasObjectVisiblityChanged( false );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Focus callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////

void EvasEventHandler::OnEvasObjectFocusIn( void *data, Evas* evas, Evas_Object* evasObject, void* event )
{
  EvasEventHandler* eventHandler = static_cast<EvasEventHandler*>( data );

  eventHandler->GetEvasEventInterface().OnEvasObjectFocusIn();
}

void EvasEventHandler::OnEvasObjectFocusOut( void *data, Evas* evas, Evas_Object* evasObject, void* event )
{
  EvasEventHandler* eventHandler = static_cast<EvasEventHandler*>( data );

  eventHandler->GetEvasEventInterface().OnEvasObjectFocusOut();
}

void EvasEventHandler::OnEvasFocusIn( void *data, Evas* evas, void* event )
{
}

void EvasEventHandler::OnEvasFocusOut( void *data, Evas* evas, void* event )
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Elm Access callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////

Eina_Bool EvasEventHandler::OnElmAccessActionHighlight( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo )
{
  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo( actionInfo, daliActionInfo );

  return eventInterface->OnElmAccessibilityActionEvent( daliActionInfo );
}

Eina_Bool EvasEventHandler::OnElmAccessActionUnhighlight( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo )
{
  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo( actionInfo, daliActionInfo );

  return eventInterface->OnElmAccessibilityActionEvent( daliActionInfo );
}

Eina_Bool EvasEventHandler::OnElmAccessActionHighlightNext( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo )
{
  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo( actionInfo, daliActionInfo );

  return eventInterface->OnElmAccessibilityActionEvent( daliActionInfo );
}

Eina_Bool EvasEventHandler::OnElmAccessActionHighlightPrev( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo )
{
  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo( actionInfo, daliActionInfo );

  return eventInterface->OnElmAccessibilityActionEvent( daliActionInfo );
}

Eina_Bool EvasEventHandler::OnElmAccessActionActivate( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo )
{
  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo( actionInfo, daliActionInfo );

  return eventInterface->OnElmAccessibilityActionEvent( daliActionInfo );
}

Eina_Bool EvasEventHandler::OnElmAccessActionScroll( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo )
{
  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo( actionInfo, daliActionInfo );

  return eventInterface->OnElmAccessibilityActionEvent( daliActionInfo );
}

Eina_Bool EvasEventHandler::OnElmAccessActionUp( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo )
{
  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo( actionInfo, daliActionInfo );

  return eventInterface->OnElmAccessibilityActionEvent( daliActionInfo );
}

Eina_Bool EvasEventHandler::OnElmAccessActionDown( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo )
{
  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo( actionInfo, daliActionInfo );

  return eventInterface->OnElmAccessibilityActionEvent( daliActionInfo );
}

Eina_Bool EvasEventHandler::OnElmAccessActionBack( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo )
{
  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo( actionInfo, daliActionInfo );

  return eventInterface->OnElmAccessibilityActionEvent( daliActionInfo );
}

Eina_Bool EvasEventHandler::OnElmAccessActionRead( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo )
{
  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo( actionInfo, daliActionInfo );

  return eventInterface->OnElmAccessibilityActionEvent( daliActionInfo );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Elm Focus callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////

void EvasEventHandler::OnEvasObjectSmartFocused( void *data, Evas_Object* evasObject, void* event )
{
  EvasEventHandler* eventHandler = static_cast<EvasEventHandler*>( data );
  EvasEventInterface& evasPlugin = eventHandler->GetEvasEventInterface();

  if( eventHandler->mEvasEventInterface.GetEvasWrapper()->GetFocusTarget() == evasObject )
  {
    Evas_Object* topWidget = elm_object_top_widget_get( evasObject );

    if( !strcmp( "elm_win", elm_object_widget_type_get( topWidget ) ) )
    {
      if( elm_win_focus_highlight_enabled_get( topWidget ) == EINA_TRUE )
      {
        // To allow that KeyboardFocusManager can handle the keyboard focus
        Dali::Integration::KeyEvent fakeKeyEvent( Dali::KeyEvent( "", "", 0, 0, 100, KeyEvent::Down ) );

        evasPlugin.OnEvasObjectKeyEvent( fakeKeyEvent );
      }
    }

    evas_object_focus_set( eventHandler->mEvasEventInterface.GetEvasWrapper()->GetRenderTarget(), EINA_TRUE );
  }
}

void EvasEventHandler::OnEvasObjectSmartUnfocused( void *data, Evas_Object* evasObject, void* event )
{
  EvasEventHandler* eventHandler = static_cast<EvasEventHandler*>( data );
  if( eventHandler->mEvasEventInterface.GetEvasWrapper()->GetFocusTarget() == evasObject )
  {
    evas_object_focus_set( eventHandler->mEvasEventInterface.GetEvasWrapper()->GetRenderTarget(), EINA_FALSE );
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Ecore Wl2 callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////

Eina_Bool EvasEventHandler::OnEcoreWl2EventWindowVisibilityChange( void* data, int type, void* event )
{
  Ecore_Wl2_Event_Window_Visibility_Change* eventWindowVisibilityChange = static_cast<Ecore_Wl2_Event_Window_Visibility_Change*>( event );

  EvasEventInterface* eventInterface = static_cast<EvasEventInterface*>( data );

  // 0 is visible and 1 is invisible
  eventInterface->OnEcoreWl2VisibilityChange( !eventWindowVisibilityChange->fully_obscured );

  return ECORE_CALLBACK_PASS_ON;
}

}  // namespace Internal

}  // namespace Extension

}  // namespace Dali
