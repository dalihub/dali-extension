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

#include "tizen-web-engine-certificate.h"
#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineCertificate::TizenWebEngineCertificate(Ewk_Certificate_Policy_Decision* decision)
  : ewkCertificatePolicyDecision(decision)
  , ewkCertificateInfo(0)
{
}

TizenWebEngineCertificate::TizenWebEngineCertificate(Ewk_Certificate_Info* info)
  : ewkCertificatePolicyDecision(0)
  , ewkCertificateInfo(info)
{
}

TizenWebEngineCertificate::~TizenWebEngineCertificate()
{
}

void TizenWebEngineCertificate::Allow(bool allowed)
{
  if (ewkCertificatePolicyDecision)
  {
    ewk_certificate_policy_decision_allowed_set(ewkCertificatePolicyDecision, allowed);
  }
}

bool TizenWebEngineCertificate::IsFromMainFrame() const
{
  return ewkCertificatePolicyDecision ? ewk_certificate_policy_decision_from_main_frame_get(ewkCertificatePolicyDecision) : false;
}

std::string TizenWebEngineCertificate::GetPem() const
{
  const char* pem = nullptr;
  if (ewkCertificateInfo)
  {
    pem = ewk_certificate_info_pem_get(ewkCertificateInfo);
  }
  return pem ? std::string(pem) : std::string();
}

bool TizenWebEngineCertificate::IsContextSecure() const
{
  return ewkCertificateInfo ? ewk_certificate_info_is_context_secure(ewkCertificateInfo) : false;
}

int TizenWebEngineCertificate::GetPolicyDecisionError() const
{
  return ewkCertificatePolicyDecision ? ewk_certificate_policy_decision_error_get(ewkCertificatePolicyDecision) : -1;
}

bool TizenWebEngineCertificate::SuspendPolicyDecision() const
{
  return ewkCertificatePolicyDecision ? ewk_certificate_policy_decision_suspend(ewkCertificatePolicyDecision) : false;
}

} // namespace Plugin
} // namespace Dali
