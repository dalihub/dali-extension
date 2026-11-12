#pragma once

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

#include <dali/devel-api/adaptor-framework/web-engine/web-engine-profile.h>
#include <dali/public-api/signals/slot-delegate.h>
#include <map>
#include <memory>
#include <vector>
#include <wv_context.h>
#include <wv_view.h>
namespace DALI_NAMESPACE
{
namespace Plugin
{
class WebEngineManager
{
public:
  struct Profile
  {
    Profile(wv_context_h context, bool incognito, bool owned);
    ~Profile();
    void Release();
    wv_context_h context;
    bool incognito;
    bool owned;
    std::unique_ptr<WebEngineContext> wrapper;
    std::unique_ptr<WebEngineCookieManager> cookies;
  };
  using ProfilePtr = std::shared_ptr<Profile>;

  static WebEngineManager& Get();
  static bool IsAvailable();
  bool Initialize();
  ProfilePtr GetDefaultProfile();
  ProfilePtr CreateProfile(bool incognito, const char* bundlePath);
  Dali::WebEngineContext* GetContext(bool incognito = false);
  Dali::WebEngineCookieManager* GetCookieManager(bool incognito = false);
  void Add(wv_view_h view, Dali::WebEnginePlugin* engine);
  void Remove(wv_view_h view);
  Dali::WebEnginePlugin* Find(wv_view_h view);
  wv_view_h Find(Dali::WebEnginePlugin* plugin);

private:
  WebEngineManager();
  ~WebEngineManager();
  void OnTerminated();
  void TrackProfile(const ProfilePtr& profile);
  SlotDelegate<WebEngineManager> mSlotDelegate;
  std::map<wv_view_h, Dali::WebEnginePlugin*> mWebEngines;
  std::vector<std::weak_ptr<Profile>> mProfiles;
  std::weak_ptr<Profile> mDefaultProfile;
  ProfilePtr mLegacyDefaultProfile;
  bool mWebEngineManagerAvailable = false;
};
} // namespace Plugin
} // namespace DALI_NAMESPACE
