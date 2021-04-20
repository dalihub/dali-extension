#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_SECURITY_ORIGIN_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_SECURITY_ORIGIN_H

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
#include <ewk_security_origin.h>
#include <string>

#include <dali/devel-api/adaptor-framework/web-engine-security-origin.h>

namespace Dali
{
namespace Plugin
{

/**
 * @brief A class TizenWebEngineSecurityOrigin for security origin.
 */
class TizenWebEngineSecurityOrigin : public Dali::WebEngineSecurityOrigin
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineSecurityOrigin(Ewk_Security_Origin*);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineSecurityOrigin();

  /**
   * @copydoc Dali::WebEngineSecurityOrigin::GetHost()
   */
  std::string GetHost() const override;

  /**
   * @copydoc Dali::WebEngineSecurityOrigin::GetProtocol()
   */
  std::string GetProtocol() const override;

  /**
   * @brief Get security origin.
   *
   * @return origin pointer.
   */
  Ewk_Security_Origin* GetSecurityOrigin() const;

private:
  Ewk_Security_Origin* ewkSecurityOrigin;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_SECURITY_ORIGIN_H
