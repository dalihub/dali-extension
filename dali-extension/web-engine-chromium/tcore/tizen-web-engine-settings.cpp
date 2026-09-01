/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

#include <tizen-web-engine-settings.h>

#include <dali/integration-api/debug.h>

#include <wv_settings.h>
#include <wv_settings_product.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineSettings::TizenWebEngineSettings(wv_settings_h settings)
: wvSettings(settings)
{
}

TizenWebEngineSettings::~TizenWebEngineSettings()
{
}

void TizenWebEngineSettings::AllowMixedContents(bool allowed)
{
  wv_settings_mixed_contents_set(wvSettings, allowed);
}

void TizenWebEngineSettings::EnableSpatialNavigation(bool enabled)
{
  wv_settings_spatial_navigation_enabled_set(wvSettings, enabled);
}

uint32_t TizenWebEngineSettings::GetDefaultFontSize() const
{
  return wv_settings_default_font_size_get(wvSettings);
}

void TizenWebEngineSettings::SetDefaultFontSize(uint32_t size)
{
  wv_settings_default_font_size_set(wvSettings, size);
}

void TizenWebEngineSettings::EnableWebSecurity(bool enabled)
{
  // WV GAP (WV_REQUIREMENTS.md B): wv_settings_web_security_enabled_set() is
  // not declared by the target WV headers, so the request is ignored.
}

void TizenWebEngineSettings::EnableCacheBuilder(bool enabled)
{
  wv_settings_cache_builder_enabled_set(wvSettings, enabled);
}

void TizenWebEngineSettings::UseScrollbarThumbFocusNotifications(bool used)
{
  wv_settings_uses_scrollbar_thumb_focus_notifications_set(wvSettings, used);
}

void TizenWebEngineSettings::EnableDoNotTrack(bool enabled)
{
  wv_settings_do_not_track_set(wvSettings, enabled);
}

void TizenWebEngineSettings::AllowFileAccessFromExternalUrl(bool allowed)
{
  wv_settings_allow_file_access_from_external_url_set(wvSettings, allowed);
}

bool TizenWebEngineSettings::IsJavaScriptEnabled() const
{
  return wv_settings_javascript_enabled_get(wvSettings);
}

void TizenWebEngineSettings::EnableJavaScript(bool enabled)
{
  wv_settings_javascript_enabled_set(wvSettings, enabled);
}

bool TizenWebEngineSettings::IsAutoFittingEnabled() const
{
  return wv_settings_auto_fitting_get(wvSettings);
}

void TizenWebEngineSettings::EnableAutoFitting(bool enabled)
{
  wv_settings_auto_fitting_set(wvSettings, enabled);
}

bool TizenWebEngineSettings::ArePluginsEnabled() const
{
  return wv_settings_plugins_enabled_get(wvSettings);
}

void TizenWebEngineSettings::EnablePlugins(bool enabled)
{
  wv_settings_plugins_enabled_set(wvSettings, enabled);
}

bool TizenWebEngineSettings::IsPrivateBrowsingEnabled() const
{
  return wv_settings_private_browsing_enabled_get(wvSettings);
}

void TizenWebEngineSettings::EnablePrivateBrowsing(bool enabled)
{
  wv_settings_private_browsing_enabled_set(wvSettings, enabled);
}

bool TizenWebEngineSettings::IsLinkMagnifierEnabled() const
{
  return wv_settings_link_magnifier_enabled_get(wvSettings);
}

void TizenWebEngineSettings::EnableLinkMagnifier(bool enabled)
{
  wv_settings_link_magnifier_enabled_set(wvSettings, enabled);
}

bool TizenWebEngineSettings::IsKeypadWithoutUserActionUsed() const
{
  return wv_settings_uses_keypad_without_user_action_get(wvSettings);
}

void TizenWebEngineSettings::UseKeypadWithoutUserAction(bool used)
{
  wv_settings_uses_keypad_without_user_action_set(wvSettings, used);
}

bool TizenWebEngineSettings::IsAutofillPasswordFormEnabled() const
{
  return wv_settings_autofill_password_form_enabled_get(wvSettings);
}

void TizenWebEngineSettings::EnableAutofillPasswordForm(bool enabled)
{
  wv_settings_autofill_password_form_enabled_set(wvSettings, enabled);
}

bool TizenWebEngineSettings::IsFormCandidateDataEnabled() const
{
  return wv_settings_form_candidate_data_enabled_get(wvSettings);
}

void TizenWebEngineSettings::EnableFormCandidateData(bool enabled)
{
  wv_settings_form_candidate_data_enabled_set(wvSettings, enabled);
}

bool TizenWebEngineSettings::IsTextSelectionEnabled() const
{
  return wv_settings_text_selection_enabled_get(wvSettings);
}

