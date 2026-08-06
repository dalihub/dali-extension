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

#ifndef DALI_EXTENSION_WEB_ENGINE_LWE_COOKIE_MANAGER_H
#define DALI_EXTENSION_WEB_ENGINE_LWE_COOKIE_MANAGER_H

#include <dali/devel-api/adaptor-framework/web-engine/web-engine-cookie-manager.h>

namespace Dali
{
namespace Plugin
{
class WebEngineLweCookieManager : public Dali::WebEngineCookieManager
{
public:
  void SetCookieAcceptPolicy(CookieAcceptPolicy policy) override;
  CookieAcceptPolicy GetCookieAcceptPolicy() const override;
  void ClearCookies() override;
  void SetPersistentStorage(const std::string& path, CookiePersistentStorage storage) override;
  void ChangesWatch(WebEngineCookieManagerChangesWatchCallback callback) override;

private:
  CookieAcceptPolicy mAcceptPolicy{CookieAcceptPolicy::NO_THIRD_PARTY};
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_EXTENSION_WEB_ENGINE_LWE_COOKIE_MANAGER_H
