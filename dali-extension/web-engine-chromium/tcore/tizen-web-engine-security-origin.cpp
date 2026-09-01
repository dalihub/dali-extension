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

TizenWebEngineSecurityOrigin::TizenWebEngineSecurityOrigin(wv_security_origin_h origin)
: wvSecurityOrigin(origin)
{
}

TizenWebEngineSecurityOrigin::~TizenWebEngineSecurityOrigin()
{
}

std::string TizenWebEngineSecurityOrigin::GetHost() const
{
  const char* host = wv_security_origin_host_get(wvSecurityOrigin);
  return host ? std::string(host) : std::string();
}

std::string TizenWebEngineSecurityOrigin::GetProtocol() const
{
  const char* protocol = wv_security_origin_protocol_get(wvSecurityOrigin);
  return protocol ? std::string(protocol) : std::string();
}

wv_security_origin_h TizenWebEngineSecurityOrigin::GetSecurityOrigin() const
{
  return wvSecurityOrigin;
}

} // namespace Plugin
} // namespace Dali
