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

#include "tizen-web-engine-cookie-manager.h"
#include <wv_cookie_manager_product.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineCookieManager::TizenWebEngineCookieManager(wv_cookie_manager_h manager)
: mWvCookieManager(manager),
  mCookieAcceptancePolicy(WV_COOKIE_ACCEPT_POLICY_NO_THIRD_PARTY),
  mWebChangesWatchCallback(nullptr)
{
}

TizenWebEngineCookieManager::~TizenWebEngineCookieManager()
{
  wv_cookie_manager_changes_watch(mWvCookieManager, nullptr, nullptr);
}

void TizenWebEngineCookieManager::SetCookieAcceptPolicy(Dali::WebEngineCookieManager::CookieAcceptPolicy policy)
{
  mCookieAcceptancePolicy = static_cast<wv_cookie_accept_policy_e>(policy);
  wv_cookie_manager_accept_policy_set(mWvCookieManager, mCookieAcceptancePolicy);
}

Dali::WebEngineCookieManager::CookieAcceptPolicy TizenWebEngineCookieManager::GetCookieAcceptPolicy() const
{
  return static_cast<Dali::WebEngineCookieManager::CookieAcceptPolicy>(mCookieAcceptancePolicy);
}

void TizenWebEngineCookieManager::SetPersistentStorage(const std::string& path, Dali::WebEngineCookieManager::CookiePersistentStorage storage)
{
  wv_cookie_manager_persistent_storage_set(mWvCookieManager, path.c_str(), static_cast<wv_cookie_manager_persistent_storage_e>(storage));
}

void TizenWebEngineCookieManager::ClearCookies()
{
  wv_cookie_manager_cookies_clear(mWvCookieManager);
}

void TizenWebEngineCookieManager::ChangesWatch(Dali::WebEngineCookieManager::WebEngineCookieManagerChangesWatchCallback callback)
{
  mWebChangesWatchCallback = callback;
  if(mWebChangesWatchCallback)
  {
    wv_cookie_manager_changes_watch(mWvCookieManager, &TizenWebEngineCookieManager::OnChangesWatch, this);
  }
  else
  {
    wv_cookie_manager_changes_watch(mWvCookieManager, nullptr, nullptr);
  }
}

void TizenWebEngineCookieManager::OnChangesWatch(void*, void* data)
{
  TizenWebEngineCookieManager* pThis = static_cast<TizenWebEngineCookieManager*>(data);
  if(pThis->mWebChangesWatchCallback)
  {
    pThis->mWebChangesWatchCallback();
  }
}

} // namespace Plugin
} // namespace Dali
