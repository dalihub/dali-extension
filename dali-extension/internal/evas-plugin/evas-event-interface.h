#ifndef DALI_EXTENSION_INTERNAL_EVAS_EVENT_INTERFACE_H
#define DALI_EXTENSION_INTERNAL_EVAS_EVENT_INTERFACE_H

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
#include <dali/integration-api/events/key-event-integ.h>
#include <dali/integration-api/events/point.h>
#include <dali/integration-api/events/wheel-event-integ.h>

namespace Dali
{

template<typename T>
class Rect;

struct TouchPoint;
struct WheelEvent;
struct KeyEvent;

namespace Extension
{

namespace Internal
{

class EvasWrapper;

enum AccessActionType
{
  ACCESS_ACTION_FIRST = -1,

  ACCESS_ACTION_HIGHLIGHT, /* highlight an object */

  ACCESS_ACTION_UNHIGHLIGHT, /* unhighlight an object */

  ACCESS_ACTION_HIGHLIGHT_NEXT, /* set highlight to next object */

  ACCESS_ACTION_HIGHLIGHT_PREV, /* set highlight to previous object */

  ACCESS_ACTION_ACTIVATE, /* activate a highlight object */

  ACCESS_ACTION_SCROLL, /* scroll if one of highlight object parents is scrollable */

  ACCESS_ACTION_UP, /* change value up of highlight object */

  ACCESS_ACTION_DOWN, /* change value down of highlight object */

  ACCESS_ACTION_BACK, /* go back to a previous view (ex: pop naviframe item) */

  ACCESS_ACTION_READ, /* highlight an object */

  ACCESS_ACTION_LAST
};

struct AccessActionInfo
{
  int          x;
  int          y;
  unsigned int mouseType; /* 0: mouse down
                             1: mouse move
                             2: mouse up   */
  AccessActionType actionType;
  AccessActionType actionBy;
  bool             highlightCycle : 1;
  unsigned int     timeStamp;
};

class EvasEventInterface
{
public:
  /**
   * @brief Gets an EvasWrapper instance to connect events
   *
   * @return The EvasWrapper instance
   */
  virtual EvasWrapper* GetEvasWrapper() const = 0;

  /**
   * @brief Handle evas object event
   *
   * @param[in] touchPoint is the information of touch
   * @param[in] timeStamp is that touch is occured
   */
  virtual void OnEvasObjectTouchEvent(Dali::Integration::Point& touchPoint, unsigned long timeStamp) = 0;

  /**
   * @brief Handle evas object event
   *
   * @param[in] wheelEvent is the information of wheel
   */
  virtual void OnEvasObjectWheelEvent(Dali::Integration::WheelEvent& wheelEvent) = 0;

  /**
   * @brief Handle evas object event
   *
   * @param[in] keyEvent is the information of key
   */
  virtual void OnEvasObjectKeyEvent(Dali::Integration::KeyEvent& keyEvent) = 0;

  /**
   * @brief Handle evas object geometry
   *
   * @param geometry is the move information of evas object
   */
  virtual void OnEvasObjectMove(const Rect<int>& geometry) = 0;

  /**
   * @brief Handle evas object geometry
   *
   * @param geometry is the resize information of evas object
   */
  virtual void OnEvasObjectResize(const Rect<int>& geometry) = 0;

  /**
   * @brief Handle evas object visibility
   *
   * @param visible is the visibility information of evas object
   */
  virtual void OnEvasObjectVisiblityChanged(bool visible) = 0;

  /**
   * @brief Handle evas object focus in
   */
  virtual void OnEvasObjectFocusIn() = 0;

  /**
   * @brief Handle evas object focus out
   */
  virtual void OnEvasObjectFocusOut() = 0;

  /**
   * @brief Called when the accessibility action event dispatched from elm_access.
   * @param[in] accessActionInfo elm accessibility action information structure
   * @return True if the event was handled
   */
  virtual bool OnElmAccessibilityActionEvent(AccessActionInfo& accessActionInfo) = 0;

  /**
   * @brief Process the ecore wayland visibility.
   *
   * @param[in] visible True is that ecore wayland window is show up and false is not
   */
  virtual void OnEcoreWl2VisibilityChange(bool visible) = 0;
};

} // namespace Internal

} // namespace Extension

} // namespace Dali

#endif
