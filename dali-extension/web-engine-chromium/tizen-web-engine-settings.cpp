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
  return std::string( ewk_settings_default_text_encoding_name_get( ewkSettings ) );
}

void TizenWebEngineSettings::SetDefaultTextEncodingName( const std::string& defaultTextEncodingName )
{
  ewk_settings_default_text_encoding_name_set( ewkSettings, defaultTextEncodingName.c_str() );
}

} // namespace Plugin
} // namespace Dali
