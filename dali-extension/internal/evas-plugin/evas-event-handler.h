#ifndef DALI_EXTENSION_INTERNAL_EVAS_EVENT_HANDLER
#define DALI_EXTENSION_INTERNAL_EVAS_EVENT_HANDLER

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
#include <Evas.h>
#include <Elementary.h>
#include <Ecore_Wl2.h>
#include <Ecore_Input.h>
#include <Ecore_IMF.h>
#include <Ecore_IMF_Evas.h>

#include <string>
#include <vector>

// INTERNAL INCLUDES

namespace Dali
{

namespace Extension
{

namespace Internal
{

class EvasEventInterface;

class EvasEventHandler
{
public:
  /**
   * @brief Constructor
   *
   * @param[in] evasEventInterface Used to send event to evas plugin
   */
  EvasEventHandler( EvasEventInterface& evasEventInterface );

  /**
   * Destructor.
   */
  ~EvasEventHandler();

public:

  EvasEventInterface& GetEvasEventInterface()
  {
    return mEvasEventInterface;
  }

private:

  void EnableEcoreWl2Events();

  void DisableEcoreWl2Events();

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Event callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Called when the mouse down is received
   */
  static void OnEvasObjectMouseDown( void *data, Evas* evas, Evas_Object* evasObject, void* event );

  /**
   * @brief Called when the mouse up is received
   */
  static void OnEvasObjectMouseUp( void *data, Evas* evas, Evas_Object* evasObject, void* event );

  /**
   * @brief Called when the mouse move is received
   */
  static void OnEvasObjectMouseMove( void *data, Evas* evas, Evas_Object* evasObject, void* event );

  /**
   * @brief Called when the mouse wheel is received
   */
  static void OnEvasObjectMouseWheel( void *data, Evas* evas, Evas_Object* evasObject, void* event );

  /**
   * @brief Called when the multi-touch down is received
   */
  static void OnEvasObjectMultiTouchDown( void *data, Evas* evas, Evas_Object* evasObject, void* event );

  /**
   * @brief Called when the multi-touch up is received
   */
  static void OnEvasObjectMultiTouchUp( void *data, Evas* evas, Evas_Object* evasObject, void* event );

  /**
   * @brief Called when the multi-touch move is received
   */
  static void OnEvasObjectMultiTouchMove( void *data, Evas* evas, Evas_Object* evasObject, void* event );

  /**
   * @brief Called when key down is received
   */
  static void OnEvasObjectKeyDown( void *data, Evas* evas, Evas_Object* evasObject, void* event );

  /**
   * @brief Called when key up is received
   */
  static void OnEvasObjectKeyUp( void *data, Evas* evas, Evas_Object* evasObject, void* event );

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Geometry callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Called when move is received
   */
  static void OnEvasObjectMove( void *data, Evas* evas, Evas_Object* evasObject, void* event );

  /**
   * @brief Called when resize is received
   */
  static void OnEvasObjectResize( void *data, Evas* evas, Evas_Object* evasObject, void* event );

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Visibility callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Called when show is received
   */
  static void OnEvasObjectShow( void *data, Evas* evas, Evas_Object* evasObject, void* event );

  /**
   * @brief Called when hide is received
   */
  static void OnEvasObjectHide( void *data, Evas* evas, Evas_Object* evasObject, void* event );

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Focus callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Called when evas object focus in is received
   */
  static void OnEvasObjectFocusIn( void *data, Evas* evas, Evas_Object* evasObject, void* event );

  /**
   * @brief Called when evas object focus out is received
   */
  static void OnEvasObjectFocusOut( void *data, Evas* evas, Evas_Object* evasObject, void* event );

  /**
   * @brief Called when evas focus in is received
   */
  static void OnEvasFocusIn( void *data, Evas* evas, void* event );

  /**
   * @brief Called when evas focus out is received
   */
  static void OnEvasFocusOut( void *data, Evas* evas, void* event );

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Elm Access callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  static Eina_Bool OnElmAccessActionHighlight( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info );

  static Eina_Bool OnElmAccessActionUnhighlight( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info );

  static Eina_Bool OnElmAccessActionHighlightNext( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info );

  static Eina_Bool OnElmAccessActionHighlightPrev( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info );

  static Eina_Bool OnElmAccessActionActivate( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info );

  static Eina_Bool OnElmAccessActionScroll( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info );

  static Eina_Bool OnElmAccessActionUp( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info );

  static Eina_Bool OnElmAccessActionDown( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info );

  static Eina_Bool OnElmAccessActionBack( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info );

  static Eina_Bool OnElmAccessActionRead( void* data, Evas_Object* evasObject, Elm_Access_Action_Info* info );

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Elm Focus callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  static void OnEvasObjectSmartFocused( void* data, Evas_Object* evasObject, void* event );

  static void OnEvasObjectSmartUnfocused( void* data, Evas_Object* evasObject, void* event );

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Ecore Wl2 callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  static Eina_Bool OnEcoreWl2EventWindowVisibilityChange( void* data, int type, void* event );

private:

  EvasEventInterface&               mEvasEventInterface;
  std::vector<Ecore_Event_Handler*> mEcoreEventHandlers;
};

}  // namespace Internal

}  // namespace Extension

}  // namespace Dali

#endif
