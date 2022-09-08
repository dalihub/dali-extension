#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_POLICY_DECISION_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_POLICY_DECISION_H

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
#include <ewk_policy_decision.h>
#include <memory>
#include <string>

#include <dali/devel-api/adaptor-framework/web-engine-frame.h>
#include <dali/devel-api/adaptor-framework/web-engine-policy-decision.h>

namespace Dali
{
namespace Plugin
{

/**
 * @brief A class TizenWebEnginePolicyDecision for policy decision of chromium.
 */
class TizenWebEnginePolicyDecision : public Dali::WebEnginePolicyDecision
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEnginePolicyDecision(Ewk_Policy_Decision*);

  /**
   * @brief Destructor.
   */
  ~TizenWebEnginePolicyDecision();

  /**
   * @copydoc Dali::WebEnginePolicyDecision::GetUrl()
   */
  std::string GetUrl() const override;

  /**
   * @copydoc Dali::WebEnginePolicyDecision::GetCookie()
   */
  std::string GetCookie() const override;

  /**
   * @copydoc Dali::WebEnginePolicyDecision::GetDecisionType()
   */
  DecisionType GetDecisionType() const override;

  /**
   * @copydoc Dali::WebEnginePolicyDecision::GetResponseMime()
   */
  std::string GetResponseMime() const override;

  /**
   * @copydoc Dali::WebEnginePolicyDecision::GetResponseStatusCode()
   */
  int32_t GetResponseStatusCode() const override;

  /**
   * @copydoc Dali::WebEnginePolicyDecision::GetNavigationType()
   */
  NavigationType GetNavigationType() const override;

  /**
   * @copydoc Dali::WebEnginePolicyDecision::GetFrame()
   */
  WebEngineFrame& GetFrame() const override;

  /**
   * @copydoc Dali::WebEnginePolicyDecision::GetScheme()
   */
  std::string GetScheme() const override;

  /**
   * @copydoc Dali::WebEnginePolicyDecision::Use()
   */
  bool Use() override;

  /**
   * @copydoc Dali::WebEnginePolicyDecision::Ignore()
   */
  bool Ignore() override;

  /**
   * @copydoc Dali::WebEnginePolicyDecision::Suspend()
   */
  bool Suspend() override;

private:
  Ewk_Policy_Decision*                    ewkPolicyDecision;
  mutable std::unique_ptr<WebEngineFrame> webEngineFrame;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_POLICY_DECISION_H
