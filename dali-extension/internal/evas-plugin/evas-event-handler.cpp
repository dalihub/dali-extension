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
#include "evas-event-handler.h"

namespace Dali
{

namespace Extension
{

namespace Internal
{

/**
 * Copied from x server
 * @brief Retrieve the current milliseconds.
 *
 * @return the current milliseconds.
 */
static unsigned int GetCurrentMilliSeconds(void)
{
  struct timeval tv;

  struct timespec tp;
  static clockid_t clockid;

  if (!clockid)
  {
#ifdef CLOCK_MONOTONIC_COARSE
    if (clock_getres(CLOCK_MONOTONIC_COARSE, &tp) == 0 &&
      (tp.tv_nsec / 1000) <= 1000 && clock_gettime(CLOCK_MONOTONIC_COARSE, &tp) == 0)
    {
      clockid = CLOCK_MONOTONIC_COARSE;
    }
    else
#endif
    if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
    {
      clockid = CLOCK_MONOTONIC;
    }
    else
    {
      clockid = ~0L;
    }
  }
  if (clockid != ~0L && clock_gettime(clockid, &tp) == 0)
  {
    return (tp.tv_sec * 1000) + (tp.tv_nsec / 1000000L);
  }

  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

namespace
{

const int TOUCH_DEVICE_ID = 0;

const char* EVAS_OBJECT_FOCUSED_EVENT_NAME = "focused";
const char* EVAS_OBJECT_UNFOCUSED_EVENT_NAME = "unfocused";

/**
 * @brief Retrieve the dali screen touch point.
 *
 * @param[in] canvasX is the x of canvas position
 * @param[in] canvasY is the y of canvas position
 * @param[in] evasObject is rendered by dali
 * @param[out] screenX is the x of local position of evasObject
 * @param[out] screenY is the y of local position of evasObject
 */
void GetScreenPosition(Evas_Coord canvasX, Evas_Coord canvasY, Evas_Object* evasObject, Evas_Coord &screenX, Evas_Coord &screenY)
{
  Evas_Coord objX = 0, objY = 0, objW = 0, objH = 0;
  evas_object_geometry_get(evasObject, &objX, &objY, &objW, &objH);

  screenX = canvasX - objX;
  screenY = canvasY - objY;
}

#ifdef DALI_EVASPLUGIN_USE_IMF_MANAGER
/**
 * @brief Evas_Modifier enums in Ecore_Input.h do not match Ecore_Event_Modifier in Ecore_Input.h.
 *        This function converts from Evas_Modifier to Ecore_Event_Modifier enums.
 *
 * @param[in] evasModifier the Evas_Modifier input.
 * @return the Ecore_Event_Modifier output.
 */
unsigned int EvasModifierToEcoreModifier(Evas_Modifier* evasModifier)
{
   unsigned int modifier = 0;  // If no other matches returns NONE.

   if (evas_key_modifier_is_set(evasModifier, "Shift"))
   {
     modifier |= ECORE_EVENT_MODIFIER_SHIFT;
   }

   if (evas_key_modifier_is_set(evasModifier, "Alt"))
   {
     modifier |= ECORE_EVENT_MODIFIER_ALT;
   }

   if (evas_key_modifier_is_set(evasModifier, "AltGr"))
   {
     modifier |= ECORE_EVENT_MODIFIER_ALTGR;
   }

   if (evas_key_modifier_is_set(evasModifier, "Control"))
   {
     modifier |= ECORE_EVENT_MODIFIER_CTRL;
   }

   if (evas_key_modifier_is_set(evasModifier, "Win") ||
       evas_key_modifier_is_set(evasModifier, "Super") ||
       evas_key_modifier_is_set(evasModifier, "Hyper"))
   {
     modifier |= ECORE_EVENT_MODIFIER_WIN;
   }

   return modifier;
}
#endif


void ConvertActionInfo(Elm_Access_Action_Info* elmActionInfo, Dali::Extension::Internal::AccessActionInfo&  daliActionInfo)
{
  daliActionInfo.x = elmActionInfo->x;
  daliActionInfo.y = elmActionInfo->y;
  daliActionInfo.mouseType = elmActionInfo->mouse_type;

  switch(elmActionInfo->action_type)
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

  switch(elmActionInfo->action_by)
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

} // namespace

EvasEventHandler::EvasEventHandler(Evas_Object* imageEvasObject,
                                   Evas_Object* elmAccessEvasObject,
                                   Evas_Object* elmFocusEvasObject,
                                   EvasPluginEventInterface& evasPluginEventInterface)
: mEvasPluginEventInterface(evasPluginEventInterface)
, mEvas(evas_object_evas_get(imageEvasObject))
, mImageEvasObject(imageEvasObject)
, mElmAccessEvasObject(elmAccessEvasObject)
, mElmFocusEvasObject(elmFocusEvasObject)
{
  // Register the evas event callbacks
  evas_object_event_callback_add(mImageEvasObject, EVAS_CALLBACK_MOUSE_DOWN,  OnEvasObjectMouseDown,      &evasPluginEventInterface);
  evas_object_event_callback_add(mImageEvasObject, EVAS_CALLBACK_MOUSE_UP,    OnEvasObjectMouseUp,        &evasPluginEventInterface);
  evas_object_event_callback_add(mImageEvasObject, EVAS_CALLBACK_MOUSE_MOVE,  OnEvasObjectMouseMove,      &evasPluginEventInterface);
  evas_object_event_callback_add(mImageEvasObject, EVAS_CALLBACK_MOUSE_WHEEL, OnEvasObjectMouseWheel,     &evasPluginEventInterface);
  evas_object_event_callback_add(mImageEvasObject, EVAS_CALLBACK_MULTI_DOWN,  OnEvasObjectMultiTouchDown, &evasPluginEventInterface);
  evas_object_event_callback_add(mImageEvasObject, EVAS_CALLBACK_MULTI_UP,    OnEvasObjectMultiTouchUp,   &evasPluginEventInterface);
  evas_object_event_callback_add(mImageEvasObject, EVAS_CALLBACK_MULTI_MOVE,  OnEvasObjectMultiTouchMove, &evasPluginEventInterface);
  evas_object_event_callback_add(mImageEvasObject, EVAS_CALLBACK_KEY_DOWN,    OnEvasObjectKeyDown,        &evasPluginEventInterface);
  evas_object_event_callback_add(mImageEvasObject, EVAS_CALLBACK_KEY_UP,      OnEvasObjectKeyUp,          &evasPluginEventInterface);

  // Register the evas geometry callbacks
  evas_object_event_callback_add(mImageEvasObject, EVAS_CALLBACK_MOVE,   OnEvasObjectMove,   &evasPluginEventInterface);
  evas_object_event_callback_add(mImageEvasObject, EVAS_CALLBACK_RESIZE, OnEvasObjectResize, &evasPluginEventInterface);

  // Register the evas focus callbacks
  evas_object_event_callback_add(mImageEvasObject, EVAS_CALLBACK_FOCUS_IN,  OnEvasObjectFocusIn,  &evasPluginEventInterface);
  evas_object_event_callback_add(mImageEvasObject, EVAS_CALLBACK_FOCUS_OUT, OnEvasObjectFocusOut, &evasPluginEventInterface);

  evas_event_callback_add(mEvas, EVAS_CALLBACK_CANVAS_FOCUS_IN,  OnEvasFocusIn,  &evasPluginEventInterface);
  evas_event_callback_add(mEvas, EVAS_CALLBACK_CANVAS_FOCUS_OUT, OnEvasFocusOut, &evasPluginEventInterface);

  // Register the evas render callbacks
  evas_event_callback_add(mEvas, EVAS_CALLBACK_RENDER_POST, OnEvasRenderPost, &evasPluginEventInterface);

  // Register the elm access action callbacks and these callbacks are disconnected when mElmAccessEvasObject is unregistred
  elm_access_action_cb_set(mElmAccessEvasObject, ELM_ACCESS_ACTION_HIGHLIGHT,      OnElmAccessActionHighlight,      &evasPluginEventInterface);
  elm_access_action_cb_set(mElmAccessEvasObject, ELM_ACCESS_ACTION_UNHIGHLIGHT,    OnElmAccessActionUnhighlight,    &evasPluginEventInterface);
  elm_access_action_cb_set(mElmAccessEvasObject, ELM_ACCESS_ACTION_HIGHLIGHT_NEXT, OnElmAccessActionHighlightNext,  &evasPluginEventInterface);
  elm_access_action_cb_set(mElmAccessEvasObject, ELM_ACCESS_ACTION_HIGHLIGHT_PREV, OnElmAccessActionHighlightPrev, &evasPluginEventInterface);
  elm_access_action_cb_set(mElmAccessEvasObject, ELM_ACCESS_ACTION_ACTIVATE,       OnElmAccessActionActivate,       &evasPluginEventInterface);
  elm_access_action_cb_set(mElmAccessEvasObject, ELM_ACCESS_ACTION_UP,             OnElmAccessActionUp,             &evasPluginEventInterface);
  elm_access_action_cb_set(mElmAccessEvasObject, ELM_ACCESS_ACTION_DOWN,           OnElmAccessActionDown,           &evasPluginEventInterface);
  elm_access_action_cb_set(mElmAccessEvasObject, ELM_ACCESS_ACTION_SCROLL,         OnElmAccessActionScroll,         &evasPluginEventInterface);
  elm_access_action_cb_set(mElmAccessEvasObject, ELM_ACCESS_ACTION_BACK,           OnElmAccessActionBack,           &evasPluginEventInterface);
  elm_access_action_cb_set(mElmAccessEvasObject, ELM_ACCESS_ACTION_READ,           OnElmAccessActionRead,           &evasPluginEventInterface);

  // Register the elm focus callbacks
  evas_object_smart_callback_add(mElmFocusEvasObject, EVAS_OBJECT_FOCUSED_EVENT_NAME,   OnEvasObjectSmartFocused,   this);
  evas_object_smart_callback_add(mElmFocusEvasObject, EVAS_OBJECT_UNFOCUSED_EVENT_NAME, OnEvasObjectSmartUnfocused, this);
}

EvasEventHandler::~EvasEventHandler()
{
  // Unregister the evas event callbacks.
  evas_object_event_callback_del(mImageEvasObject, EVAS_CALLBACK_MOUSE_DOWN,  OnEvasObjectMouseDown);
  evas_object_event_callback_del(mImageEvasObject, EVAS_CALLBACK_MOUSE_UP,    OnEvasObjectMouseUp);
  evas_object_event_callback_del(mImageEvasObject, EVAS_CALLBACK_MOUSE_MOVE,  OnEvasObjectMouseMove);
  evas_object_event_callback_del(mImageEvasObject, EVAS_CALLBACK_MOUSE_WHEEL, OnEvasObjectMouseWheel);
  evas_object_event_callback_del(mImageEvasObject, EVAS_CALLBACK_MULTI_DOWN,  OnEvasObjectMultiTouchDown);
  evas_object_event_callback_del(mImageEvasObject, EVAS_CALLBACK_MULTI_UP,    OnEvasObjectMultiTouchUp);
  evas_object_event_callback_del(mImageEvasObject, EVAS_CALLBACK_MULTI_MOVE,  OnEvasObjectMultiTouchMove);
  evas_object_event_callback_del(mImageEvasObject, EVAS_CALLBACK_KEY_DOWN,    OnEvasObjectKeyDown);
  evas_object_event_callback_del(mImageEvasObject, EVAS_CALLBACK_KEY_UP,      OnEvasObjectKeyUp);

  // Unregister the evas geometry callbacks.
  evas_object_event_callback_del(mImageEvasObject, EVAS_CALLBACK_MOVE,   OnEvasObjectMove);
  evas_object_event_callback_del(mImageEvasObject, EVAS_CALLBACK_RESIZE, OnEvasObjectResize);

  // Unregister the evas focus callbacks
  evas_object_event_callback_del(mImageEvasObject, EVAS_CALLBACK_FOCUS_IN,  OnEvasObjectFocusIn);
  evas_object_event_callback_del(mImageEvasObject, EVAS_CALLBACK_FOCUS_OUT, OnEvasObjectFocusOut);

  evas_event_callback_del(mEvas, EVAS_CALLBACK_CANVAS_FOCUS_IN,  OnEvasFocusIn);
  evas_event_callback_del(mEvas, EVAS_CALLBACK_CANVAS_FOCUS_OUT, OnEvasFocusOut);

  // Register the evas render callbacks
  evas_event_callback_del(mEvas, EVAS_CALLBACK_RENDER_POST, OnEvasRenderPost);

  // Unregister the elm focus callbacks
  evas_object_smart_callback_del(mElmFocusEvasObject, EVAS_OBJECT_FOCUSED_EVENT_NAME,   OnEvasObjectSmartFocused);
  evas_object_smart_callback_del(mElmFocusEvasObject, EVAS_OBJECT_UNFOCUSED_EVENT_NAME, OnEvasObjectSmartUnfocused);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Event callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////

void EvasEventHandler::OnEvasObjectMouseDown(void *data, Evas* evas, Evas_Object* evasObject, void* event)
{
  Evas_Event_Mouse_Down* eventMouseDown = static_cast<Evas_Event_Mouse_Down*>(event);

  Evas_Coord screenX = 0.0f, screenY = 0.0f;
  GetScreenPosition(eventMouseDown->canvas.x, eventMouseDown->canvas.y, evasObject, screenX, screenY);

  TouchPoint touchPoint = TouchPoint(TOUCH_DEVICE_ID, TouchPoint::Down, screenX, screenY);
  unsigned long timeStamp = eventMouseDown->timestamp;
  if (timeStamp < 1)
  {
    timeStamp = GetCurrentMilliSeconds();
  }

  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  pEvasPlugin->OnEvasObjectTouchEvent(touchPoint, timeStamp);
}

void EvasEventHandler::OnEvasObjectMouseUp(void *data, Evas* evas, Evas_Object* evasObject, void* event)
{
  Evas_Event_Mouse_Up* eventMouseUp = static_cast<Evas_Event_Mouse_Up*>(event);

  Evas_Coord screenX = 0.0f, screenY = 0.0f;
  GetScreenPosition(eventMouseUp->canvas.x, eventMouseUp->canvas.y, evasObject, screenX, screenY);

  TouchPoint touchPoint = TouchPoint(TOUCH_DEVICE_ID, TouchPoint::Up, screenX, screenY);
  unsigned long timeStamp = eventMouseUp->timestamp;
  if (timeStamp < 1)
  {
    timeStamp = GetCurrentMilliSeconds();
  }

  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  pEvasPlugin->OnEvasObjectTouchEvent(touchPoint, timeStamp);
}

void EvasEventHandler::OnEvasObjectMouseMove(void *data, Evas* evas, Evas_Object* evasObject, void* event)
{
  Evas_Event_Mouse_Move* eventMouseMove = static_cast<Evas_Event_Mouse_Move*>(event);

  Evas_Coord screenX = 0.0f, screenY = 0.0f;
  GetScreenPosition(eventMouseMove->cur.canvas.x, eventMouseMove->cur.canvas.y, evasObject, screenX, screenY);

  TouchPoint touchPoint = TouchPoint(TOUCH_DEVICE_ID, TouchPoint::Motion, screenX, screenY);
  unsigned long timeStamp = eventMouseMove->timestamp;
  if (timeStamp < 1)
  {
    timeStamp = GetCurrentMilliSeconds();
  }

  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  pEvasPlugin->OnEvasObjectTouchEvent(touchPoint, timeStamp);
}

void EvasEventHandler::OnEvasObjectMouseWheel(void *data, Evas* evas, Evas_Object* evasObject, void* event)
{
  Evas_Event_Mouse_Wheel* eventMouseWheel = static_cast<Evas_Event_Mouse_Wheel*>(event);

  Evas_Coord screenX = 0.0f, screenY = 0.0f;
  GetScreenPosition(eventMouseWheel->canvas.x, eventMouseWheel->canvas.y, evasObject, screenX, screenY);

  int direction = eventMouseWheel->direction;
  unsigned int modifiers = -1;  // TODO: Need to check evas modifier
  Vector2 point = Vector2(screenX, screenY);
  int z = eventMouseWheel->z;
  unsigned int timeStamp = eventMouseWheel->timestamp;
  if (timeStamp < 1)
  {
    timeStamp = GetCurrentMilliSeconds();
  }

  WheelEvent wheelEvent(WheelEvent::MOUSE_WHEEL, direction, modifiers, point, z, timeStamp);

  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  pEvasPlugin->OnEvasObjectWheelEvent(wheelEvent);
}

void EvasEventHandler::OnEvasObjectMultiTouchDown(void *data, Evas* evas, Evas_Object* evasObject, void* event)
{
  Evas_Event_Multi_Down* eventMultiDown = static_cast<Evas_Event_Multi_Down*>(event);

  Evas_Coord screenX = 0.0f, screenY = 0.0f;
  GetScreenPosition(eventMultiDown->canvas.x, eventMultiDown->canvas.y, evasObject, screenX, screenY);

  TouchPoint touchPoint = TouchPoint(eventMultiDown->device, TouchPoint::Down, screenX, screenY);
  unsigned long timeStamp = eventMultiDown->timestamp;
  if (timeStamp < 1)
  {
    timeStamp = GetCurrentMilliSeconds();
  }

  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  pEvasPlugin->OnEvasObjectTouchEvent(touchPoint, timeStamp);
}

void EvasEventHandler::OnEvasObjectMultiTouchUp(void *data, Evas* evas, Evas_Object* evasObject, void* event)
{
  Evas_Event_Multi_Up* eventMultiUp = static_cast<Evas_Event_Multi_Up*>(event);

  Evas_Coord screenX = 0.0f, screenY = 0.0f;
  GetScreenPosition(eventMultiUp->canvas.x, eventMultiUp->canvas.y, evasObject, screenX, screenY);

  TouchPoint touchPoint = TouchPoint(eventMultiUp->device, TouchPoint::Up, screenX, screenY);
  unsigned long timeStamp = eventMultiUp->timestamp;
  if (timeStamp < 1)
  {
    timeStamp = GetCurrentMilliSeconds();
  }

  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  pEvasPlugin->OnEvasObjectTouchEvent(touchPoint, timeStamp);
}

void EvasEventHandler::OnEvasObjectMultiTouchMove(void *data, Evas* evas, Evas_Object* evasObject, void* event)
{
  Evas_Event_Multi_Move* eventMultiMove = static_cast<Evas_Event_Multi_Move*>(event);

  Evas_Coord screenX = 0.0f, screenY = 0.0f;
  GetScreenPosition(eventMultiMove->cur.canvas.x, eventMultiMove->cur.canvas.y, evasObject, screenX, screenY);

  TouchPoint touchPoint = TouchPoint(eventMultiMove->device, TouchPoint::Motion, screenX, screenY);
  unsigned long timeStamp = eventMultiMove->timestamp;
  if (timeStamp < 1)
  {
    timeStamp = GetCurrentMilliSeconds();
  }

  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  pEvasPlugin->OnEvasObjectTouchEvent(touchPoint, timeStamp);
}

void EvasEventHandler::OnEvasObjectKeyDown(void *data, Evas* evas, Evas_Object* evasObject, void* event)
{
#ifdef DALI_EVASPLUGIN_USE_IMF_MANAGER
  Evas_Event_Key_Down* eventKeyDown = static_cast<Evas_Event_Key_Down*>(event);
  bool eventHandled = false;

  // If a device key then skip ecore_imf_context_filter_event.
  if (!KeyLookup::IsDeviceButton(eventKeyDown->keyname))
  {
    ImfManager imfManager = ImfManager::Get();
    if(imfManager)
    {
      Ecore_IMF_Context* imfContext = Dali::Internal::Adaptor::ImfManager::GetImplementation(imfManager).GetContext();
      if (imfContext)
      {
        // We're consuming key down event so we have to pass to IMF so that it can parse it as well.
        Ecore_IMF_Event_Key_Down imfEventKeyDown;
        ecore_imf_evas_event_key_down_wrap(eventKeyDown, &imfEventKeyDown);

        eventHandled = ecore_imf_context_filter_event(imfContext, ECORE_IMF_EVENT_KEY_DOWN, reinterpret_cast<Ecore_IMF_Event*>(&imfEventKeyDown));

        if (!eventHandled)
        {
          // Menu, home, back button must skip ecore_imf_context_filter_event.
          static const char* escapeKeyName = KeyLookup::GetKeyName(DALI_KEY_ESCAPE);
          static const char* returnKeyName = KeyLookup::GetKeyName(static_cast<Dali::KEY>(DALI_EXTENSION_INTERNAL_KEY_RETURN));
          static const char* kpEnterKeyName = KeyLookup::GetKeyName(static_cast<Dali::KEY>(DALI_EXTENSION_INTERNAL_KEY_KP_ENTER));
          if ((escapeKeyName && !strcmp(eventKeyDown->keyname, escapeKeyName)) ||
              (returnKeyName && !strcmp(eventKeyDown->keyname, returnKeyName)) ||
              (kpEnterKeyName && !strcmp(eventKeyDown->keyname, kpEnterKeyName)))
          {
            ecore_imf_context_reset(imfContext);
          }
        }
      }
    }
  }

  // If the event wasn't handled then we should send a key event.
  if (!eventHandled)
  {
    std::string keyName = eventKeyDown->keyname;
    std::string keyString = (eventKeyDown->string != NULL) ? eventKeyDown->string : "";
    int keyCode = KeyLookup::GetDaliKeyCode(eventKeyDown->keyname);
    int modifier = EvasModifierToEcoreModifier(eventKeyDown->modifiers);
    unsigned long timeStamp = eventKeyDown->timestamp;
    if (timeStamp < 1)
    {
      timeStamp = GetCurrentMilliSeconds();
    }

    KeyEvent keyEvent(keyName, keyString, keyCode, modifier, timeStamp, KeyEvent::Down);

    EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
    pEvasPlugin->OnEvasObjectKeyEvent(keyEvent);
  }
#endif
}

void EvasEventHandler::OnEvasObjectKeyUp(void *data, Evas* evas, Evas_Object* evasObject, void* event)
{
#ifdef DALI_EVASPLUGIN_USE_IMF_MANAGER
  Evas_Event_Key_Up* eventKeyUp = static_cast<Evas_Event_Key_Up*>(event);
  bool eventHandled = false;

  // Menu, home, back button must skip ecore_imf_context_filter_event.
  static const char* menuKeyName = KeyLookup::GetKeyName(DALI_KEY_MENU);
  static const char* homeKeyName = KeyLookup::GetKeyName(DALI_KEY_HOME);
  static const char* backKeyName = KeyLookup::GetKeyName(DALI_KEY_BACK);
  if ((menuKeyName && !strcmp(eventKeyUp->keyname, menuKeyName)) &&
      (homeKeyName && !strcmp(eventKeyUp->keyname, homeKeyName)) &&
      (backKeyName && !strcmp(eventKeyUp->keyname, backKeyName)))
  {
    ImfManager imfManager = ImfManager::Get();
    if(imfManager)
    {
      Ecore_IMF_Context* imfContext = Dali::Internal::Adaptor::ImfManager::GetImplementation(imfManager).GetContext();

      if (imfContext)
      {
        Ecore_IMF_Event_Key_Up imfEventKeyUp;
        ecore_imf_evas_event_key_up_wrap(eventKeyUp, &imfEventKeyUp);

        eventHandled = ecore_imf_context_filter_event(imfContext, ECORE_IMF_EVENT_KEY_UP, reinterpret_cast<Ecore_IMF_Event*>(&imfEventKeyUp));
      }
    }
  }

  if (!eventHandled)
  {
    std::string keyName = eventKeyUp->keyname;
    std::string keyString = (eventKeyUp->string != NULL) ? eventKeyUp->string : "";
    int keyCode = KeyLookup::GetDaliKeyCode(eventKeyUp->keyname);
    int modifier = EvasModifierToEcoreModifier(eventKeyUp->modifiers);
    unsigned long timeStamp = eventKeyUp->timestamp;
    if (timeStamp < 1)
    {
      timeStamp = GetCurrentMilliSeconds();
    }

    KeyEvent keyEvent(keyName, keyString, keyCode, modifier, timeStamp, KeyEvent::Up);

    EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
    pEvasPlugin->OnEvasObjectKeyEvent(keyEvent);
  }
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Geometry callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////

void EvasEventHandler::OnEvasObjectMove(void *data, Evas* evas, Evas_Object* evasObject, void* event)
{
  Rect<int> geometry;
  evas_object_geometry_get(evasObject, &geometry.x, &geometry.y, &geometry.width, &geometry.height);

  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  pEvasPlugin->OnEvasObjectMove(geometry);
}

void EvasEventHandler::OnEvasObjectResize(void *data, Evas* evas, Evas_Object* evasObject, void* event)
{
  Rect<int> geometry;
  evas_object_geometry_get(evasObject, &geometry.x, &geometry.y, &geometry.width, &geometry.height);

  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  pEvasPlugin->OnEvasObjectResize(geometry);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Focus callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////

void EvasEventHandler::OnEvasObjectFocusIn(void *data, Evas* evas, Evas_Object* evasObject, void* event)
{
  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  pEvasPlugin->OnEvasObjectFocusIn();
}

void EvasEventHandler::OnEvasObjectFocusOut(void *data, Evas* evas, Evas_Object* evasObject, void* event)
{
  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  pEvasPlugin->OnEvasObjectFocusOut();
}

void EvasEventHandler::OnEvasFocusIn(void *data, Evas* evas, void* event)
{
}

void EvasEventHandler::OnEvasFocusOut(void *data, Evas* evas, void* event)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Render callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////

void EvasEventHandler::OnEvasRenderPost(void *data, Evas* evas, void* event)
{
  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  pEvasPlugin->OnEvasPostRender();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Elm Access callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////

Eina_Bool EvasEventHandler::OnElmAccessActionHighlight(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo)
{
  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo(actionInfo, daliActionInfo);

  return pEvasPlugin->OnElmAccessActionHighlight(daliActionInfo);
}

Eina_Bool EvasEventHandler::OnElmAccessActionUnhighlight(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo)
{
  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo(actionInfo, daliActionInfo);

  return pEvasPlugin->OnElmAccessActionUnhighlight(daliActionInfo);
}

Eina_Bool EvasEventHandler::OnElmAccessActionHighlightNext(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo)
{
  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo(actionInfo, daliActionInfo);

  return pEvasPlugin->OnElmAccessActionHighlightNext(daliActionInfo);
}

Eina_Bool EvasEventHandler::OnElmAccessActionHighlightPrev(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo)
{
  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo(actionInfo, daliActionInfo);

  return pEvasPlugin->OnElmAccessActionHighlightPrev(daliActionInfo);
}

Eina_Bool EvasEventHandler::OnElmAccessActionActivate(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo)
{
  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo(actionInfo, daliActionInfo);

  return pEvasPlugin->OnElmAccessActionActivate(daliActionInfo);
}

Eina_Bool EvasEventHandler::OnElmAccessActionScroll(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo)
{
  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo(actionInfo, daliActionInfo);

  return pEvasPlugin->OnElmAccessActionScroll(daliActionInfo);
}

Eina_Bool EvasEventHandler::OnElmAccessActionUp(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo)
{
  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo(actionInfo, daliActionInfo);

  return pEvasPlugin->OnElmAccessActionUp(daliActionInfo);
}

Eina_Bool EvasEventHandler::OnElmAccessActionDown(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo)
{
  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo(actionInfo, daliActionInfo);

  return pEvasPlugin->OnElmAccessActionDown(daliActionInfo);
}

Eina_Bool EvasEventHandler::OnElmAccessActionBack(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo)
{
  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo(actionInfo, daliActionInfo);

  return pEvasPlugin->OnElmAccessActionBack(daliActionInfo);
}

Eina_Bool EvasEventHandler::OnElmAccessActionRead(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* actionInfo)
{
  EvasPluginEventInterface* pEvasPlugin = static_cast<EvasPluginEventInterface*>(data);
  Dali::Extension::Internal::AccessActionInfo daliActionInfo;
  ConvertActionInfo(actionInfo, daliActionInfo);

  return pEvasPlugin->OnElmAccessActionRead(daliActionInfo);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Elm Focus callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////

void EvasEventHandler::OnEvasObjectSmartFocused(void *data, Evas_Object* evasObject, void* event)
{
  EvasEventHandler* eventHandler = static_cast<EvasEventHandler*>(data);
  EvasPluginEventInterface& evasPlugin = eventHandler->GetEvasPluginInterface();

  if (eventHandler->mElmFocusEvasObject == evasObject)
  {
    Evas_Object* topWidget = elm_object_top_widget_get(evasObject);

    if (!strcmp("elm_win", elm_object_widget_type_get(topWidget)))
    {
      if (elm_win_focus_highlight_enabled_get(topWidget) == EINA_TRUE)
      {
        // To allow that KeyboardFocusManager can handle the keyboard focus
        KeyEvent fakeKeyEvent("", "", 0, 0, 100, KeyEvent::Down);

        evasPlugin.OnEvasObjectKeyEvent(fakeKeyEvent);
      }
    }

    evas_object_focus_set(eventHandler->mImageEvasObject, EINA_TRUE);
  }
}

void EvasEventHandler::OnEvasObjectSmartUnfocused(void *data, Evas_Object* evasObject, void* event)
{
  EvasEventHandler* eventHandler = static_cast<EvasEventHandler*>(data);
  if (eventHandler->mElmFocusEvasObject == evasObject)
  {
    evas_object_focus_set(eventHandler->mImageEvasObject, EINA_FALSE);
  }
}

}  // namespace Internal

}  // namespace Extension

}  // namespace Dali
