#ifndef DALI_PLUGIN_WEB_ENGINE_SETTINGS_H
#define DALI_PLUGIN_WEB_ENGINE_SETTINGS_H

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/web-engine-settings.h>

struct Ewk_Settings;

namespace Dali
{
namespace Plugin
{

/**
 * @brief A class TizenWebEngineSettings for settings of chromium.
 */
class TizenWebEngineSettings : public Dali::WebEngineSettings
{

public:

  /**
   * @brief Constructor.
   */
  TizenWebEngineSettings( Ewk_Settings* );

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineSettings();

  /**
   *@brief Allow running mixed contents or not.
   *
   * @param[in] allowed if true, allow to run mixed contents,
   *      otherwise not allow
   */
  void AllowMixedContents( bool allowed ) override;

  /**
   * @brief Enable the spatial navigation or not.
   *
   * @param[in] enabled if true, use spatial navigation,
   *       otherwise to disable
   */
  void EnableSpatialNavigation( bool enabled ) override;

  /**
   * @brief Get the default font size.
   *
   * @return defaut font size.
   */
  uint32_t GetDefaultFontSize() const override;

  /**
   * @brief Set the default font size.
   *
   * @param[in] size a new default font size to set
   */
  void SetDefaultFontSize( uint32_t size ) override;

  /**
   * @brief Enables/disables web security.
   *
   * @param[in] enabled if true, to enable the web security
   *       otherwise to disable
   *
   */
  void EnableWebSecurity( bool enabled ) override;

  /**
   * @brief Allow/Disallow file access from external url
   *
   * @param[in] allowed if true, to allow file access from external url
   *       otherwise to disallow
   */
  void AllowFileAccessFromExternalUrl( bool allowed ) override;

  /**
   * @brief Check if javascript is enabled or not.
   *
   * @return true if enabled, false if disabled.
   */
  bool IsJavaScriptEnabled() const override;

  /**
   * @brief Enable/Disable javascript
   *
   * @param[in] enabled if true, to enable javascript
   *       otherwise to disable
   */
  void EnableJavaScript( bool enabled ) override;

  /**
   * @brief Enables/disables if the scripts can open new windows.
   *
   * @param[in] allowed if true, the scripts can open new windows,
   *       otherwise not
   */
  void AllowScriptsOpenWindows( bool allowed ) override;

  /**
   * @brief Check if images are loaded automatically or not.
   *
   * @return true if enabled, false if disabled.
   */
  bool AreImagesLoadedAutomatically() const override;

  /**
   * @brief Enable/Disable to load images automatically
   *
   * @param[in] automatic if true, to load images automatically,
   *       otherwise not
   */
  void AllowImagesLoadAutomatically( bool automatic ) override;

  /**
   * @brief Get the default encoding name.
   *
   * @return defaut encoding name.
   */
  std::string GetDefaultTextEncodingName() const override;

  /**
   * @brief Set the default encoding name.
   *
   * @param[in] defaultTextEncodingName a default encoding name to set
   */
  void SetDefaultTextEncodingName( const std::string& defaultTextEncodingName ) override;

private:

  Ewk_Settings* ewkSettings;

};
} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_WEB_ENGINE_SETTINGS_H
