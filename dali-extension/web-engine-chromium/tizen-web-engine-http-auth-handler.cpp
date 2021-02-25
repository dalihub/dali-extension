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

#include "tizen-web-engine-http-auth-handler.h"

namespace Dali
{
namespace Plugin
{

TizenWebEngineHttpAuthHandler::TizenWebEngineHttpAuthHandler(Ewk_Auth_Challenge* challenge)
  : ewkAuthChallenge(challenge)
{
}

TizenWebEngineHttpAuthHandler::~TizenWebEngineHttpAuthHandler()
{
}

std::string TizenWebEngineHttpAuthHandler::GetRealm() const
{
  return std::string(ewk_auth_challenge_realm_get(ewkAuthChallenge));
}

void TizenWebEngineHttpAuthHandler::Suspend()
{
  ewk_auth_challenge_suspend(ewkAuthChallenge);
}

void TizenWebEngineHttpAuthHandler::UseCredential(const std::string& user, const std::string& password)
{
  ewk_auth_challenge_credential_use(ewkAuthChallenge, user.c_str(), password.c_str());
}

void TizenWebEngineHttpAuthHandler::CancelCredential()
{
  ewk_auth_challenge_credential_cancel(ewkAuthChallenge);
}

} // namespace Plugin
} // namespace Dali
