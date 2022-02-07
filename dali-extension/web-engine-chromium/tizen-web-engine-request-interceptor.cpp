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
  const char* url = ewk_intercept_request_url_get(ewkRequestInterceptor);
  if (url)
  {
    requestUrl = std::string(url);
  }

  const char* method = ewk_intercept_request_http_method_get(ewkRequestInterceptor);
  if (method)
  {
    requestMethod = std::string(method);
  }

  const Eina_Hash* hash = ewk_intercept_request_headers_get(ewkRequestInterceptor);
  if (hash)
  {
    eina_hash_foreach(hash, &TizenWebEngineRequestInterceptor::IterateRequestHeaders, this);
  }

  mIsThreadWaiting = true;
}

TizenWebEngineRequestInterceptor::~TizenWebEngineRequestInterceptor()
{
}

std::string TizenWebEngineRequestInterceptor::GetUrl() const
{
  return requestUrl;
}

Dali::Property::Map TizenWebEngineRequestInterceptor::GetHeaders() const
{
  return requestHeaders;
}

std::string TizenWebEngineRequestInterceptor::GetMethod() const
{
  return requestMethod;
}

bool TizenWebEngineRequestInterceptor::Ignore()
{
  std::unique_lock<std::mutex> lock(mMutex);
  mTaskQueue.push_back(std::bind(&TizenWebEngineRequestInterceptor::IgnoreIo, this));
  return true;
}

bool TizenWebEngineRequestInterceptor::IgnoreIo()
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

bool TizenWebEngineRequestInterceptor::AddResponseHeaders(const Dali::Property::Map& headers)
{
  Eina_Hash* headerMap = eina_hash_string_small_new(nullptr);
  Dali::Property::Map::SizeType count = headers.Count();
  for(uint32_t i = 0; i < count; i++)
  {
    Dali::Property::Key key = headers.GetKeyAt(i);
    if(key.type == Dali::Property::Key::STRING)
    {
      std::string value;
      if(headers.GetValue(i).Get(value))
      {
        eina_hash_add(headerMap, key.stringKey.c_str(), value.c_str());
      }
    }
  }
  bool result = ewk_intercept_request_response_header_map_add(ewkRequestInterceptor, headerMap);
  eina_hash_free(headerMap);
  return result;
}

bool TizenWebEngineRequestInterceptor::AddResponseBody(const int8_t* body, uint32_t length)
{
  return ewk_intercept_request_response_body_set(ewkRequestInterceptor, (const char*)body, length);
}

bool TizenWebEngineRequestInterceptor::AddResponse(const std::string& headers, const int8_t* body, uint32_t length)
{
  return ewk_intercept_request_response_set(ewkRequestInterceptor, headers.c_str(), (const char*)body, length);
}

bool TizenWebEngineRequestInterceptor::WriteResponseChunk(const int8_t* chunk, uint32_t length)
{
  return ewk_intercept_request_response_write_chunk(ewkRequestInterceptor, (const char*)chunk, length);
}

void TizenWebEngineRequestInterceptor::WaitAndRunTasks()
{
  // wait for tasks from main thread.
  std::unique_lock<std::mutex> lock(mMutex);
  while(mIsThreadWaiting)
  {
    mCondition.wait(lock);
  }
  mIsThreadWaiting = true;

  // execute tasks on io thread.
  for(std::vector<TaskCallback>::iterator iter = mTaskQueue.begin(); iter != mTaskQueue.end(); iter++)
  {
    (*iter)();
  }
  mTaskQueue.clear();
}

void TizenWebEngineRequestInterceptor::NotifyTaskReady()
{
  std::unique_lock<std::mutex> lock(mMutex);
  mIsThreadWaiting = false;

  // wake up the io thread
  mCondition.notify_all();
}

Eina_Bool TizenWebEngineRequestInterceptor::IterateRequestHeaders(const Eina_Hash*, const void* key, void* data, void* fdata)
{
  TizenWebEngineRequestInterceptor* pThis = static_cast<TizenWebEngineRequestInterceptor*>(fdata);
  pThis->requestHeaders.Insert((const char*)key, (char*)data);
  return true;
}

} // namespace Plugin
} // namespace Dali
