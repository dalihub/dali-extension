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

#include "web-engine-lwe-cookie-manager.h"

#include "web-engine-lwe-backend.h"

namespace Dali
{
namespace Plugin
{

void WebEngineLweCookieManager::SetCookieAcceptPolicy(CookieAcceptPolicy policy)
{
  mAcceptPolicy = policy;
}

Dali::WebEngineCookieManager::CookieAcceptPolicy WebEngineLweCookieManager::GetCookieAcceptPolicy() const
{
  return mAcceptPolicy;
}

void WebEngineLweCookieManager::ClearCookies()
{
  ClearWebEngineLweCookies();
}

void WebEngineLweCookieManager::SetPersistentStorage(const std::string&, CookiePersistentStorage)
{
}

void WebEngineLweCookieManager::ChangesWatch(WebEngineCookieManagerChangesWatchCallback)
{
}

} // namespace Plugin
} // namespace Dali
