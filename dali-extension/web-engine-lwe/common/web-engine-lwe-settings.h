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

#ifndef DALI_EXTENSION_WEB_ENGINE_LWE_SETTINGS_H
#define DALI_EXTENSION_WEB_ENGINE_LWE_SETTINGS_H

#include <LWEWebView.h>

#include <dali/devel-api/adaptor-framework/web-engine/web-engine-settings.h>
#include <dali/public-api/math/vector4.h>

#include <string>

namespace Dali
{
namespace Plugin
{
/**
 * LWE implementation of WebEngineSettings.
 *
 * SetSettings() is asynchronous in LWE. Keep one settings snapshot for the
 * lifetime of the WebContainer so consecutive writes cannot overwrite each
 * other with stale values read back from the engine.
 */
class WebEngineLweSettings : public Dali::WebEngineSettings
{
public:
  explicit WebEngineLweSettings(LWE::WebContainer* webContainer);
  ~WebEngineLweSettings() override = default;

  void SetBaseBackgroundColor(Dali::Vector4 color);

  void AllowMixedContents(bool allowed) override;
  void EnableSpatialNavigation(bool enabled) override;
  uint32_t GetDefaultFontSize() const override;
  void SetDefaultFontSize(uint32_t size) override;
  void EnableWebSecurity(bool enabled) override;
  void EnableCacheBuilder(bool enabled) override;
  void UseScrollbarThumbFocusNotifications(bool used) override;
  void EnableDoNotTrack(bool enabled) override;
  void AllowFileAccessFromExternalUrl(bool allowed) override;
  bool IsJavaScriptEnabled() const override;
  void EnableJavaScript(bool enabled) override;
  bool IsAutoFittingEnabled() const override;
  void EnableAutoFitting(bool enabled) override;
  bool ArePluginsEnabled() const override;
  void EnablePlugins(bool enabled) override;
  bool IsPrivateBrowsingEnabled() const override;
  void EnablePrivateBrowsing(bool enabled) override;
  bool IsLinkMagnifierEnabled() const override;
  void EnableLinkMagnifier(bool enabled) override;
  bool IsKeypadWithoutUserActionUsed() const override;
  void UseKeypadWithoutUserAction(bool used) override;
  bool IsAutofillPasswordFormEnabled() const override;
  void EnableAutofillPasswordForm(bool enabled) override;
  bool IsFormCandidateDataEnabled() const override;
  void EnableFormCandidateData(bool enabled) override;
  bool IsTextSelectionEnabled() const override;
  void EnableTextSelection(bool enabled) override;
  bool IsTextAutosizingEnabled() const override;
  void EnableTextAutosizing(bool enabled) override;
  bool IsArrowScrollEnabled() const override;
  void EnableArrowScroll(bool enable) override;
  bool IsClipboardEnabled() const override;
  void EnableClipboard(bool enabled) override;
  bool IsImePanelEnabled() const override;
  void EnableImePanel(bool enabled) override;
  void AllowScriptsOpenWindows(bool allowed) override;
  bool AreImagesLoadedAutomatically() const override;
  void AllowImagesLoadAutomatically(bool automatic) override;
  std::string GetDefaultTextEncodingName() const override;
  void SetDefaultTextEncodingName(const std::string& defaultTextEncodingName) override;
  bool SetViewportMetaTag(bool enable) override;
  bool SetForceZoom(bool enable) override;
  bool IsZoomForced() const override;
  bool SetTextZoomEnabled(bool enable) override;
  bool IsTextZoomEnabled() const override;
  void SetExtraFeature(const std::string& feature, bool enable) override;
  bool IsExtraFeatureEnabled(const std::string& feature) const override;
  bool SetExtraFeatureValue(const std::string& feature, const std::string& value) override;
  std::string GetExtraFeatureValue(const std::string& feature) const override;
  void SetImeStyle(int style) override;
  int GetImeStyle() const override;
  void SetDefaultAudioInputDevice(const std::string& deviceId) const override;
  void EnableDragAndDrop(bool enable) override;

private:
  void Apply();

private:
  LWE::WebContainer* mWebContainer;
  LWE::Settings      mSettings;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_EXTENSION_WEB_ENGINE_LWE_SETTINGS_H
