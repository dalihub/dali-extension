#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_LOAD_ERROR_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_LOAD_ERROR_H

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
#include <dali/devel-api/adaptor-framework/web-engine-load-error.h>
#include <ewk_error.h>
#include <ewk_error_internal.h>
#include <string>

namespace Dali
{
namespace Plugin
{

/**
 * @brief A class TizenWebEngineLoadError for loading error.
 */
class TizenWebEngineLoadError : public Dali::WebEngineLoadError
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineLoadError(Ewk_Error*);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineLoadError();

  /**
   * @copydoc Dali::WebEngineLoadError::GetUrl()
   */
  std::string GetUrl() const override;

  /**
   * @copydoc Dali::WebEngineLoadError::GetUrl()
   */
  ErrorCode GetCode() const override;

  /**
   * @copydoc Dali::WebEngineLoadError::GetUrl()
   */
  std::string GetDescription() const override;

  /**
   * @copydoc Dali::WebEngineLoadError::GetUrl()
   */
  ErrorType GetType() const override;

private:
  Ewk_Error* ewkError;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_LOAD_ERROR_H
