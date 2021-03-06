#ifndef DALI_TIZEN_COLOR_CONTROLLER_PLUGIN_H
#define DALI_TIZEN_COLOR_CONTROLLER_PLUGIN_H

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
#include <string>
#include <dali/public-api/math/vector4.h>
#include <dali/devel-api/adaptor-framework/color-controller-plugin.h>

namespace Dali
{

namespace Plugin
{

class TizenColorController : public Dali::ColorControllerPlugin
{

public:

  /**
   * @brief TizenColorController constructor.
   */
  TizenColorController();

  /**
   * @brief TizenColorController destructor.
   */
  virtual ~TizenColorController();

public:
  /**
   * @copydoc Dali::ColorControllerPlugin::RetrieveColor(const std::string&, Vector4&)
   */
  virtual bool RetrieveColor( const std::string& colorCode, Vector4& colorValue ) const;

  /**
   * @copydoc Dali::ColorControllerPlugin::RetrieveColor(const std::string&, Vector4&, Vector4&, Vector4&)
   */
  virtual bool RetrieveColor( const std::string& colorCode , Vector4& textColor, Vector4& textOutlineColor, Vector4& textShadowColor ) const;
};

}
}
#endif // DALI_TIZEN_COLOR_CONTROLLER_PLUGIN_H
