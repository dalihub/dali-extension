#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_FORM_REPOST_DECISION_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_FORM_REPOST_DECISION_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
// WV GAP (WV_REQUIREMENTS.md A-3): WV has no form-repost-decision module, so
// neither <wv_form_repost_decision_product.h> nor wv_form_repost_decision_h
// exists. The handle is carried as an opaque pointer until WV provides them.
#include <string>

#include <dali/devel-api/adaptor-framework/web-engine/web-engine-form-repost-decision.h>

namespace Dali
{
namespace Plugin
{
/**
 * @brief A class TizenWebEngineFormRepostDecision for form repost decision.
 */
class TizenWebEngineFormRepostDecision : public Dali::WebEngineFormRepostDecision
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineFormRepostDecision(void* request);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineFormRepostDecision();

  /**
   * @copydoc Dali::WebEngineFormRepostDecision::Reply()
   */
  void Reply(bool allowed) override;

private:
  void* wvFormRepostDecision;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_FORM_REPOST_DECISION_H
