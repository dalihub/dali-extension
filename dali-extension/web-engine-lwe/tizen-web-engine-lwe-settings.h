#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_LWE_SETTINGS_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_LWE_SETTINGS_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <string> // dali/devel-api/adaptor-framework/web-engine/web-engine-settings.h uses std::string without including it

#include <dali/devel-api/adaptor-framework/web-engine/web-engine-settings.h>
#include <dali/public-api/math/vector4.h>

#include <LWEWebView.h>

namespace Dali
{
namespace Plugin
{
/**
 * @brief A class TizenWebEngineLweSettings for settings of LWE(Starfish).
 *
 * Most of DALi's WebEngineSettings API has no corresponding state in LWE's
 * current public Settings, so those methods stay no-op/default (matching the
 * previous NullWebEngineSettings behaviour) until LWE exposes the underlying
 * feature. Only the methods with a direct LWE::Settings/WebContainer
 * counterpart are wired to the engine; each is documented at the override.
 */
class TizenWebEngineLweSettings : public Dali::WebEngineSettings
{
public:
  /**
   * @brief Constructor.
   *
   * Takes a snapshot of the WebContainer's current settings and keeps it as
   * the single source of truth for this plugin instance. LWE::Settings is a
   * value type and WebContainer::SetSettings() applies asynchronously on the
   * LWE main thread, so re-fetching GetSettings() before every write would
   * read a stale snapshot and clobber a write that has not been applied yet.
   */
  TizenWebEngineLweSettings(LWE::WebContainer* webContainer);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineLweSettings() override;

  /**
   * @brief Sets the base background color directly on the cached settings
   * snapshot and applies it, without going through a fresh GetSettings()
   * round-trip. Used by TizenWebEngineLWE::SetDocumentBackgroundColor() so it
   * shares the same snapshot as the rest of this class instead of clobbering
   * settings written here (see class comment).
   */
  void SetBaseBackgroundColor(Dali::Vector4 color);

  /**
   * @copydoc Dali::WebEngineSettings::AllowMixedContents()
   */
  void AllowMixedContents(bool allowed) override;

  /**
   * @copydoc Dali::WebEngineSettings::EnableSpatialNavigation()
   */
  void EnableSpatialNavigation(bool enabled) override;

  /**
   * @copydoc Dali::WebEngineSettings::GetDefaultFontSize()
   */
  uint32_t GetDefaultFontSize() const override;

  /**
   * @copydoc Dali::WebEngineSettings::SetDefaultFontSize()
   */
  void SetDefaultFontSize(uint32_t size) override;

  /**
   * @copydoc Dali::WebEngineSettings::EnableWebSecurity()
   */
  void EnableWebSecurity(bool enabled) override;

  /**
   * @copydoc Dali::WebEngineSettings::EnableCacheBuilder()
   */
  void EnableCacheBuilder(bool enabled) override;

  /**
   * @copydoc Dali::WebEngineSettings::UseScrollbarThumbFocusNotifications()
   */
  void UseScrollbarThumbFocusNotifications(bool used) override;

  /**
   * @copydoc Dali::WebEngineSettings::EnableDoNotTrack()
   */
  void EnableDoNotTrack(bool enabled) override;

  /**
   * @copydoc Dali::WebEngineSettings::AllowFileAccessFromExternalUrl()
   */
  void AllowFileAccessFromExternalUrl(bool allowed) override;

  /**
   * @copydoc Dali::WebEngineSettings::IsJavaScriptEnabled()
   */
  bool IsJavaScriptEnabled() const override;

  /**
   * @copydoc Dali::WebEngineSettings::EnableJavaScript()
   */
  void EnableJavaScript(bool enabled) override;

  /**
   * @copydoc Dali::WebEngineSettings::IsAutoFittingEnabled()
   */
  bool IsAutoFittingEnabled() const override;

  /**
   * @copydoc Dali::WebEngineSettings::EnableAutoFitting()
   */
  void EnableAutoFitting(bool enabled) override;

  /**
   * @copydoc Dali::WebEngineSettings::ArePluginsEnabled()
   */
  bool ArePluginsEnabled() const override;

  /**
   * @copydoc Dali::WebEngineSettings::EnablePlugins()
   */
  void EnablePlugins(bool enabled) override;

  /**
   * @copydoc Dali::WebEngineSettings::IsPrivateBrowsingEnabled()
   */
  bool IsPrivateBrowsingEnabled() const override;

  /**
   * @copydoc Dali::WebEngineSettings::EnablePrivateBrowsing()
   */
  void EnablePrivateBrowsing(bool enabled) override;

  /**
   * @copydoc Dali::WebEngineSettings::IsLinkMagnifierEnabled()
   */
  bool IsLinkMagnifierEnabled() const override;

  /**
   * @copydoc Dali::WebEngineSettings::EnableLinkMagnifier()
   */
  void EnableLinkMagnifier(bool enabled) override;

  /**
   * @copydoc Dali::WebEngineSettings::IsKeypadWithoutUserActionUsed()
   */
  bool IsKeypadWithoutUserActionUsed() const override;

