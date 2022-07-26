#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_REQUEST_INTERCEPTOR_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_REQUEST_INTERCEPTOR_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-request-interceptor.h>

#include <Evas.h>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
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
   * @copydoc Dali::WebEngineRequestInterceptor::GetWebEngine()
   */
  Dali::WebEnginePlugin* GetWebEngine() const override;

  /**
   * @copydoc Dali::WebEngineRequestInterceptor::GetUrl()
   */
  std::string GetUrl() const override;

  /**
   * @copydoc Dali::WebEngineRequestInterceptor::GetHeaders()
   */
  Dali::Property::Map GetHeaders() const override;

  /**
   * @copydoc Dali::WebEngineRequestInterceptor::GetMethod()
   */
  std::string GetMethod() const override;

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
   * @copydoc Dali::WebEngineRequestInterceptor::AddResponseHeaders()
   */
  bool AddResponseHeaders(const Dali::Property::Map& headers) override;

  /**
   * @copydoc Dali::WebEngineRequestInterceptor::AddResponseBody()
   */
  bool AddResponseBody(const int8_t* body, uint32_t length) override;

  /**
   * @copydoc Dali::WebEngineRequestInterceptor::AddResponse()
   */
  bool AddResponse(const std::string& headers, const int8_t* body, uint32_t length) override;

  /**
   * @copydoc Dali::WebEngineRequestInterceptor::WriteResponseChunk()
   */
  bool WriteResponseChunk(const int8_t* chunk, uint32_t length) override;

  /**
   * @brief Wait for and run tasks on io-thread.
   */
  void WaitAndRunTasks();

  /**
   * @brief Notify task ready on main thread.
   */
  void NotifyTaskReady();

private:
  /**
   * @brief Task callback.
   */
  using TaskCallback = std::function<bool(void)>;

  /**
   * @copydoc Dali::WebEngineRequestInterceptor::Ignore()
   * @note It is run on IO thread
   */
  bool IgnoreIo();

  /**
   * @brief Iterator attributes.
   *
   * @param[in] hash Hash map that need be iterated
   * @param[in] key Key of hash map
   * @param[in] data Value of hash map
   * @param[in] fdata User data for iterating hash map
   *
   * @return true if succeeded, false otherwise
   */
  static Eina_Bool IterateRequestHeaders(const Eina_Hash* hash, const void* key, void* data, void* fdata);

private:
  Ewk_Intercept_Request* ewkRequestInterceptor;
  Evas_Object*           ewkWebView;
  std::string            requestUrl;
  std::string            requestMethod;
  Dali::Property::Map    requestHeaders;

  bool                      mIsThreadWaiting;
  std::vector<TaskCallback> mTaskQueue;
  std::mutex                mMutex;
  std::condition_variable   mCondition;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_REQUEST_INTERCEPTOR_H
