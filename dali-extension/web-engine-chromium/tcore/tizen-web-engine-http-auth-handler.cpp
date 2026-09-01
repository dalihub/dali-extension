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

#include "tizen-web-engine-http-auth-handler.h"

namespace Dali
{
namespace Plugin
{

TizenWebEngineHttpAuthHandler::TizenWebEngineHttpAuthHandler(wv_auth_challenge_h challenge)
: wvAuthChallenge(challenge)
{
}

TizenWebEngineHttpAuthHandler::~TizenWebEngineHttpAuthHandler()
{
}

std::string TizenWebEngineHttpAuthHandler::GetRealm() const
{
  const char* realm = wv_auth_challenge_realm_get(wvAuthChallenge);
  return realm ? std::string(realm) : std::string();
}

void TizenWebEngineHttpAuthHandler::Suspend()
{
  wv_auth_challenge_suspend(wvAuthChallenge);
}

void TizenWebEngineHttpAuthHandler::UseCredential(const std::string& user, const std::string& password)
{
  wv_auth_challenge_credential_use(wvAuthChallenge, user.c_str(), password.c_str());
}

void TizenWebEngineHttpAuthHandler::CancelCredential()
{
  wv_auth_challenge_credential_cancel(wvAuthChallenge);
}

} // namespace Plugin
} // namespace Dali
