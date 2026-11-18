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

#include "test-dependencies.h"
// Inspect registry size without adding a test accessor to the product API.
#define private public
#include "tizen-web-engine-manager.h"
#undef private
// Keep the inspected class and its implementation in one translation unit.
#include "tizen-web-engine-manager.cpp"
#include <algorithm>
#include <iostream>

using Manager = Dali::Plugin::WebEngineManager;

class View : public Dali::WebEnginePlugin
{
public:
  explicit View(Manager::ProfilePtr profile) : profile(std::move(profile))
  {
  }
  void Destroy() override
  {
    assert(profile->context && profile->wrapper && profile->cookies);
    Test::events.push_back(1);
    profile.reset();
  }
  Manager::ProfilePtr profile;
};

int main()
{
  const auto* api = GetWebEngineProfileApi(1);
  assert(api && api->structSize == sizeof(Dali::WebEngineProfileApi));
  assert(api->createDirectRenderingView);
  assert(!api->createDirectRenderingView(nullptr, nullptr, 10, 10));
  auto& manager = Manager::Get();
  // Native refresh-rate setters affect the whole process, not one context.
  // Exercise production profile creation around an explicit native setting.
  {
    auto first = manager.CreateProfile(false, "");
    assert(first && Test::refreshRate == 60 && Test::refreshRateCalls == 1);
    ewk_context_max_refresh_rate_set(first->context, 30);
    auto second = manager.CreateProfile(false, "");
    auto privateProfile = manager.CreateProfile(true, "");
    auto defaultProfile = manager.GetDefaultProfile();
    assert(second && privateProfile && defaultProfile);
    assert(manager.Initialize());
    assert(Test::refreshRate == 30 && Test::refreshRateCalls == 2);
  }
  const auto initialUnrefs = Test::unrefs;
  // Default-only applications must not retain every expired control block.
  for(int i = 0; i < 1000; ++i)
  {
    auto profile = manager.GetDefaultProfile();
    assert(profile && manager.mProfiles.size() == 1);
    assert(profile == manager.GetDefaultProfile());
    assert(profile.use_count() == 1);
    std::weak_ptr<Manager::Profile> weak = profile;
    profile.reset();
    assert(weak.expired());
    assert(Test::defaultContext.wrappers == 0 && Test::unrefs == initialUnrefs);
  }
  auto retained = manager.CreateProfile(true, "bundle");
  auto* native = retained->context;
  for(int i = 0; i < 1000; ++i)
  {
    auto profile = manager.GetDefaultProfile();
    assert(manager.mProfiles.size() == 2);
    assert(retained->context == native && retained.use_count() == 1);
    assert(native->wrappers == 2);
  }
  // Named registration also prunes expired defaults and named profiles.
  {
    auto profile = manager.CreateProfile(false, "");
    assert(manager.mProfiles.size() == 2);
  }
  auto defaultProfile = manager.GetDefaultProfile();
  for(int i = 0; i < 1000; ++i)
  {
    auto profile = manager.CreateProfile(false, "");
    assert(profile && manager.mProfiles.size() == 3);
    assert(profile.use_count() == 1);
    assert(defaultProfile.use_count() == 1 && Test::defaultContext.wrappers == 2);
  }
  assert(Test::refreshRate == 30 && Test::refreshRateCalls == 2);
  assert(Test::ownedContexts == 1);
  assert(defaultProfile == manager.GetDefaultProfile());
  auto viewProfile = manager.CreateProfile(false, "");
  View view(viewProfile);
  Evas_Object nativeView;
  manager.Add(&nativeView, &view);
  viewProfile.reset();
  Test::events.clear();
  Dali::LifecycleController::Get().Emit();
  assert(!retained->context && !retained->wrapper && !retained->cookies);
  assert(!defaultProfile->context && !defaultProfile->wrapper && !defaultProfile->cookies);
  assert(manager.mProfiles.empty());
  assert((Test::events == std::vector<int>{1, 2, 2, 3}));
  const auto unrefs = Test::unrefs;
  retained.reset();
  defaultProfile.reset();
  Dali::LifecycleController::Get().Emit();
  assert(Test::unrefs == unrefs);
  assert(!manager.GetDefaultProfile());
  std::cout << "profile tracking, weak ownership and termination order passed\n";
}