  /**
   * @copydoc Dali::WebEngineSettings::UseKeypadWithoutUserAction()
   */
  void UseKeypadWithoutUserAction(bool used) override;

  /**
   * @copydoc Dali::WebEngineSettings::IsAutofillPasswordFormEnabled()
   */
  bool IsAutofillPasswordFormEnabled() const override;

  /**
   * @copydoc Dali::WebEngineSettings::EnableAutofillPasswordForm()
   */
  void EnableAutofillPasswordForm(bool enabled) override;

  /**
   * @copydoc Dali::WebEngineSettings::IsFormCandidateDataEnabled()
   */
  bool IsFormCandidateDataEnabled() const override;

  /**
   * @copydoc Dali::WebEngineSettings::EnableFormCandidateData()
   */
  void EnableFormCandidateData(bool enabled) override;

  /**
   * @copydoc Dali::WebEngineSettings::IsTextSelectionEnabled()
   */
  bool IsTextSelectionEnabled() const override;

  /**
   * @copydoc Dali::WebEngineSettings::EnableTextSelection()
   */
  void EnableTextSelection(bool enabled) override;

  /**
   * @copydoc Dali::WebEngineSettings::IsTextAutosizingEnabled()
   */
  bool IsTextAutosizingEnabled() const override;

  /**
   * @copydoc Dali::WebEngineSettings::EnableTextAutosizing()
   */
  void EnableTextAutosizing(bool enabled) override;

  /**
   * @copydoc Dali::WebEngineSettings::IsArrowScrollEnabled()
   */
  bool IsArrowScrollEnabled() const override;

  /**
   * @copydoc Dali::WebEngineSettings::EnableArrowScroll()
   */
  void EnableArrowScroll(bool enable) override;

  /**
   * @copydoc Dali::WebEngineSettings::IsClipboardEnabled()
   */
  bool IsClipboardEnabled() const override;

  /**
   * @copydoc Dali::WebEngineSettings::EnableClipboard()
   */
  void EnableClipboard(bool enabled) override;

  /**
   * @copydoc Dali::WebEngineSettings::IsImePanelEnabled()
   */
  bool IsImePanelEnabled() const override;

  /**
   * @copydoc Dali::WebEngineSettings::EnableImePanel()
   */
  void EnableImePanel(bool enabled) override;

  /**
   * @copydoc Dali::WebEngineSettings::AllowScriptsOpenWindows()
   */
  void AllowScriptsOpenWindows(bool allowed) override;

  /**
   * @copydoc Dali::WebEngineSettings::AreImagesLoadedAutomatically()
   */
  bool AreImagesLoadedAutomatically() const override;

  /**
   * @copydoc Dali::WebEngineSettings::AllowImagesLoadAutomatically()
   */
  void AllowImagesLoadAutomatically(bool automatic) override;

  /**
   * @copydoc Dali::WebEngineSettings::GetDefaultTextEncodingName()
   */
  std::string GetDefaultTextEncodingName() const override;

  /**
   * @copydoc Dali::WebEngineSettings::SetDefaultTextEncodingName()
   */
  void SetDefaultTextEncodingName(const std::string& defaultTextEncodingName) override;

  /**
   * @copydoc Dali::WebEngineSettings::SetViewportMetaTag()
   */
  bool SetViewportMetaTag(bool enable) override;

  /**
   * @copydoc Dali::WebEngineSettings::SetForceZoom()
   */
  bool SetForceZoom(bool enable) override;

  /**
   * @copydoc Dali::WebEngineSettings::IsZoomForced()
   */
  bool IsZoomForced() const override;

  /**
   * @copydoc Dali::WebEngineSettings::SetTextZoomEnabled()
   */
  bool SetTextZoomEnabled(bool enable) override;

  /**
   * @copydoc Dali::WebEngineSettings::IsTextZoomEnabled()
   */
  bool IsTextZoomEnabled() const override;

  /**
   * @copydoc Dali::WebEngineSettings::SetExtraFeature()
   */
  void SetExtraFeature(const std::string& feature, bool enable) override;

  /**
   * @copydoc Dali::WebEngineSettings::IsExtraFeatureEnabled()
   */
  bool IsExtraFeatureEnabled(const std::string& feature) const override;

  /**
   * @copydoc Dali::WebEngineSettings::SetImeStyle()
   */
  void SetImeStyle(int style) override;

  /**
   * @copydoc Dali::WebEngineSettings::GetImeStyle()
   */
  int GetImeStyle() const override;

  /**
   * @copydoc Dali::WebEngineSettings::SetDefaultAudioInputDevice()
   */
  void SetDefaultAudioInputDevice(const std::string& deviceId) const override;

  /**
   * @copydoc Dali::WebEngineSettings::EnableDragAndDrop()
   */
  void EnableDragAndDrop(bool enable) override;

private:
  /**
   * @brief Pushes the cached settings snapshot to the engine.
   */
  void Apply();

  LWE::WebContainer* mWebContainer; ///< Not owned; outlives this settings object.
  LWE::Settings      mSettings;     ///< Cached snapshot; the only copy this class writes to.
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_LWE_SETTINGS_H
