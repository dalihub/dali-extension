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

#include "tizen-web-engine-policy-decision.h"
#include "tizen-web-engine-frame.h"

#include <dali/integration-api/debug.h>

#include <wv_policy_decision_internal.h>

namespace Dali
{
namespace Plugin
{

TizenWebEnginePolicyDecision::TizenWebEnginePolicyDecision(wv_policy_decision_h decision)
: wvPolicyDecision(decision)
{
}

TizenWebEnginePolicyDecision::~TizenWebEnginePolicyDecision()
{
}

std::string TizenWebEnginePolicyDecision::GetUrl() const
{
  const char* url = wv_policy_decision_url_get(wvPolicyDecision);
  return url ? std::string(url) : std::string();
}

std::string TizenWebEnginePolicyDecision::GetCookie() const
{
  const char* cookie = wv_policy_decision_cookie_get(wvPolicyDecision);
  return cookie ? std::string(cookie) : std::string();
}

Dali::WebEnginePolicyDecision::DecisionType TizenWebEnginePolicyDecision::GetDecisionType() const
{
  return (Dali::WebEnginePolicyDecision::DecisionType)wv_policy_decision_type_get(wvPolicyDecision);
}

std::string TizenWebEnginePolicyDecision::GetResponseMime() const
{
  const char* mime = wv_policy_decision_response_mime_get(wvPolicyDecision);
  return mime ? std::string(mime) : std::string();
}

int32_t TizenWebEnginePolicyDecision::GetResponseStatusCode() const
{
  return wv_policy_decision_response_status_code_get(wvPolicyDecision);
}

Dali::WebEnginePolicyDecision::NavigationType TizenWebEnginePolicyDecision::GetNavigationType() const
{
  return (Dali::WebEnginePolicyDecision::NavigationType)wv_policy_decision_navigation_type_get(wvPolicyDecision);
}

Dali::WebEngineFrame& TizenWebEnginePolicyDecision::GetFrame() const
{
  wv_frame_h wvFrame = wv_policy_decision_frame_get(wvPolicyDecision);
  webEngineFrame.reset(new TizenWebEngineFrame(wvFrame));
  return *(webEngineFrame.get());
}

std::string TizenWebEnginePolicyDecision::GetScheme() const
{
  const char* scheme = wv_policy_decision_scheme_get(wvPolicyDecision);
  return scheme ? std::string(scheme) : std::string();
}

bool TizenWebEnginePolicyDecision::Use()
{
  return wv_policy_decision_use(wvPolicyDecision);
}

bool TizenWebEnginePolicyDecision::Ignore()
{
  return wv_policy_decision_ignore(wvPolicyDecision);
}

bool TizenWebEnginePolicyDecision::Suspend()
{
  return wv_policy_decision_suspend(wvPolicyDecision);
}

} // namespace Plugin
} // namespace Dali
