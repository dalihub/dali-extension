#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_HTTP_AUTH_HANDLER_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_HTTP_AUTH_HANDLER_H

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
#include <ewk_auth_challenge_internal.h>
#include <string>

#include <dali/devel-api/adaptor-framework/web-engine-http-auth-handler.h>

namespace Dali
{
namespace Plugin
{
/**
 * @brief A class TizenWebEngineHttpAuthHandler for http authentication.
 */
class TizenWebEngineHttpAuthHandler : public Dali::WebEngineHttpAuthHandler
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineHttpAuthHandler(Ewk_Auth_Challenge*);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineHttpAuthHandler();

  /**
   * @copydoc Dali::WebEngineHttpAuthHandler::GetRealm()
   */
  std::string GetRealm() const override;

  /**
   * @copydoc Dali::WebEngineHttpAuthHandler::Suspend()
   */
  void Suspend() override;

  /**
   * @copydoc Dali::WebEngineHttpAuthHandler::UseCredential()
   */
  void UseCredential(const std::string& user, const std::string& password) override;

  /**
   * @copydoc Dali::WebEngineHttpAuthHandler::CancelCredential()
   */
  void CancelCredential() override;

private:
  Ewk_Auth_Challenge* ewkAuthChallenge;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_HTTP_AUTH_HANDLER_H
