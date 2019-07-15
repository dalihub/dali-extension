#ifndef __DALI_EXTENSION_INTERNAL_EVAS_PLUGIN_VISIBILITY_INTERFACE__
#define __DALI_EXTENSION_INTERNAL_EVAS_PLUGIN_VISIBILITY_INTERFACE__

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

namespace Dali
{

namespace Extension
{

namespace Internal
{

class EvasPluginVisibilityInterface
{
public:
  /**
   * @brief Process the ecore wayland visibility.
   *
   * @param[in] visibility True is that ecore wayland window is show up and false is not
   */
  virtual void OnEcoreWlVisibility(bool visibility) = 0;
};

}  // namespace Internal

}  // namespace Extension

}  // namespace Dali
#endif
