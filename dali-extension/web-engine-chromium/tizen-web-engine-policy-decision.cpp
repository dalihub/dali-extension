/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

#include <ewk_policy_decision_internal.h>

namespace Dali
{
namespace Plugin
{

TizenWebEnginePolicyDecision::TizenWebEnginePolicyDecision(Ewk_Policy_Decision* decision)
  : ewkPolicyDecision(decision)
{
}

TizenWebEnginePolicyDecision::~TizenWebEnginePolicyDecision()
{
}

std::string TizenWebEnginePolicyDecision::GetUrl() const
{
  return ewk_policy_decision_url_get(ewkPolicyDecision);
}

std::string TizenWebEnginePolicyDecision::GetCookie() const
{
  return ewk_policy_decision_cookie_get(ewkPolicyDecision);
}

Dali::WebEnginePolicyDecision::DecisionType TizenWebEnginePolicyDecision::GetDecisionType() const
{
  return (Dali::WebEnginePolicyDecision::DecisionType)ewk_policy_decision_type_get(ewkPolicyDecision);
}

std::string TizenWebEnginePolicyDecision::GetResponseMime() const
{
  return ewk_policy_decision_response_mime_get(ewkPolicyDecision);
}

int32_t TizenWebEnginePolicyDecision::GetResponseStatusCode() const
{
  return ewk_policy_decision_response_status_code_get(ewkPolicyDecision);
}

Dali::WebEnginePolicyDecision::NavigationType TizenWebEnginePolicyDecision::GetNavigationType() const
{
  return (Dali::WebEnginePolicyDecision::NavigationType)ewk_policy_decision_navigation_type_get(ewkPolicyDecision);
}

Dali::WebEngineFrame& TizenWebEnginePolicyDecision::GetFrame() const
{
  Ewk_Frame_Ref ewkFrame = ewk_policy_decision_frame_get(ewkPolicyDecision);
  webEngineFrame.reset(new TizenWebEngineFrame(ewkFrame));
  return *(webEngineFrame.get());
}

std::string TizenWebEnginePolicyDecision::GetScheme() const
{
  return std::string(ewk_policy_decision_scheme_get(ewkPolicyDecision));
}

bool TizenWebEnginePolicyDecision::Use()
{
  return ewk_policy_decision_use(ewkPolicyDecision);
}

bool TizenWebEnginePolicyDecision::Ignore()
{
  return ewk_policy_decision_ignore(ewkPolicyDecision);
}

bool TizenWebEnginePolicyDecision::Suspend()
{
  return ewk_policy_decision_suspend(ewkPolicyDecision);
}

} // namespace Plugin
} // namespace Dali
