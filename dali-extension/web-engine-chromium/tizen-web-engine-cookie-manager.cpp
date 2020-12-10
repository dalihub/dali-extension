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

#include "tizen-web-engine-cookie-manager.h"

#include <ewk_cookie_manager.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineCookieManager::TizenWebEngineCookieManager( Ewk_Cookie_Manager* manager )
  : ewkCookieManager( manager )
{
}

TizenWebEngineCookieManager::~TizenWebEngineCookieManager()
{
}

void TizenWebEngineCookieManager::SetCookieAcceptPolicy( Dali::WebEngineCookieManager::CookieAcceptPolicy policy )
{
  ewk_cookie_manager_accept_policy_set( ewkCookieManager, static_cast< Ewk_Cookie_Accept_Policy >( policy ) );
}

Dali::WebEngineCookieManager::CookieAcceptPolicy TizenWebEngineCookieManager::GetCookieAcceptPolicy() const
{
  //todo...
  return Dali::WebEngineCookieManager::CookieAcceptPolicy::NO_THIRD_PARTY;
}

void TizenWebEngineCookieManager::SetPersistentStorage( const std::string& path, Dali::WebEngineCookieManager::CookiePersistentStorage storage )
{
  ewk_cookie_manager_persistent_storage_set( ewkCookieManager, path.c_str(), static_cast< Ewk_Cookie_Persistent_Storage >( storage ) );
}

void TizenWebEngineCookieManager::ClearCookies()
{
  ewk_cookie_manager_cookies_clear( ewkCookieManager );
}

} // namespace Plugin
} // namespace Dali