void TizenWebEngineSettings::EnableTextSelection(bool enabled)
{
  wv_settings_text_selection_enabled_set(wvSettings, enabled);
}

bool TizenWebEngineSettings::IsTextAutosizingEnabled() const
{
  return wv_settings_text_autosizing_enabled_get(wvSettings);
}

void TizenWebEngineSettings::EnableTextAutosizing(bool enabled)
{
  wv_settings_text_autosizing_enabled_set(wvSettings, enabled);
}

bool TizenWebEngineSettings::IsArrowScrollEnabled() const
{
  return wv_settings_uses_arrow_scroll_get(wvSettings);
}

void TizenWebEngineSettings::EnableArrowScroll(bool enabled)
{
  wv_settings_uses_arrow_scroll_set(wvSettings, enabled);
}

bool TizenWebEngineSettings::IsClipboardEnabled() const
{
  return wv_settings_clipboard_enabled_get(wvSettings);
}

void TizenWebEngineSettings::EnableClipboard(bool enabled)
{
  wv_settings_clipboard_enabled_set(wvSettings, enabled);
}

bool TizenWebEngineSettings::IsImePanelEnabled() const
{
  return wv_settings_ime_panel_enabled_get(wvSettings);
}

void TizenWebEngineSettings::EnableImePanel(bool enabled)
{
  wv_settings_ime_panel_enabled_set(wvSettings, enabled);
}

void TizenWebEngineSettings::AllowScriptsOpenWindows(bool allowed)
{
  wv_settings_scripts_window_open_set(wvSettings, allowed);
}

bool TizenWebEngineSettings::AreImagesLoadedAutomatically() const
{
  return wv_settings_loads_images_automatically_get(wvSettings);
}

void TizenWebEngineSettings::AllowImagesLoadAutomatically(bool automatic)
{
  wv_settings_loads_images_automatically_set(wvSettings, automatic);
}

std::string TizenWebEngineSettings::GetDefaultTextEncodingName() const
{
  const char* name = wv_settings_default_text_encoding_name_get(wvSettings);
  return name ? std::string(name) : std::string();
}

void TizenWebEngineSettings::SetDefaultTextEncodingName(const std::string& defaultTextEncodingName)
{
  wv_settings_default_text_encoding_name_set(wvSettings, defaultTextEncodingName.c_str());
}

bool TizenWebEngineSettings::SetViewportMetaTag(bool enable)
{
  // WV GAP (WV_REQUIREMENTS.md B): wv_settings_viewport_meta_tag_set() is not
  // declared by the target WV headers; report the setting as not applied.
  return false;
}

bool TizenWebEngineSettings::SetForceZoom(bool enable)
{
  return wv_settings_force_zoom_set(wvSettings, enable);
}

bool TizenWebEngineSettings::IsZoomForced() const
{
  return wv_settings_force_zoom_get(wvSettings);
}

bool TizenWebEngineSettings::SetTextZoomEnabled(bool enable)
{
  return wv_settings_text_zoom_enabled_set(wvSettings, enable);
}

bool TizenWebEngineSettings::IsTextZoomEnabled() const
{
  return wv_settings_text_zoom_enabled_get(wvSettings);
}

void TizenWebEngineSettings::SetExtraFeature(const std::string& feature, bool enable)
{
  wv_settings_extra_feature_set(wvSettings, feature.c_str(), enable ? "true" : "false");
}

bool TizenWebEngineSettings::IsExtraFeatureEnabled(const std::string& feature) const
{
  return wv_settings_extra_feature_get(wvSettings, feature.c_str());
}

void TizenWebEngineSettings::SetImeStyle(int style)
{
  wv_settings_ime_style_set(wvSettings, (wv_settings_ime_style_e)style);
}

int TizenWebEngineSettings::GetImeStyle() const
{
  return (int)wv_settings_ime_style_get(wvSettings);
}

void TizenWebEngineSettings::SetDefaultAudioInputDevice(const std::string& deviceId) const
{
  wv_settings_default_audio_input_device_set(wvSettings, deviceId.c_str());
}

void TizenWebEngineSettings::EnableDragAndDrop(bool enable)
{
  wv_settings_drag_drop_enabled_set(wvSettings, enable);
}

bool TizenWebEngineSettings::SetExtraFeatureValue(const std::string& feature, const std::string& value)
{
  // NOT IMPLEMENTED: Chromium has no corresponding setting.
  return false;
}

std::string TizenWebEngineSettings::GetExtraFeatureValue(const std::string& feature) const
{
  // NOT IMPLEMENTED: Chromium has no corresponding setting.
  return {};
}

} // namespace Plugin
} // namespace Dali
