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
#include <string>
#include <dali/devel-api/adaptor-framework/web-engine-cookie-manager.h>

struct Ewk_Cookie_Manager;

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
  TizenWebEngineCookieManager( Ewk_Cookie_Manager* );

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineCookieManager();

  /**
   * @brief Sets @a policy as the cookie acceptance policy for @a manager.
   *
   * @details By default, only cookies set by the main document loaded are
   *          accepted.
   *
   * @param[in] policy A #Dali::WebEngineCookieManager::CookieAcceptPolicy
   */
  void SetCookieAcceptPolicy( CookieAcceptPolicy policy ) override;

  /**
   * @brief Gets the cookie acceptance policy.
   * The default is Dali::WebEngineCookieManager::CookieAcceptPolicy::NO_THIRD_PARTY.
   * @see Dali::WebEngineCookieManager::CookieAcceptPolicy::Type
   */
  CookieAcceptPolicy GetCookieAcceptPolicy() const override;

  /**
   * @brief Sets the @a path where non-session cookies are stored persistently using
   *        @a storage as the format to read/write the cookies.
   *
   * @details Cookies are initially read from @a path/Cookies to create an initial
   *          set of cookies. Then, non-session cookies will be written to @a path/Cookies.
   *          By default, @a manager doesn't store the cookies persistently, so you need to
   *          call this method to keep cookies saved across sessions.
   *          If @a path does not exist it will be created.
   *
   * @param[in] path The path where to read/write Cookies
   * @param[in] storage The type of storage
   */
  void SetPersistentStorage( const std::string& path, CookiePersistentStorage storage ) override;

  /**
   * @brief Deletes all the cookies of @a manager.
   */
  void ClearCookies() override;

private:

  Ewk_Cookie_Manager* ewkCookieManager;

};
} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_WEB_ENGINE_COOKIE_MANAGER_H
