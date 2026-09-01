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

#include "tizen-web-engine-form-repost-decision.h"

#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineFormRepostDecision::TizenWebEngineFormRepostDecision(void* request)
: wvFormRepostDecision(request)
{
}

TizenWebEngineFormRepostDecision::~TizenWebEngineFormRepostDecision()
{
}

void TizenWebEngineFormRepostDecision::Reply(bool allowed)
{
  // WV GAP (WV_REQUIREMENTS.md A-3): WV exposes no form-repost-decision reply
  // API, so the decision cannot be forwarded to the engine and the repost
  // prompt is left unanswered.

}

} // namespace Plugin
} // namespace Dali