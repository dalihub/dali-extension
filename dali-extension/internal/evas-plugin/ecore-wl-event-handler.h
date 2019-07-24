#ifndef __DALI_EXTENSION_INTERNAL_ECORE_X_EVENT_HANDLER__
#define __DALI_EXTENSION_INTERNAL_ECORE_X_EVENT_HANDLER__

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
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_Wl2.h>
#include <vector>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include "evas-plugin-visibility-interface.h"

namespace Dali
{

namespace Extension
{

namespace Internal
{

class EvasPluginVisibilityInterface;

class EcoreWlEventHandler
{
public:
  /**
   * @brief Constructor
   *
   * @param[in] window is id of ecore wayland window.
   * @param[in] evasPluginVisibilityInterface Used to send ecore wayland event to evas plugin
   */
  EcoreWlEventHandler(Ecore_Wl2_Window* window, EvasPluginVisibilityInterface& evasPluginVisibilityInterface);

  /**
   * @brief Destructor
   */
  ~EcoreWlEventHandler();

  /**
   * @brief Send the ecore wayland visibility.
   *
   * @param[in] visibility True is that ecore wayland window is show up and false is not
   */
  void SendEcoreWlVisibility(bool visibility);

private:
  EvasPluginVisibilityInterface& mEvasPluginVisibilityInterface;

  struct Impl;
  Impl* mImpl;
};

}  // namespace Internal

}  // namespace Extension

}  // namespace Dali

#endif
