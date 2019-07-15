#ifndef __DALI_EXTENSION_INTERNAL_EVAS_EVENT_HANDLER__
#define __DALI_EXTENSION_INTERNAL_EVAS_EVENT_HANDLER__

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
#include <Evas.h>

#include <Elementary.h>
#include <Ecore_Wl2.h>
#include <Ecore_Input.h>
#include <Ecore_IMF.h>
#include <Ecore_IMF_Evas.h>

#include <dali/public-api/events/touch-point.h>
#include <dali/public-api/events/wheel-event.h>
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/math/rect.h>
// #include <dali/integration-api/adaptors/input-method-context.h>

#include <string>

// INTERNAL INCLUDES
#include "evas-plugin-event-interface.h"



namespace Dali
{
template <typename T>
class Rect;

struct TouchPoint;
struct WheelEvent;
struct KeyEvent;

namespace Extension
{

namespace Internal
{

class EvasPluginEventInterface;

class EvasEventHandler
{
public:
  /**
   * @brief Constructor
   *
   * @param[in] imageEvasObject is stream showing the pixmap which is drawn by dali
   *
   *
   * @param[in] evasPluginEventInterface Used to send event to evas plugin
   */
  EvasEventHandler(Evas_Object* imageEvasObject,
                   Evas_Object* elmAccessEvasObject,
                   Evas_Object* elmFocusEvasObject,
                   EvasPluginEventInterface& evasPluginEventInterface);

  /**
   * Destructor.
   */
  ~EvasEventHandler();

public:

  EvasPluginEventInterface& GetEvasPluginInterface() { return mEvasPluginEventInterface; }

private:

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Event callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Called when the mouse down is received
   */
  static void OnEvasObjectMouseDown(void *data, Evas* evas, Evas_Object* evasObject, void* event);

  /**
   * @brief Called when the mouse up is received
   */
  static void OnEvasObjectMouseUp(void *data, Evas* evas, Evas_Object* evasObject, void* event);

  /**
   * @brief Called when the mouse move is received
   */
  static void OnEvasObjectMouseMove(void *data, Evas* evas, Evas_Object* evasObject, void* event);

  /**
   * @brief Called when the mouse wheel is received
   */
  static void OnEvasObjectMouseWheel(void *data, Evas* evas, Evas_Object* evasObject, void* event);

  /**
   * @brief Called when the multi-touch down is received
   */
  static void OnEvasObjectMultiTouchDown(void *data, Evas* evas, Evas_Object* evasObject, void* event);

  /**
   * @brief Called when the multi-touch up is received
   */
  static void OnEvasObjectMultiTouchUp(void *data, Evas* evas, Evas_Object* evasObject, void* event);

  /**
   * @brief Called when the multi-touch move is received
   */
  static void OnEvasObjectMultiTouchMove(void *data, Evas* evas, Evas_Object* evasObject, void* event);

  /**
   * @brief Called when key down is received
   */
  static void OnEvasObjectKeyDown(void *data, Evas* evas, Evas_Object* evasObject, void* event);

  /**
   * @brief Called when key up is received
   */
  static void OnEvasObjectKeyUp(void *data, Evas* evas, Evas_Object* evasObject, void* event);

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Geometry callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Called when move is received
   */
  static void OnEvasObjectMove(void *data, Evas* evas, Evas_Object* evasObject, void* event);

  /**
   * @brief Called when resize is received
   */
  static void OnEvasObjectResize(void *data, Evas* evas, Evas_Object* evasObject, void* event);

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Focus callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Called when evas object focus in is received
   */
  static void OnEvasObjectFocusIn(void *data, Evas* evas, Evas_Object* evasObject, void* event);

  /**
   * @brief Called when evas object focus out is received
   */
  static void OnEvasObjectFocusOut(void *data, Evas* evas, Evas_Object* evasObject, void* event);

  /**
   * @brief Called when evas focus in is received
   */
  static void OnEvasFocusIn(void *data, Evas* evas, void* event);

  /**
   * @brief Called when evas focus out is received
   */
  static void OnEvasFocusOut(void *data, Evas* evas, void* event);

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Called when evas render post
   */
  static void OnEvasRenderPost(void *data, Evas* evas, void* event);

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Elm Access callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  static Eina_Bool OnElmAccessActionHighlight(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info);

  static Eina_Bool OnElmAccessActionUnhighlight(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info);

  static Eina_Bool OnElmAccessActionHighlightNext(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info);

  static Eina_Bool OnElmAccessActionHighlightPrev(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info);

  static Eina_Bool OnElmAccessActionActivate(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info);

  static Eina_Bool OnElmAccessActionScroll(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info);

  static Eina_Bool OnElmAccessActionUp(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info);

  static Eina_Bool OnElmAccessActionDown(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info);

  static Eina_Bool OnElmAccessActionBack(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info);

  static Eina_Bool OnElmAccessActionRead(void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info);

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Elm Focus callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  static void OnEvasObjectSmartFocused(void *data, Evas_Object* evasObject, void* event);

  static void OnEvasObjectSmartUnfocused(void *data, Evas_Object* evasObject, void* event);

private:
  EvasPluginEventInterface& mEvasPluginEventInterface;

  Evas* mEvas;
  Evas_Object* mImageEvasObject;
  Evas_Object* mElmAccessEvasObject;
  Evas_Object* mElmFocusEvasObject;
};

}  // namespace Internal

}  // namespace Extension

}  // namespace Dali

#endif
