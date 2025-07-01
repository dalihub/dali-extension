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

#include <tizen-web-engine-settings.h>

#include <ewk_settings.h>
#include <ewk_settings_product.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineSettings::TizenWebEngineSettings( Ewk_Settings* settings )
  : ewkSettings( settings )
{
}

TizenWebEngineSettings::~TizenWebEngineSettings()
{
}

void TizenWebEngineSettings::AllowMixedContents( bool allowed )
{
  ewk_settings_mixed_contents_set( ewkSettings, allowed );
}

void TizenWebEngineSettings::EnableSpatialNavigation( bool enabled )
{
  ewk_settings_spatial_navigation_enabled_set( ewkSettings, enabled );
}

uint32_t TizenWebEngineSettings::GetDefaultFontSize() const
{
  return ewk_settings_default_font_size_get( ewkSettings );
}

void TizenWebEngineSettings::SetDefaultFontSize( uint32_t size )
{
  ewk_settings_default_font_size_set( ewkSettings, size );
}

void TizenWebEngineSettings::EnableWebSecurity( bool enabled )
{
  ewk_settings_web_security_enabled_set( ewkSettings, enabled );
}

void TizenWebEngineSettings::EnableCacheBuilder( bool enabled )
{
  ewk_settings_cache_builder_enabled_set( ewkSettings, enabled );
}

void TizenWebEngineSettings::UseScrollbarThumbFocusNotifications( bool used )
{
  ewk_settings_uses_scrollbar_thumb_focus_notifications_set( ewkSettings, used );
}

void TizenWebEngineSettings::EnableDoNotTrack( bool enabled )
{
  ewk_settings_do_not_track_set( ewkSettings, enabled );
}

void TizenWebEngineSettings::AllowFileAccessFromExternalUrl( bool allowed )
{
  ewk_settings_allow_file_access_from_external_url_set( ewkSettings, allowed );
}

bool TizenWebEngineSettings::IsJavaScriptEnabled() const
{
  return ewk_settings_javascript_enabled_get( ewkSettings );
}

void TizenWebEngineSettings::EnableJavaScript( bool enabled )
{
  ewk_settings_javascript_enabled_set( ewkSettings, enabled );
}

bool TizenWebEngineSettings::IsAutoFittingEnabled() const
{
  return ewk_settings_auto_fitting_get( ewkSettings );
}

void TizenWebEngineSettings::EnableAutoFitting( bool enabled )
{
  ewk_settings_auto_fitting_set( ewkSettings, enabled );
}

bool TizenWebEngineSettings::ArePluginsEnabled() const
{
  return ewk_settings_plugins_enabled_get( ewkSettings );
}

void TizenWebEngineSettings::EnablePlugins( bool enabled )
{
  ewk_settings_plugins_enabled_set( ewkSettings, enabled );
}

bool TizenWebEngineSettings::IsPrivateBrowsingEnabled() const
{
  return ewk_settings_private_browsing_enabled_get( ewkSettings );
}

void TizenWebEngineSettings::EnablePrivateBrowsing( bool enabled )
{
  ewk_settings_private_browsing_enabled_set( ewkSettings, enabled );
}

bool TizenWebEngineSettings::IsLinkMagnifierEnabled() const
{
  return ewk_settings_link_magnifier_enabled_get( ewkSettings );
}

void TizenWebEngineSettings::EnableLinkMagnifier( bool enabled )
{
  ewk_settings_link_magnifier_enabled_set( ewkSettings, enabled );
}

bool TizenWebEngineSettings::IsKeypadWithoutUserActionUsed() const
{
  return ewk_settings_uses_keypad_without_user_action_get( ewkSettings );
}

void TizenWebEngineSettings::UseKeypadWithoutUserAction( bool used )
{
  ewk_settings_uses_keypad_without_user_action_set( ewkSettings, used );
}

bool TizenWebEngineSettings::IsAutofillPasswordFormEnabled() const
{
  return ewk_settings_autofill_password_form_enabled_get( ewkSettings );
}

void TizenWebEngineSettings::EnableAutofillPasswordForm( bool enabled )
{
  ewk_settings_autofill_password_form_enabled_set( ewkSettings, enabled );
}

bool TizenWebEngineSettings::IsFormCandidateDataEnabled() const
{
  return ewk_settings_form_candidate_data_enabled_get( ewkSettings );
}

void TizenWebEngineSettings::EnableFormCandidateData( bool enabled )
{
  ewk_settings_form_candidate_data_enabled_set( ewkSettings, enabled );
}

