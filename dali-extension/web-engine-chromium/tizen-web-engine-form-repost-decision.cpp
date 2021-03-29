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

#include "tizen-web-engine-form-repost-decision.h"

namespace Dali
{
namespace Plugin
{

TizenWebEngineFormRepostDecision::TizenWebEngineFormRepostDecision(Ewk_Form_Repost_Decision_Request* request)
  : ewkFormRepostDecision(request)
{
}

TizenWebEngineFormRepostDecision::~TizenWebEngineFormRepostDecision()
{
}

void TizenWebEngineFormRepostDecision::Reply(bool allowed)
{
  ewk_form_repost_decision_request_reply(ewkFormRepostDecision, allowed);
}

} // namespace Plugin
} // namespace Dali