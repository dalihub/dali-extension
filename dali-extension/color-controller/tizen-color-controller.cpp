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
#include "tizen-color-controller.h"

// EXTERNAL INCLUDES
#include <dali/public-api/common/constants.h>

// The plugin factories
extern "C" DALI_EXPORT_API Dali::ColorControllerPlugin* CreateColorControllerPlugin(void)
{
  return new Dali::Plugin::TizenColorController;
}

namespace Dali
{

namespace Plugin
{

TizenColorController::TizenColorController()
{
}

TizenColorController::~TizenColorController()
{
}

bool TizenColorController::RetrieveColor(const std::string& colorCode, Vector4& colorValue) const
{
  colorValue = Color::WHITE;
  return true;
}

bool TizenColorController::RetrieveColor(const std::string& colorCode, Vector4& textColor, Vector4& textOutlineColor, Vector4& textShadowColor) const
{
  textColor        = Color::WHITE;
  textOutlineColor = Color::WHITE;
  textShadowColor  = Color::WHITE;
  return true;
}

} // namespace Plugin

} //namespace Dali