bool TizenWebEngineSettings::IsTextSelectionEnabled() const
{
  return ewk_settings_text_selection_enabled_get( ewkSettings );
}

void TizenWebEngineSettings::EnableTextSelection( bool enabled )
{
  ewk_settings_text_selection_enabled_set( ewkSettings, enabled );
}

bool TizenWebEngineSettings::IsTextAutosizingEnabled() const
{
  return ewk_settings_text_autosizing_enabled_get( ewkSettings );
}

void TizenWebEngineSettings::EnableTextAutosizing( bool enabled )
{
  ewk_settings_text_autosizing_enabled_set( ewkSettings, enabled );
}

bool TizenWebEngineSettings::IsArrowScrollEnabled() const
{
  return ewk_settings_uses_arrow_scroll_get( ewkSettings );
}

void TizenWebEngineSettings::EnableArrowScroll( bool enabled)
{
  ewk_settings_uses_arrow_scroll_set( ewkSettings, enabled );
}

bool TizenWebEngineSettings::IsClipboardEnabled() const
{
  return ewk_settings_clipboard_enabled_get( ewkSettings );
}

void TizenWebEngineSettings::EnableClipboard( bool enabled)
{
  ewk_settings_clipboard_enabled_set( ewkSettings, enabled );
}

bool TizenWebEngineSettings::IsImePanelEnabled() const
{
  return ewk_settings_ime_panel_enabled_get( ewkSettings );
}

void TizenWebEngineSettings::EnableImePanel( bool enabled)
{
  ewk_settings_ime_panel_enabled_set( ewkSettings, enabled );
}

void TizenWebEngineSettings::AllowScriptsOpenWindows( bool allowed )
{
  ewk_settings_scripts_window_open_set( ewkSettings, allowed );
}

bool TizenWebEngineSettings::AreImagesLoadedAutomatically() const
{
  return ewk_settings_loads_images_automatically_get( ewkSettings );
}

void TizenWebEngineSettings::AllowImagesLoadAutomatically( bool automatic )
{
  ewk_settings_loads_images_automatically_set( ewkSettings, automatic );
}

std::string TizenWebEngineSettings::GetDefaultTextEncodingName() const
{
  const char* name = ewk_settings_default_text_encoding_name_get( ewkSettings );
  return name ? std::string(name) : std::string();
}

void TizenWebEngineSettings::SetDefaultTextEncodingName( const std::string& defaultTextEncodingName )
{
  ewk_settings_default_text_encoding_name_set( ewkSettings, defaultTextEncodingName.c_str() );
}

bool TizenWebEngineSettings::SetViewportMetaTag(bool enable)
{
  return ewk_settings_viewport_meta_tag_set(ewkSettings, enable);
}

bool TizenWebEngineSettings::SetForceZoom(bool enable)
{
  return ewk_settings_force_zoom_set(ewkSettings, enable);
}

bool TizenWebEngineSettings::IsZoomForced() const
{
  return ewk_settings_force_zoom_get(ewkSettings);
}

bool TizenWebEngineSettings::SetTextZoomEnabled(bool enable)
{
  return ewk_settings_text_zoom_enabled_set(ewkSettings, enable);
}

bool TizenWebEngineSettings::IsTextZoomEnabled() const
{
  return ewk_settings_text_zoom_enabled_get(ewkSettings);
}

void TizenWebEngineSettings::SetExtraFeature(const std::string& feature, bool enable)
{
  ewk_settings_extra_feature_set(ewkSettings, feature.c_str(), enable);
}

bool TizenWebEngineSettings::IsExtraFeatureEnabled(const std::string& feature) const
{
  return ewk_settings_extra_feature_get(ewkSettings, feature.c_str());
}

void TizenWebEngineSettings::SetImeStyle(int style)
{
  ewk_settings_ime_style_set(ewkSettings, (Ewk_Ime_Style)style);
}

int TizenWebEngineSettings::GetImeStyle() const
{
  return (int)ewk_settings_ime_style_get(ewkSettings);
}

void TizenWebEngineSettings::SetDefaultAudioInputDevice(const std::string& deviceId) const
{
  ewk_settings_default_audio_input_device_set(ewkSettings, deviceId.c_str());
}

void TizenWebEngineSettings::EnableDragAndDrop(bool enable)
{
  ewk_settings_drag_drop_enabled_set(ewkSettings, enable);
}

} // namespace Plugin
} // namespace Dali
