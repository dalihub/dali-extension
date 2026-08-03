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

// CLASS HEADER
#include "tizen-web-engine-lwe-settings.h"

namespace Dali
{
namespace Plugin
{

TizenWebEngineLweSettings::TizenWebEngineLweSettings(LWE::WebContainer* webContainer)
: mWebContainer(webContainer),
  mSettings(mWebContainer->GetSettings())
{
}

TizenWebEngineLweSettings::~TizenWebEngineLweSettings()
{
}

void TizenWebEngineLweSettings::Apply()
{
  mWebContainer->SetSettings(mSettings);
}

void TizenWebEngineLweSettings::SetBaseBackgroundColor(Dali::Vector4 color)
{
  mSettings.SetBaseBackgroundColor(static_cast<unsigned char>(color.r * 255),
                                   static_cast<unsigned char>(color.g * 255),
                                   static_cast<unsigned char>(color.b * 255),
                                   static_cast<unsigned char>(color.a * 255));
  Apply();
}

void TizenWebEngineLweSettings::AllowMixedContents(bool allowed)
{
  // NOT IMPLEMENTED: LWE has no mixed-content policy.
}

void TizenWebEngineLweSettings::EnableSpatialNavigation(bool enabled)
{
  mSettings.SetUseSpatialNavigation(enabled);
  Apply();
}

uint32_t TizenWebEngineLweSettings::GetDefaultFontSize() const
{
  // Default font size is a WebContainer convenience method, not part of the
  // Settings value type, so it's read straight from the engine rather than
  // the cached mSettings snapshot.
  return mWebContainer->GetDefaultFontSize();
}

void TizenWebEngineLweSettings::SetDefaultFontSize(uint32_t size)
{
  // See GetDefaultFontSize() above; clamped to 1..72 by the engine
  // (LWEWebContainerDelegate.cpp).
  mWebContainer->SetDefaultFontSize(size);
}

void TizenWebEngineLweSettings::EnableWebSecurity(bool enabled)
{
  mSettings.SetWebSecurityMode(enabled ? LWE::WebSecurityMode::Enable : LWE::WebSecurityMode::Disable);
  Apply();
}

void TizenWebEngineLweSettings::EnableCacheBuilder(bool enabled)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

void TizenWebEngineLweSettings::UseScrollbarThumbFocusNotifications(bool used)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

void TizenWebEngineLweSettings::EnableDoNotTrack(bool enabled)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

void TizenWebEngineLweSettings::AllowFileAccessFromExternalUrl(bool allowed)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

bool TizenWebEngineLweSettings::IsJavaScriptEnabled() const
{
  // NOT IMPLEMENTED: LWE has no JavaScript on/off toggle; JS always runs.
  return false;
}

void TizenWebEngineLweSettings::EnableJavaScript(bool enabled)
{
  // NOT IMPLEMENTED: LWE has no JavaScript on/off toggle; JS always runs.
}

bool TizenWebEngineLweSettings::IsAutoFittingEnabled() const
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return false;
}

void TizenWebEngineLweSettings::EnableAutoFitting(bool enabled)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

bool TizenWebEngineLweSettings::ArePluginsEnabled() const
{
  // NOT IMPLEMENTED: LWE has no NPAPI/plugin architecture.
  return false;
}

void TizenWebEngineLweSettings::EnablePlugins(bool enabled)
{
  // NOT IMPLEMENTED: LWE has no NPAPI/plugin architecture.
}

bool TizenWebEngineLweSettings::IsPrivateBrowsingEnabled() const
{
  // NOT IMPLEMENTED: LWE has no private-browsing/session-partition concept.
  return false;
}

void TizenWebEngineLweSettings::EnablePrivateBrowsing(bool enabled)
{
  // NOT IMPLEMENTED: LWE has no private-browsing/session-partition concept.
}

bool TizenWebEngineLweSettings::IsLinkMagnifierEnabled() const
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return false;
}

void TizenWebEngineLweSettings::EnableLinkMagnifier(bool enabled)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

bool TizenWebEngineLweSettings::IsKeypadWithoutUserActionUsed() const
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return false;
}

void TizenWebEngineLweSettings::UseKeypadWithoutUserAction(bool used)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

bool TizenWebEngineLweSettings::IsAutofillPasswordFormEnabled() const
{
  // NOT IMPLEMENTED: LWE has no autofill engine.
  return false;
}

void TizenWebEngineLweSettings::EnableAutofillPasswordForm(bool enabled)
{
  // NOT IMPLEMENTED: LWE has no autofill engine.
}

bool TizenWebEngineLweSettings::IsFormCandidateDataEnabled() const
{
  // NOT IMPLEMENTED: LWE has no autofill engine.
  return false;
}

void TizenWebEngineLweSettings::EnableFormCandidateData(bool enabled)
{
  // NOT IMPLEMENTED: LWE has no autofill engine.
}

