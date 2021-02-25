#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_CERTIFICATE_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_CERTIFICATE_H

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

// EXTERNAL INCLUDES
#include <ewk_certificate_internal.h>
#include <string>

#include <dali/devel-api/adaptor-framework/web-engine-certificate.h>

namespace Dali
{
namespace Plugin
{

/**
 * @brief A class TizenWebEngineCertificate for ssl certificate.
 */
class TizenWebEngineCertificate : public Dali::WebEngineCertificate
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineCertificate(Ewk_Certificate_Policy_Decision*);

  /**
   * @brief Constructor.
   */
  TizenWebEngineCertificate(Ewk_Certificate_Info*);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineCertificate();

  /**
   * @copydoc Dali::WebEngineCertificate::Allow()
   */
  void Allow(bool allowed) override;

  /**
   * @copydoc Dali::WebEngineCertificate::IsFromMainFrame()
   */
  bool IsFromMainFrame() const override;

  /**
   * @copydoc Dali::WebEngineCertificate::GetPem()
   */
  std::string GetPem() const override;

  /**
   * @copydoc Dali::WebEngineCertificate::IsContextSecure()
   */
  bool IsContextSecure() const override;

private:
  Ewk_Certificate_Policy_Decision* ewkCertificatePolicyDecision;
  Ewk_Certificate_Info*            ewkCertificateInfo;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_CERTIFICATE_H
