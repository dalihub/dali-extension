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

#include "tizen-web-engine-security-origin.h"

namespace Dali
{
namespace Plugin
{

TizenWebEngineSecurityOrigin::TizenWebEngineSecurityOrigin(Ewk_Security_Origin* origin)
: ewkSecurityOrigin(origin)
{
}

TizenWebEngineSecurityOrigin::~TizenWebEngineSecurityOrigin()
{
}

std::string TizenWebEngineSecurityOrigin::GetHost() const
{
  const char* host = ewk_security_origin_host_get(ewkSecurityOrigin);
  return host ? std::string(host) : std::string();
}

std::string TizenWebEngineSecurityOrigin::GetProtocol() const
{
  const char* protocol = ewk_security_origin_protocol_get(ewkSecurityOrigin);
  return protocol ? std::string(protocol) : std::string();
}

Ewk_Security_Origin* TizenWebEngineSecurityOrigin::GetSecurityOrigin() const
{
  return ewkSecurityOrigin;
}

} // namespace Plugin
} // namespace Dali
