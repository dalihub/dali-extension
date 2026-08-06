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

#include "web-engine-lwe-settings.h"

#include <stdexcept>

namespace Dali
{
namespace Plugin
{

WebEngineLweSettings::WebEngineLweSettings(LWE::WebContainer* webContainer)
: mWebContainer(webContainer),
  mSettings(mWebContainer->GetSettings())
{
}

void WebEngineLweSettings::Apply()
{
  mWebContainer->SetSettings(mSettings);
}

void WebEngineLweSettings::SetBaseBackgroundColor(Dali::Vector4 color)
{
  mSettings.SetBaseBackgroundColor(static_cast<unsigned char>(color.r * 255.0f),
                                   static_cast<unsigned char>(color.g * 255.0f),
                                   static_cast<unsigned char>(color.b * 255.0f),
                                   static_cast<unsigned char>(color.a * 255.0f));
  Apply();
}

void WebEngineLweSettings::AllowMixedContents(bool)
{
}

void WebEngineLweSettings::EnableSpatialNavigation(bool enabled)
{
  mSettings.SetUseSpatialNavigation(enabled);
  Apply();
}

uint32_t WebEngineLweSettings::GetDefaultFontSize() const
{
  return mWebContainer->GetDefaultFontSize();
}

void WebEngineLweSettings::SetDefaultFontSize(uint32_t size)
{
  mWebContainer->SetDefaultFontSize(size);
}

void WebEngineLweSettings::EnableWebSecurity(bool enabled)
{
  mSettings.SetWebSecurityMode(enabled ? LWE::WebSecurityMode::Enable : LWE::WebSecurityMode::Disable);
  Apply();
}

void WebEngineLweSettings::EnableCacheBuilder(bool)
{
}

void WebEngineLweSettings::UseScrollbarThumbFocusNotifications(bool)
{
}

void WebEngineLweSettings::EnableDoNotTrack(bool)
{
}

void WebEngineLweSettings::AllowFileAccessFromExternalUrl(bool)
{
}

bool WebEngineLweSettings::IsJavaScriptEnabled() const
{
  return false;
}

void WebEngineLweSettings::EnableJavaScript(bool)
{
}

bool WebEngineLweSettings::IsAutoFittingEnabled() const
{
  return false;
}

void WebEngineLweSettings::EnableAutoFitting(bool)
{
}

bool WebEngineLweSettings::ArePluginsEnabled() const
{
  return false;
}

void WebEngineLweSettings::EnablePlugins(bool)
{
}

bool WebEngineLweSettings::IsPrivateBrowsingEnabled() const
{
  return false;
}

void WebEngineLweSettings::EnablePrivateBrowsing(bool)
{
}

bool WebEngineLweSettings::IsLinkMagnifierEnabled() const
{
  return false;
}

void WebEngineLweSettings::EnableLinkMagnifier(bool)
{
}

bool WebEngineLweSettings::IsKeypadWithoutUserActionUsed() const
{
  return false;
}

void WebEngineLweSettings::UseKeypadWithoutUserAction(bool)
{
}

bool WebEngineLweSettings::IsAutofillPasswordFormEnabled() const
{
  return false;
}

void WebEngineLweSettings::EnableAutofillPasswordForm(bool)
{
}

bool WebEngineLweSettings::IsFormCandidateDataEnabled() const
{
  return false;
}

void WebEngineLweSettings::EnableFormCandidateData(bool)
{
}

bool WebEngineLweSettings::IsTextSelectionEnabled() const
{
  return false;
}

void WebEngineLweSettings::EnableTextSelection(bool)
{
}

bool WebEngineLweSettings::IsTextAutosizingEnabled() const
{
  return false;
}

void WebEngineLweSettings::EnableTextAutosizing(bool)
{
}

bool WebEngineLweSettings::IsArrowScrollEnabled() const
{
  return false;
}

void WebEngineLweSettings::EnableArrowScroll(bool)
{
}

bool WebEngineLweSettings::IsClipboardEnabled() const
{
  return false;
}

void WebEngineLweSettings::EnableClipboard(bool)
{
}

bool WebEngineLweSettings::IsImePanelEnabled() const
{
  return false;
}

void WebEngineLweSettings::EnableImePanel(bool)
{
}

void WebEngineLweSettings::AllowScriptsOpenWindows(bool)
{
}

bool WebEngineLweSettings::AreImagesLoadedAutomatically() const
{
  return false;
}

void WebEngineLweSettings::AllowImagesLoadAutomatically(bool)
{
}

std::string WebEngineLweSettings::GetDefaultTextEncodingName() const
{
  return {};
}

void WebEngineLweSettings::SetDefaultTextEncodingName(const std::string&)
{
}

bool WebEngineLweSettings::SetViewportMetaTag(bool)
{
  return false;
}

bool WebEngineLweSettings::SetForceZoom(bool)
{
  return false;
}

bool WebEngineLweSettings::IsZoomForced() const
{
  return false;
}

bool WebEngineLweSettings::SetTextZoomEnabled(bool)
{
  return false;
}

bool WebEngineLweSettings::IsTextZoomEnabled() const
{
  return false;
}

void WebEngineLweSettings::SetExtraFeature(const std::string& feature, bool enable)
{
  if(feature == "ttsMode")
  {
    mSettings.SetTTSMode(static_cast<LWE::TTSMode>(enable));
  }
  else if(feature == "useHttp2")
  {
    mSettings.SetUseHttp2(enable);
  }
  else if(feature == "scrollbarVisible")
  {
    mSettings.SetScrollbarVisible(enable);
  }
  else if(feature == "needsDownloadWebFontsEarly")
  {
    mSettings.SetNeedsDownloadWebFontsEarly(enable);
  }
  else if(feature == "useExternalPopup")
  {
    mSettings.SetUseExternalPopup(enable);
  }
  else if(feature == "showLoadFailMsg" || feature == "--show-fps")
  {
    mSettings.UpdateSetting(feature, enable ? "true" : "false");
  }
  else
  {
    mSettings.UpdateSetting(feature, enable ? "True" : "False");
  }
  Apply();
}

bool WebEngineLweSettings::IsExtraFeatureEnabled(const std::string& feature) const
{
  if(feature == "ttsMode")
  {
    return mSettings.GetTTSMode() == LWE::TTSMode::Forced;
  }
  if(feature == "useHttp2")
  {
    return mSettings.UseHttp2();
  }
  if(feature == "scrollbarVisible")
  {
    return mSettings.ScrollbarVisible();
  }
  if(feature == "needsDownloadWebFontsEarly")
  {
    return mSettings.NeedsDownloadWebFontsEarly();
  }
  if(feature == "useExternalPopup")
  {
    return mSettings.UseExternalPopup();
  }

  const std::string value = mSettings.GetSetting(feature);
  return value == "True" || value == "true";
}

bool WebEngineLweSettings::SetExtraFeatureValue(const std::string& feature, const std::string& value)
{
  if(feature == "ttsLanguage")
  {
    mSettings.SetTTSLanguage(value);
    Apply();
    return true;
  }
  if(feature == "imageDownscaleThreshold")
  {
    try
    {
      mSettings.SetNeedsDownScaleImageResourceLargerThan(std::stoul(value));
    }
    catch(const std::exception&)
    {
      return false;
    }
    Apply();
    return true;
  }

  mSettings.UpdateSetting(feature, value);
  Apply();
  return true;
}

std::string WebEngineLweSettings::GetExtraFeatureValue(const std::string& feature) const
{
  if(feature == "ttsLanguage")
  {
    return mSettings.GetTTSLanguage();
  }
  if(feature == "imageDownscaleThreshold")
  {
    return std::to_string(mSettings.NeedsDownScaleImageResourceLargerThan());
  }
  return mSettings.GetSetting(feature);
}

void WebEngineLweSettings::SetImeStyle(int)
{
}

int WebEngineLweSettings::GetImeStyle() const
{
  return 0;
}

void WebEngineLweSettings::SetDefaultAudioInputDevice(const std::string&) const
{
}

void WebEngineLweSettings::EnableDragAndDrop(bool)
{
}

} // namespace Plugin
} // namespace Dali
