#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_LWE_LOAD_ERROR_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_LWE_LOAD_ERROR_H

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

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-load-error.h>

#include <LWEWebView.h>

#include <string>

namespace Dali
{
namespace Plugin
{
/**
 * @brief A class TizenWebEngineLweLoadError for the loading error of LWE(Starfish).
 *
 * Wraps a copy of LWE::ResourceError (url/code/description) and maps its
 * engine-internal error code to the DALi WebEngineLoadError::ErrorCode/
 * ErrorType enums. LWE::ResourceError does not survive past the
 * RegisterOnReceivedErrorHandler callback that produces it, so the values
 * are copied rather than referenced.
 */
class TizenWebEngineLweLoadError : public Dali::WebEngineLoadError
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineLweLoadError(const LWE::ResourceError& error);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineLweLoadError() override;

  /**
   * @copydoc Dali::WebEngineLoadError::GetUrl()
   */
  std::string GetUrl() const override;

  /**
   * @copydoc Dali::WebEngineLoadError::GetCode()
   */
  ErrorCode GetCode() const override;

  /**
   * @copydoc Dali::WebEngineLoadError::GetDescription()
   */
  std::string GetDescription() const override;

  /**
   * @copydoc Dali::WebEngineLoadError::GetType()
   */
  ErrorType GetType() const override;

private:
  std::string mUrl;
  std::string mDescription;
  int         mErrorCode; ///< Raw Starfish::RequestErrorType value (see .cpp mapping table).
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_LWE_LOAD_ERROR_H
