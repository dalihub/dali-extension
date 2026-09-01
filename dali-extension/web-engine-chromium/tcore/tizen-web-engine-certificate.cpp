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

#include "tizen-web-engine-certificate.h"
#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineCertificate::TizenWebEngineCertificate(wv_certificate_policy_decision_h decision)
: wvCertificatePolicyDecision(decision),
  wvCertificateInfo(0)
{
}

TizenWebEngineCertificate::TizenWebEngineCertificate(wv_certificate_info_h info)
: wvCertificatePolicyDecision(0),
  wvCertificateInfo(info)
{
}

TizenWebEngineCertificate::~TizenWebEngineCertificate()
{
}

void TizenWebEngineCertificate::Allow(bool allowed)
{
  if(wvCertificatePolicyDecision)
  {
    wv_certificate_policy_decision_allowed_set(wvCertificatePolicyDecision, allowed);
  }
}

bool TizenWebEngineCertificate::IsFromMainFrame() const
{
  return wvCertificatePolicyDecision ? wv_certificate_policy_decision_from_main_frame_get(wvCertificatePolicyDecision) : false;
}

std::string TizenWebEngineCertificate::GetPem() const
{
  const char* pem = nullptr;
  if(wvCertificateInfo)
  {
    pem = wv_certificate_info_pem_get(wvCertificateInfo);
  }
  return pem ? std::string(pem) : std::string();
}

bool TizenWebEngineCertificate::IsContextSecure() const
{
  return wvCertificateInfo ? wv_certificate_info_is_context_secure(wvCertificateInfo) : false;
}

int TizenWebEngineCertificate::GetPolicyDecisionError() const
{
  return wvCertificatePolicyDecision ? wv_certificate_policy_decision_error_get(wvCertificatePolicyDecision) : -1;
}

bool TizenWebEngineCertificate::SuspendPolicyDecision() const
{
  return wvCertificatePolicyDecision ? wv_certificate_policy_decision_suspend(wvCertificatePolicyDecision) : false;
}

} // namespace Plugin
} // namespace Dali
