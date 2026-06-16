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

TizenWebEngineLoadError::TizenWebEngineLoadError(Ewk_Error* error)
: ewkError(error)
{
}

TizenWebEngineLoadError::~TizenWebEngineLoadError()
{
}

std::string TizenWebEngineLoadError::GetUrl() const
{
  const char* url = ewk_error_url_get(ewkError);
  return url ? std::string(url) : std::string();
}

Dali::WebEngineLoadError::ErrorCode TizenWebEngineLoadError::GetCode() const
{
  int code = ewk_error_code_get(ewkError);
  if(code > (int)Dali::WebEngineLoadError::ErrorCode::INTERNAL_SERVER)
  {
    code = (int)Dali::WebEngineLoadError::ErrorCode::OTHER;
  }
  return static_cast<Dali::WebEngineLoadError::ErrorCode>(code);
}

std::string TizenWebEngineLoadError::GetDescription() const
{
  const char* description = ewk_error_description_get(ewkError);
  return description ? std::string(description) : std::string();
}

Dali::WebEngineLoadError::ErrorType TizenWebEngineLoadError::GetType() const
{
  return static_cast<Dali::WebEngineLoadError::ErrorType>(ewk_error_type_get(ewkError));
}

} // namespace Plugin
} // namespace Dali
