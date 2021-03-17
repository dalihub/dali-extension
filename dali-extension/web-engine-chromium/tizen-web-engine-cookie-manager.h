#ifndef DALI_PLUGIN_WEB_ENGINE_COOKIE_MANAGER_H
#define DALI_PLUGIN_WEB_ENGINE_COOKIE_MANAGER_H

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/web-engine-cookie-manager.h>
#include <ewk_cookie_manager.h>
#include <string>

namespace Dali
{
namespace Plugin
{

/**
 * @brief A class TizenWebEngineCookieManager for cookie manager of chromium.
 */
class TizenWebEngineCookieManager : public Dali::WebEngineCookieManager
{

public:

  /**
   * @brief Constructor.
   */
  TizenWebEngineCookieManager(Ewk_Cookie_Manager*);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineCookieManager();

  /**
   * @copydoc Dali::WebEngineCookieManager::SetCookieAcceptPolicy()
   */
  void SetCookieAcceptPolicy(CookieAcceptPolicy policy) override;

  /**
   * @copydoc Dali::WebEngineCookieManager::GetCookieAcceptPolicy()
   */
  CookieAcceptPolicy GetCookieAcceptPolicy() const override;

  /**
   * @copydoc Dali::WebEngineCookieManager::SetPersistentStorage()
   */
  void SetPersistentStorage(const std::string& path, CookiePersistentStorage storage) override;

  /**
   * @copydoc Dali::WebEngineCookieManager::ClearCookies()
   */
  void ClearCookies() override;

  /**
   * @copydoc Dali::WebEngineCookieManager::ChangesWatch()
   */
  void ChangesWatch(Dali::WebEngineCookieManager::WebEngineCookieManagerChangesWatchCallback callback) override;

private:

  /**
   * @brief Callback for changing watch.
   *
   * @param[in] data Data for callback
   */
  static void OnChangesWatch(void *data);

private:
  Ewk_Cookie_Manager*      mEwkCookieManager;
  Ewk_Cookie_Accept_Policy mCookieAcceptancePolicy;
  Dali::WebEngineCookieManager::WebEngineCookieManagerChangesWatchCallback mWebChangesWatchCallback;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_WEB_ENGINE_COOKIE_MANAGER_H