bool TizenWebEngineLweSettings::IsTextSelectionEnabled() const
{
  // NOT IMPLEMENTED: LWE implements the CSS user-select property internally
  // but doesn't expose a toggle for it through this Settings API yet.
  return false;
}

void TizenWebEngineLweSettings::EnableTextSelection(bool enabled)
{
  // NOT IMPLEMENTED: see IsTextSelectionEnabled() above.
}

bool TizenWebEngineLweSettings::IsTextAutosizingEnabled() const
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return false;
}

void TizenWebEngineLweSettings::EnableTextAutosizing(bool enabled)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

bool TizenWebEngineLweSettings::IsArrowScrollEnabled() const
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return false;
}

void TizenWebEngineLweSettings::EnableArrowScroll(bool enable)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

bool TizenWebEngineLweSettings::IsClipboardEnabled() const
{
  // NOT IMPLEMENTED: LWE has no clipboard support.
  return false;
}

void TizenWebEngineLweSettings::EnableClipboard(bool enabled)
{
  // NOT IMPLEMENTED: LWE has no clipboard support.
}

bool TizenWebEngineLweSettings::IsImePanelEnabled() const
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return false;
}

void TizenWebEngineLweSettings::EnableImePanel(bool enabled)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

void TizenWebEngineLweSettings::AllowScriptsOpenWindows(bool allowed)
{
  // NOT IMPLEMENTED: window.open() itself is unimplemented in LWE.
}

bool TizenWebEngineLweSettings::AreImagesLoadedAutomatically() const
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return false;
}

void TizenWebEngineLweSettings::AllowImagesLoadAutomatically(bool automatic)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

std::string TizenWebEngineLweSettings::GetDefaultTextEncodingName() const
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return std::string();
}

void TizenWebEngineLweSettings::SetDefaultTextEncodingName(const std::string& defaultTextEncodingName)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

bool TizenWebEngineLweSettings::SetViewportMetaTag(bool enable)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return false;
}

bool TizenWebEngineLweSettings::SetForceZoom(bool enable)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return false;
}

bool TizenWebEngineLweSettings::IsZoomForced() const
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return false;
}

bool TizenWebEngineLweSettings::SetTextZoomEnabled(bool enable)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return false;
}

bool TizenWebEngineLweSettings::IsTextZoomEnabled() const
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return false;
}

void TizenWebEngineLweSettings::SetExtraFeature(const std::string& feature, bool enable)
{
  // Prefer LWE::Settings' typed accessors over the raw UpdateSetting() string
  // map where one exists, to avoid string-literal typos/case mismatches.
  // ttsMode's enum values (Default=0/Forced=1) map directly onto bool.
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
    // Debug-only keys read back with a lowercase-only comparison
    // (LWEWebContainerDelegate.cpp), unlike the "True"/"False" convention
    // used elsewhere.
    mSettings.UpdateSetting(feature, enable ? "true" : "false");
  }
  else
  {
    // No typed accessor (e.g. "videoOverlayEnabled") or an LWE key this
    // plugin doesn't otherwise know about: fall back to the raw map so any
    // boolean-valued LWE setting can still be reached through this generic
    // hook, even without a dedicated DALi API for it.
    mSettings.UpdateSetting(feature, enable ? "True" : "False");
  }
  Apply();
}

bool TizenWebEngineLweSettings::IsExtraFeatureEnabled(const std::string& feature) const
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
  // videoOverlayEnabled, showLoadFailMsg/--show-fps, and any other key this
  // plugin doesn't have a typed accessor for: read back through the raw map.
  const std::string value = mSettings.GetSetting(feature);
  return value == "True" || value == "true";
}

void TizenWebEngineLweSettings::SetImeStyle(int style)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

int TizenWebEngineLweSettings::GetImeStyle() const
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return 0;
}

void TizenWebEngineLweSettings::SetDefaultAudioInputDevice(const std::string& deviceId) const
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

void TizenWebEngineLweSettings::EnableDragAndDrop(bool enable)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

bool TizenWebEngineLweSettings::SetExtraFeatureValue(const std::string& feature, const std::string& value)
{
  if(feature == "ttsLanguage")
  {
    // LWE exposes the TTS language through a typed setting rather than the raw string map.
    mSettings.SetTTSLanguage(value);
    mTtsLanguage = value;
    Apply();
    return true;
  }

  const bool updated = mSettings.UpdateSetting(feature, value);
  if(updated)
  {
    Apply();
  }
  return updated;
}

std::string TizenWebEngineLweSettings::GetExtraFeatureValue(const std::string& feature) const
{
  if(feature == "ttsLanguage")
  {
    return mTtsLanguage;
  }
  return mSettings.GetSetting(feature);
}

} // namespace Plugin
} // namespace Dali
