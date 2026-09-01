/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include "tizen-web-engine-manager.h"

#include <wv_intercept_request_internal.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineRequestInterceptor::TizenWebEngineRequestInterceptor(wv_intercept_request_h interceptor)
: wvRequestInterceptor(interceptor)
{
  wvWebView = wv_intercept_request_view_get(wvRequestInterceptor);

  const char* url = wv_intercept_request_url_get(wvRequestInterceptor);
  if(url)
  {
    requestUrl = std::string(url);
  }

  const char* method = wv_intercept_request_http_method_get(wvRequestInterceptor);
  if(method)
  {
    requestMethod = std::string(method);
  }

  GHashTable* hash = wv_intercept_request_headers_get(wvRequestInterceptor);
  if(hash)
  {
    g_hash_table_foreach(hash, &TizenWebEngineRequestInterceptor::IterateRequestHeaders, this);
  }
}

TizenWebEngineRequestInterceptor::~TizenWebEngineRequestInterceptor()
{
}

Dali::WebEnginePlugin* TizenWebEngineRequestInterceptor::GetWebEngine() const
{
  return WebEngineManager::Get().Find(wvWebView);
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
  return wv_intercept_request_ignore(wvRequestInterceptor);
}

bool TizenWebEngineRequestInterceptor::SetResponseStatus(int statusCode, const std::string& customStatusText)
{
  return wv_intercept_request_response_status_set(wvRequestInterceptor, statusCode, customStatusText.c_str());
}

bool TizenWebEngineRequestInterceptor::AddResponseHeader(const std::string& fieldName, const std::string& fieldValue)
{
  return wv_intercept_request_response_header_add(wvRequestInterceptor, fieldName.c_str(), fieldValue.c_str());
}

bool TizenWebEngineRequestInterceptor::AddResponseHeaders(const Dali::Property::Map& headers)
{
  // Owns its keys and values: `key` and `value` below are destroyed at the end
  // of each iteration, so the table has to hold copies rather than pointers
  // into them. g_hash_table_new() would leak those copies on destroy.
  GHashTable*                   headerMap = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
  Dali::Property::Map::SizeType count     = headers.Count();
  for(uint32_t i = 0; i < count; i++)
  {
    Dali::Property::Key key = headers.GetKeyAt(i);
    if(key.type == Dali::Property::Key::STRING)
    {
      String value;
      if(headers.GetValue(i).Get(value))
      {
        g_hash_table_insert(headerMap, g_strdup(key.stringKey.CStr()), g_strdup(value.CStr()));
      }
    }
  }
  bool result = wv_intercept_request_response_header_map_add(wvRequestInterceptor, headerMap);
  g_hash_table_destroy(headerMap);
  return result;
}

bool TizenWebEngineRequestInterceptor::AddResponseBody(const int8_t* body, uint32_t length)
{
  return wv_intercept_request_response_body_set(wvRequestInterceptor, (const char*)body, length);
}

bool TizenWebEngineRequestInterceptor::AddResponse(const std::string& headers, const int8_t* body, uint32_t length)
{
  return wv_intercept_request_response_set(wvRequestInterceptor, headers.c_str(), (const char*)body, length);
}

bool TizenWebEngineRequestInterceptor::WriteResponseChunk(const int8_t* chunk, uint32_t length)
{
  return wv_intercept_request_response_write_chunk(wvRequestInterceptor, (const char*)chunk, length);
}

void TizenWebEngineRequestInterceptor::IterateRequestHeaders(gpointer key, gpointer value, gpointer userData)
{
  TizenWebEngineRequestInterceptor* pThis = static_cast<TizenWebEngineRequestInterceptor*>(userData);
  pThis->requestHeaders.Insert(static_cast<const char*>(key), static_cast<char*>(value));
}

} // namespace Plugin
} // namespace Dali
