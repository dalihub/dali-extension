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

#include "tizen-web-engine-load-error.h"

namespace Dali
{
namespace Plugin
{

TizenWebEngineLoadError::TizenWebEngineLoadError(wv_error_h error)
: wvError(error)
{
}

TizenWebEngineLoadError::~TizenWebEngineLoadError()
{
}

std::string TizenWebEngineLoadError::GetUrl() const
{
  const char* url = wv_error_url_get(wvError);
  return url ? std::string(url) : std::string();
}

Dali::WebEngineLoadError::ErrorCode TizenWebEngineLoadError::GetCode() const
{
  int code = wv_error_code_get(wvError);
  if(code > (int)Dali::WebEngineLoadError::ErrorCode::INTERNAL_SERVER)
  {
    code = (int)Dali::WebEngineLoadError::ErrorCode::OTHER;
  }
  return static_cast<Dali::WebEngineLoadError::ErrorCode>(code);
}

std::string TizenWebEngineLoadError::GetDescription() const
{
  const char* description = wv_error_description_get(wvError);
  return description ? std::string(description) : std::string();
}

Dali::WebEngineLoadError::ErrorType TizenWebEngineLoadError::GetType() const
{
  return static_cast<Dali::WebEngineLoadError::ErrorType>(wv_error_type_get(wvError));
}

} // namespace Plugin
} // namespace Dali
