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

#ifndef DALI_EXTENSION_WEB_ENGINE_LWE_LOAD_ERROR_H
#define DALI_EXTENSION_WEB_ENGINE_LWE_LOAD_ERROR_H

#include <dali/devel-api/adaptor-framework/web-engine/web-engine-load-error.h>

#include <string>

namespace Dali
{
namespace Plugin
{
class WebEngineLweLoadError : public Dali::WebEngineLoadError
{
public:
  WebEngineLweLoadError(std::string url, std::string description, int errorCode);
  ~WebEngineLweLoadError() override = default;

  std::string GetUrl() const override;
  ErrorCode GetCode() const override;
  std::string GetDescription() const override;
  ErrorType GetType() const override;

private:
  std::string mUrl;
  std::string mDescription;
  int         mErrorCode;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_EXTENSION_WEB_ENGINE_LWE_LOAD_ERROR_H
