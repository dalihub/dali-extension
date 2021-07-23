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
#include "tizen-web-engine-request-interceptor-task-queue.h"

namespace Dali
{
namespace Plugin
{

TizenWebEngineRequestInterceptor::TizenWebEngineRequestInterceptor(Ewk_Intercept_Request* interceptor)
  : ewkRequestInterceptor(interceptor)
{
  const char* url = ewk_intercept_request_url_get(ewkRequestInterceptor);
  if (url)
  {
    requestUrl = std::string(url);
  }
}

TizenWebEngineRequestInterceptor::~TizenWebEngineRequestInterceptor()
{
}

std::string TizenWebEngineRequestInterceptor::GetUrl() const
{
  return requestUrl;
}

bool TizenWebEngineRequestInterceptor::Ignore()
{
  TaskQueue::GetInstance()->AddTask(std::bind(&TizenWebEngineRequestInterceptor::IgnoreUi, this));
  return true;
}

bool TizenWebEngineRequestInterceptor::SetResponseStatus(int statusCode, const std::string& customStatusText)
{
  TaskQueue::GetInstance()->AddTask(std::bind(&TizenWebEngineRequestInterceptor::SetResponseStatusUi, this, statusCode, customStatusText));
  return true;
}

bool TizenWebEngineRequestInterceptor::AddResponseHeader(const std::string& fieldName, const std::string& fieldValue)
{
  TaskQueue::GetInstance()->AddTask(std::bind(&TizenWebEngineRequestInterceptor::AddResponseHeaderUi, this, fieldName, fieldValue));
  return true;
}

bool TizenWebEngineRequestInterceptor::AddResponseBody(const std::string& body, uint32_t length)
{
  TaskQueue::GetInstance()->AddTask(std::bind(&TizenWebEngineRequestInterceptor::AddResponseBodyUi, this, body, length));
  return true;
}

void TizenWebEngineRequestInterceptor::WaitAndRunTasks()
{
  TaskQueue::GetInstance()->WaitAndRunTasks();
}

void TizenWebEngineRequestInterceptor::NotifyTaskReady()
{
  TaskQueue::GetInstance()->NotifyTaskReady();
}

bool TizenWebEngineRequestInterceptor::IgnoreUi()
{
  return ewk_intercept_request_ignore(ewkRequestInterceptor);
}

bool TizenWebEngineRequestInterceptor::SetResponseStatusUi(int statusCode, const std::string& customStatusText)
{
  return ewk_intercept_request_response_status_set(ewkRequestInterceptor, statusCode, customStatusText.c_str());
}

bool TizenWebEngineRequestInterceptor::AddResponseHeaderUi(const std::string& fieldName, const std::string& fieldValue)
{
  return ewk_intercept_request_response_header_add(ewkRequestInterceptor, fieldName.c_str(), fieldValue.c_str());
}

bool TizenWebEngineRequestInterceptor::AddResponseBodyUi(const std::string& body, uint32_t length)
{
  return ewk_intercept_request_response_body_set(ewkRequestInterceptor, body.c_str(), length);
}

} // namespace Plugin
} // namespace Dali
