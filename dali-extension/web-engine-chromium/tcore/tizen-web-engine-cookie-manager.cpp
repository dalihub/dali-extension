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

namespace DALI_NAMESPACE
{
namespace Plugin
{

TizenWebEngineCookieManager::TizenWebEngineCookieManager(wv_cookie_manager_h manager)
: mWvCookieManager(manager),
  mCookieAcceptancePolicy(WV_COOKIE_ACCEPT_POLICY_NO_THIRD_PARTY)
{
}

TizenWebEngineCookieManager::~TizenWebEngineCookieManager()
{
  mClosing = true;
  ChangesWatch({});
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
  if(mClosing && callback)
  {
    return;
  }
  const bool wasWatching = mChangesWatch.Data() != nullptr;
  mChangesWatch.Set(std::move(callback));
  if(mChangesWatch.Data())
  {
    if(!wasWatching)
    {
      wv_cookie_manager_changes_watch(mWvCookieManager, &TizenWebEngineCookieManager::OnChangesWatch, mChangesWatch.Data());
    }
  }
  else if(wasWatching)
  {
    wv_cookie_manager_changes_watch(mWvCookieManager, nullptr, nullptr);
  }
}

void TizenWebEngineCookieManager::OnChangesWatch(void* data, void*)
{
  // chromium-efl ddd1009452ea: wv_cookie_manager_changes_watch() casts the
  // two-argument WV callback to the legacy one-argument EWK/internal callback.
  // CookieStoreManager::OnCookieChange() passes only registered user data in
  // the FIRST argument; no event info or second argument is supplied.
  // Treat the first argument as user data and never read the second argument.
  // Revisit this workaround if the engine adopts the declared two-arg WV ABI.
  WebEngineCookieWatch::Dispatch(data);
}

} // namespace Plugin
} //namespace DALI_NAMESPACE
