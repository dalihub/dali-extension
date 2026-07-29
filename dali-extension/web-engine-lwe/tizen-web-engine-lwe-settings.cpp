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
  // NOT IMPLEMENTED: LWE::Settings::SetUseSpatialNavigation exists but isn't
  // wired up yet.
}

uint32_t TizenWebEngineLweSettings::GetDefaultFontSize() const
{
  // NOT IMPLEMENTED: WebContainer::GetDefaultFontSize exists but isn't wired
  // up yet.
  return 0;
}

void TizenWebEngineLweSettings::SetDefaultFontSize(uint32_t size)
{
  // NOT IMPLEMENTED: WebContainer::SetDefaultFontSize exists but isn't wired
  // up yet.
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
  // NOT IMPLEMENTED: LWE::Settings::UpdateSetting is a raw, unvalidated
  // key-value map that could back this, but isn't wired up yet.
}

bool TizenWebEngineLweSettings::IsExtraFeatureEnabled(const std::string& feature) const
{
  // NOT IMPLEMENTED: see SetExtraFeature() above.
  return false;
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

} // namespace Plugin
} // namespace Dali
