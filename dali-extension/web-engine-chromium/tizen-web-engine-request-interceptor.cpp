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

#include "tizen-web-engine-request-interceptor.h"

namespace Dali
{
namespace Plugin
{

TizenWebEngineRequestInterceptor::TizenWebEngineRequestInterceptor(Ewk_Intercept_Request* interceptor)
  : ewkRequestInterceptor(interceptor)
{
}

TizenWebEngineRequestInterceptor::~TizenWebEngineRequestInterceptor()
{
}

std::string TizenWebEngineRequestInterceptor::GetUrl() const
{
  const char* url = ewk_intercept_request_url_get(ewkRequestInterceptor);
  return url ? std::string(url) : std::string();
}

bool TizenWebEngineRequestInterceptor::Ignore()
{
  return ewk_intercept_request_ignore(ewkRequestInterceptor);
}

bool TizenWebEngineRequestInterceptor::SetResponseStatus(int statusCode, const std::string& customStatusText)
{
  return ewk_intercept_request_response_status_set(ewkRequestInterceptor, statusCode, customStatusText.c_str());
}

bool TizenWebEngineRequestInterceptor::AddResponseHeader(const std::string& fieldName, const std::string& fieldValue)
{
  return ewk_intercept_request_response_header_add(ewkRequestInterceptor, fieldName.c_str(), fieldValue.c_str());
}

bool TizenWebEngineRequestInterceptor::AddResponseBody(const std::string& body, uint32_t length)
{
  return ewk_intercept_request_response_body_set(ewkRequestInterceptor, body.c_str(), length);
}

} // namespace Plugin
} // namespace Dali
