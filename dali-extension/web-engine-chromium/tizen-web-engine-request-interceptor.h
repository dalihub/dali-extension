#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_REQUEST_INTERCEPTOR_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_REQUEST_INTERCEPTOR_H

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
#include <dali/devel-api/adaptor-framework/web-engine-request-interceptor.h>

#include <functional>
#include <string>
#include <vector>

#include <ewk_intercept_request.h>

namespace Dali
{
namespace Plugin
{

/**
 * @brief A class TizenWebEngineRequestInterceptor for intercepting http
 * request.
 */
class TizenWebEngineRequestInterceptor : public Dali::WebEngineRequestInterceptor
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineRequestInterceptor(Ewk_Intercept_Request*);

  /**
   * @brief Destructor.
   */
  virtual ~TizenWebEngineRequestInterceptor() final;

  /**
   * @copydoc Dali::WebEngineRequestInterceptor::GetUrl()
   */
  std::string GetUrl() const override;

  /**
   * @copydoc Dali::WebEngineRequestInterceptor::Ignore()
   */
  bool Ignore() override;

  /**
   * @copydoc Dali::WebEngineRequestInterceptor::SetResponseStatus()
   */
  bool SetResponseStatus(int statusCode, const std::string& customStatusText) override;

  /**
   * @copydoc Dali::WebEngineRequestInterceptor::AddResponseHeader()
   */
  bool AddResponseHeader(const std::string& fieldName, const std::string& fieldValue) override;

  /**
   * @copydoc Dali::WebEngineRequestInterceptor::AddResponseBody()
   */
  bool AddResponseBody(const std::string& body, uint32_t length) override;

  /**
   * @brief Wait for and run tasks on ui-thread.
   */
  void WaitAndRunTasks();

  /**
   * @brief Notify task ready on main thread.
   */
  void NotifyTaskReady();

private:
  /**
   * @copydoc Dali::WebEngineRequestInterceptor::Ignore()
   * @note It is run on ui thread
   */
  bool IgnoreUi();

  /**
   * @copydoc Dali::WebEngineRequestInterceptor::SetResponseStatus()
   * @note It is run on ui thread
   */
  bool SetResponseStatusUi(int statusCode, const std::string& customStatusText);

  /**
   * @copydoc Dali::WebEngineRequestInterceptor::AddResponseHeader()
   * @note It is run on ui thread
   */
  bool AddResponseHeaderUi(const std::string& fieldName, const std::string& fieldValue);

  /**
   * @copydoc Dali::WebEngineRequestInterceptor::AddResponseBody()
   * @note It is run on ui thread
   */
  bool AddResponseBodyUi(const std::string& body, uint32_t length);

private:
  Ewk_Intercept_Request* ewkRequestInterceptor;
  std::string            requestUrl;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_REQUEST_INTERCEPTOR_H
