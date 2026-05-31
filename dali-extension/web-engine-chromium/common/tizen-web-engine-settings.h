#ifndef DALI_PLUGIN_WEB_ENGINE_SETTINGS_H
#define DALI_PLUGIN_WEB_ENGINE_SETTINGS_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

#include <dali/devel-api/adaptor-framework/web-engine/web-engine-settings.h>

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
  TizenWebEngineSettings(Ewk_Settings*);

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
  void AllowMixedContents(bool allowed) override;

  /**
   * @brief Enable the spatial navigation or not.
   *
   * @param[in] enabled if true, use spatial navigation,
   *       otherwise to disable
   */
  void EnableSpatialNavigation(bool enabled) override;

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
  void SetDefaultFontSize(uint32_t size) override;

  /**
   * @brief Enables/disables web security.
   *
   * @param[in] enabled if true, to enable the web security
   *       otherwise to disable
   *
   */
  void EnableWebSecurity(bool enabled) override;

  /**
   * @brief Enables/disables cache builder.
   *
   * @param[in] enabled if true, to enable cache builder
   *        otherwise to disable
   */
  void EnableCacheBuilder(bool enabled) override;

  /**
   * @brief Enables/disables use scrollbar thumb focus notifications.
   *
   * @param[in] used if true, to enable use scrollbar thumb focus notifications
   *        otherwise to disable
   */
  void UseScrollbarThumbFocusNotifications(bool used) override;

  /**
   * @brief Enables/disables do not track.
   *
   * @param[in] enabled if true, to enable do not track
   *        otherwise to disable
   */
  void EnableDoNotTrack(bool enabled) override;

  /**
   * @brief Allow/Disallow file access from external url
   *
   * @param[in] allowed if true, to allow file access from external url
   *       otherwise to disallow
   */
  void AllowFileAccessFromExternalUrl(bool allowed) override;

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
  void EnableJavaScript(bool enabled) override;

  /**
   * @brief Check if auto fitting is enabled or not.
   *
   * @return true if enabled, false if disabled.
   */
  bool IsAutoFittingEnabled() const override;

  /**
   * @brief Enable/Disable auto fitting
   *
   * @param[in] enabled if true, to enable auto fitting
   *       otherwise to disable
   */
  void EnableAutoFitting(bool enabled) override;

  /**
   * @brief Check if plugins are enabled or not.
   *
   * @return true if enabled, false if disabled.
   */
  bool ArePluginsEnabled() const override;

  /**
   * @brief Enable/Disable plugins
   *
   * @param[in] enabled if true, to enable plugins
   *       otherwise to disable
   */
  void EnablePlugins(bool enabled) override;

  /**
   * @brief Check if private browsing is enabled or not.
   *
   * @return true if enabled, false if disabled.
   */
  bool IsPrivateBrowsingEnabled() const override;

  /**
   * @brief Enable/Disable private browsing
   *
   * @param[in] enabled if true, to enable private browsing
   *       otherwise to disable
   */
  void EnablePrivateBrowsing(bool enabled) override;

  /**
   * @brief Check if link magnifier is enabled or not.
   *
   * @return  true if enabled, false if disabled.
   */
  bool IsLinkMagnifierEnabled() const override;

  /**
   * @brief Enable/Disable link magnifier
   *
   * @param[in] enabled if true, to enable link magnifier
   *        otherwise to disable
   */
  void EnableLinkMagnifier(bool enabled) override;

  /**
   * @brief Check if keypad without user action is used or not.
   *
   * @return true if used, false if unused.
   */
  bool IsKeypadWithoutUserActionUsed() const override;

  /**
   * @brief Used/Unused keypad without user action
   *
   * @param[in] used if true, to use keypad without user action
   *       otherwise to unused
   */
  void UseKeypadWithoutUserAction(bool used) override;

  /**
   * @brief Check if autofill password form is enabled or not.
   *
   * @return true if enabled, false if disabled.
   */
  bool IsAutofillPasswordFormEnabled() const override;

  /**
   * @brief Enable/Disable autofill password form
   *
   * @param[in] enabled if true, to enable autofill password form
   *       otherwise to disable
   */
  void EnableAutofillPasswordForm(bool enabled) override;

  /**
   * @brief Check if form candidate data is enabled or not.
   *
   * @return true if enabled, false if disabled.
   */
  bool IsFormCandidateDataEnabled() const override;

  /**
   * @brief Enable/Disable form candidate data
   *
   * @param[in] enabled if true, to enable form candidate data
   *       otherwise to disable
   */
  void EnableFormCandidateData(bool enabled) override;

  /**
   * @brief Check if text selection is enabled or not.
   *
   * @return true if enabled, false if disabled.
   */
  bool IsTextSelectionEnabled() const override;

  /**
   * @brief Enable/Disable text selection
   *
   * @param[in] enabled if true, to enable text selection
   *       otherwise to disable
   */
  void EnableTextSelection(bool enabled) override;

  /**
   * @brief Check if text autosizing is enabled or not.
   *
   * @return true if enabled, false if disabled.
   */
  bool IsTextAutosizingEnabled() const override;

  /**
   * @brief Enable/Disable text autosizing
   *
   * @param[in] enabled if true, to enable text autosizing
   *       otherwise to disable
   */
  void EnableTextAutosizing(bool enabled) override;

  /**
   * @brief Check if arrow scroll is enabled or not.
   *
   * @return true if enabled, false if disabled.
   */
  bool IsArrowScrollEnabled() const override;

  /**
   * @brief Enable/Disable arrow scroll
   *
   * @param[in] enabled if true, to enable arrow scroll
   *        otherwise to disable
   */
  void EnableArrowScroll(bool enabled) override;

  /**
   * @brief Check if clipboard is enabled or not.
   *
   * @return true if enabled, false if disabled.
   */
  bool IsClipboardEnabled() const override;

  /**
   * @brief Enable/Disable clipboard
   *
   * @param[in] enabled if true, to enable clipboard
   *        otherwise to disable
   */
  void EnableClipboard(bool enabled) override;

  /**
   * @brief Check if ime panel is enabled or not.
   *
   * @return true if enabled, false if disabled.
   */
  bool IsImePanelEnabled() const override;

  /**
   * @brief Enable/Disable ime panel
   *
   * @param[in] enabled if true, to enable ime panel
   *        otherwise to disable
   */
  void EnableImePanel(bool enabled) override;

  /**
   * @brief Enables/disables if the scripts can open new windows.
   *
   * @param[in] allowed if true, the scripts can open new windows,
   *       otherwise not
   */
  void AllowScriptsOpenWindows(bool allowed) override;

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
  void AllowImagesLoadAutomatically(bool automatic) override;

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
  void SetDefaultTextEncodingName(const std::string& defaultTextEncodingName) override;

  /**
   * @brief Enables/disables the viewport meta tag.
   *
   * By default, the viewport meta tag is enabled on mobile and wearable,
   * but it is disabled on TV.
   *
   * @param[in] enable @c true to enable the viewport meta tag
   *               @c false to disable
   *
   * @return @c true on success or @c false on failure
   */
  bool SetViewportMetaTag(bool enable) override;

  /**
   * @brief Requests setting of force zoom.
   *
   * @param[in] enable to force zoom
   *
   * @return @c true on success or @c false on failure
   */
  bool SetForceZoom(bool enable) override;

  /**
   * @brief Returns the force zoom status.
   *
   * @return @c true if enable force zoom or @c false.
   */
  bool IsZoomForced() const override;

  /**
   * @brief Requests setting use of text zoom.
   *
   * @param[in] enable to text zoom.
   *
   * @return @c true on success or @c false on failure
   */
  bool SetTextZoomEnabled(bool enable) override;

  /**
   * @brief Returns whether text zoom is enabled or not.
   *
   * @return @c true if enable text zoom or @c false.
   */
  bool IsTextZoomEnabled() const override;

  /**
   * @brief Requests enables/disables to the specific extra feature
   *
   * @param[in] feature feature name
   * @param[in] enable @c true to enable the specific extra feature
   *        @c false to disable
   */
  void SetExtraFeature(const std::string& feature, bool enable) override;

  /**
   * @brief Returns enable/disable to the specific extra feature
   *
   * @param[in] feature feature name
   *
   * @return @c true on enable or @c false on disable
   */
  bool IsExtraFeatureEnabled(const std::string& feature) const override;

  /**
   * @brief Sets the style of IME.
   *
   * @param[in] style @c IME_STYLE_FULL       full IME style
   *                  @c IME_STYLE_FLOATING   floating IME style
   *                  @c IME_STYLE_DYNAMIC    dynamic IME style
   *
   */
  void SetImeStyle(int style) override;

  /**
   * @brief Gets the style of IME.
   *
   * @return @c IME_STYLE_FULL       full IME style
   *         @c IME_STYLE_FLOATING   floating IME style
   *         @c IME_STYLE_DYNAMIC    dynamic IME style
   */
  int GetImeStyle() const override;

  /**
   * @brief Sets default audio input device
   *
   * @param[in] deviceId default device ID
   */
  void SetDefaultAudioInputDevice(const std::string& deviceId) const override;

  /**
   * @brief Enables Drag&Drop or not
   *
   * @param[in] enable if true, to enable Drag&Drop
   *       otherwise to disable
   */
  void EnableDragAndDrop(bool enable) override;

private:
  Ewk_Settings* ewkSettings;
};
} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_WEB_ENGINE_SETTINGS_H
