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

#include "tizen-web-engine-cookie-manager.h"
#include <ewk_cookie_manager_product.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineCookieManager::TizenWebEngineCookieManager(Ewk_Cookie_Manager* manager)
: mEwkCookieManager(manager),
  mCookieAcceptancePolicy(EWK_COOKIE_ACCEPT_POLICY_NO_THIRD_PARTY),
  mWebChangesWatchCallback(nullptr)
{
}

TizenWebEngineCookieManager::~TizenWebEngineCookieManager()
{
  ewk_cookie_manager_changes_watch(mEwkCookieManager, nullptr, nullptr);
}

void TizenWebEngineCookieManager::SetCookieAcceptPolicy(Dali::WebEngineCookieManager::CookieAcceptPolicy policy)
{
  mCookieAcceptancePolicy = static_cast<Ewk_Cookie_Accept_Policy>(policy);
  ewk_cookie_manager_accept_policy_set(mEwkCookieManager, mCookieAcceptancePolicy);
}

Dali::WebEngineCookieManager::CookieAcceptPolicy TizenWebEngineCookieManager::GetCookieAcceptPolicy() const
{
  return static_cast<Dali::WebEngineCookieManager::CookieAcceptPolicy>(mCookieAcceptancePolicy);
}

void TizenWebEngineCookieManager::SetPersistentStorage(const std::string& path, Dali::WebEngineCookieManager::CookiePersistentStorage storage)
{
  ewk_cookie_manager_persistent_storage_set(mEwkCookieManager, path.c_str(), static_cast<Ewk_Cookie_Persistent_Storage>(storage));
}

void TizenWebEngineCookieManager::ClearCookies()
{
  ewk_cookie_manager_cookies_clear(mEwkCookieManager);
}

void TizenWebEngineCookieManager::ChangesWatch(Dali::WebEngineCookieManager::WebEngineCookieManagerChangesWatchCallback callback)
{
  mWebChangesWatchCallback = callback;
  if(mWebChangesWatchCallback)
  {
    ewk_cookie_manager_changes_watch(mEwkCookieManager, &TizenWebEngineCookieManager::OnChangesWatch, this);
  }
  else
  {
    ewk_cookie_manager_changes_watch(mEwkCookieManager, nullptr, nullptr);
  }
}

void TizenWebEngineCookieManager::OnChangesWatch(void* data)
{
  TizenWebEngineCookieManager* pThis = static_cast<TizenWebEngineCookieManager*>(data);
  if(pThis->mWebChangesWatchCallback)
  {
    pThis->mWebChangesWatchCallback();
  }
}

} // namespace Plugin
} // namespace Dali
