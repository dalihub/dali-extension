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

#include "tizen-web-engine-manager.h"
#include "tizen-web-engine-context.h"
#include "tizen-web-engine-cookie-manager.h"
#include "tizen-web-engine-chromium.h"
#include "../../web-engine-common/web-engine-arguments.h"
#include <dali/devel-api/adaptor-framework/lifecycle-controller.h>
#include <dali/integration-api/debug.h>
#include <algorithm>
#include <Elementary.h>
#include <ewk_main.h>
#include <ewk_main_internal.h>
#include <ewk_context_internal.h>

namespace DALI_NAMESPACE
{
namespace Plugin
{
namespace
{
WebEngineArguments& Arguments()
{
  static WebEngineArguments arguments;
  return arguments;
}
}

WebEngineManager& WebEngineManager::Get()
{
  static WebEngineManager instance;
  return instance;
}

WebEngineManager::WebEngineManager()
: mSlotDelegate(this)
{
}

WebEngineManager::~WebEngineManager()
{
  OnTerminated();
}

bool WebEngineManager::IsAvailable()
{
  return Get().mWebEngineManagerAvailable;
}

bool WebEngineManager::Initialize()
{
  auto& arguments = Arguments();
  if(arguments.started)
  {
    return mWebEngineManagerAvailable;
  }
  arguments.started = true;
  arguments.PrepareNative(false);
  ewk_set_arguments(arguments.Count(), arguments.Data());
  if(elm_init(0, nullptr) <= 0)
  {
    return false;
  }
  if(ewk_init() <= 0)
  {
    elm_shutdown();
    return false;
  }
  mWindow = ecore_evas_new("wayland_shm", 0, 0, 1, 1, 0);
  if(!mWindow)
  {
    ewk_shutdown();
    elm_shutdown();
    return false;
  }
  mWebEngineManagerAvailable = true;
  arguments.succeeded = true;
  Dali::LifecycleController::Get().TerminateSignal().Connect(mSlotDelegate, &WebEngineManager::OnTerminated);
  return true;
}

WebEngineManager::Profile::Profile(Ewk_Context* nativeContext, bool privateMode, bool ownsContext)
: context(nativeContext), incognito(privateMode), owned(ownsContext)
{
  wrapper.reset(new TizenWebEngineContext(context));
  cookies.reset(new TizenWebEngineCookieManager(ewk_context_cookie_manager_get(context)));
  ewk_context_max_refresh_rate_set(context, 60);
}

WebEngineManager::Profile::~Profile()
{
  Release();
}

void WebEngineManager::Profile::Release()
{
  // Wrappers unregister callbacks before releasing the native context.
  wrapper.reset();
  cookies.reset();
  if(context && owned)
  {
    ewk_context_unref(context);
  }
  context = nullptr;
}

WebEngineManager::ProfilePtr WebEngineManager::GetDefaultProfile()
{
  if(!Initialize())
  {
    return {};
  }
  if(auto profile = mDefaultProfile.lock())
  {
    return profile;
  }
  auto context = ewk_context_default_get();
  if(!context)
  {
    return {};
  }
  auto profile = std::make_shared<Profile>(context, false, false);
  mDefaultProfile = profile;
  TrackProfile(profile);
  return profile;
}

WebEngineManager::ProfilePtr WebEngineManager::CreateProfile(bool incognito, const char* bundlePath)
{
  if(!Initialize())
  {
    return {};
  }
  const char* path = bundlePath ? bundlePath : "";
  auto context = incognito ? ewk_context_new_with_injected_bundle_path_in_incognito_mode(path)
                           : ewk_context_new_with_injected_bundle_path(path);
  if(!context)
  {
    DALI_LOG_ERROR("Native profile creation failed\n");
    return {};
  }
  auto profile = std::make_shared<Profile>(context, incognito, true);
  TrackProfile(profile);
  return profile;
}

void WebEngineManager::TrackProfile(const ProfilePtr& profile)
{
  mProfiles.erase(std::remove_if(mProfiles.begin(), mProfiles.end(), [](const std::weak_ptr<Profile>& item)
  {
    return item.expired();
  }), mProfiles.end());
  mProfiles.emplace_back(profile);
}

Dali::WebEngineContext* WebEngineManager::GetContext(bool incognito)
{
  if(incognito || !Initialize())
  {
    return nullptr;
  }
  mLegacyDefaultProfile = GetDefaultProfile();
  return mLegacyDefaultProfile ? mLegacyDefaultProfile->wrapper.get() : nullptr;
}

Dali::WebEngineCookieManager* WebEngineManager::GetCookieManager(bool incognito)
{
  if(!GetContext(incognito))
  {
    return nullptr;
  }
  return mLegacyDefaultProfile->cookies.get();
}

void WebEngineManager::Add(Evas_Object* view, Dali::WebEnginePlugin* engine)
{
  mWebEngines[view] = engine;
}

void WebEngineManager::Remove(Evas_Object* view)
{
  mWebEngines.erase(view);
}

Dali::WebEnginePlugin* WebEngineManager::Find(Evas_Object* view)
{
  auto found = mWebEngines.find(view);
  return found == mWebEngines.end() ? nullptr : found->second;
}

Evas_Object* WebEngineManager::Find(Dali::WebEnginePlugin* plugin)
{
  for(const auto& entry : mWebEngines)
  {
    if(entry.second == plugin)
    {
      return entry.first;
    }
  }
  return nullptr;
}

Ecore_Evas* WebEngineManager::GetWindow()
{
  return mWindow;
}

void WebEngineManager::OnTerminated()
{
  if(!mWebEngineManagerAvailable)
  {
    return;
  }
  mWebEngineManagerAvailable = false;
  // Destruction callbacks may also remove another view. Always consume the
  // current registry rather than keeping raw pointers in a snapshot.
  while(!mWebEngines.empty())
  {
    auto* plugin = mWebEngines.begin()->second;
    mWebEngines.erase(mWebEngines.begin());
    plugin->Destroy();
  }
  for(auto& weak : mProfiles)
  {
    if(auto profile = weak.lock())
    {
      profile->Release();
    }
  }
  mProfiles.clear();
  mLegacyDefaultProfile.reset();
  ecore_evas_free(mWindow);
  mWindow = nullptr;
  ewk_shutdown();
  elm_shutdown();
}

namespace
{
using ProfilePtr = WebEngineManager::ProfilePtr;
WebEngineArgumentsResult StoreArguments(uint32_t argc, const char* const* argv)
{
  return Arguments().Store(argc, argv);
}
void ReadArguments(uint32_t* argc, const char* const** argv)
{
  Arguments().Read(argc, argv);
}
bool InitializeEngine()
{
  return WebEngineManager::Get().Initialize();
}
void* DefaultProfile()
{
  auto profile = WebEngineManager::Get().GetDefaultProfile();
  return profile ? new ProfilePtr(std::move(profile)) : nullptr;
}
void* CreateProfile(bool incognito, const char* path)
{
  auto profile = WebEngineManager::Get().CreateProfile(incognito, path);
  return profile ? new ProfilePtr(std::move(profile)) : nullptr;
}
void DestroyProfile(void* token)
{
  delete static_cast<ProfilePtr*>(token);
}
WebEngineContext* ProfileContext(void* token)
{
  return (*static_cast<ProfilePtr*>(token))->wrapper.get();
}
WebEngineCookieManager* ProfileCookies(void* token)
{
  return (*static_cast<ProfilePtr*>(token))->cookies.get();
}
bool CreateView(WebEnginePlugin* plugin, void* token, uint32_t width, uint32_t height)
{
  return static_cast<TizenWebEngineChromium*>(plugin)->CreateWithProfile(width, height, *static_cast<ProfilePtr*>(token));
}
WebEngineDirectRenderingSessionPtr CreateDirectRenderingView(WebEnginePlugin*, void*, uint32_t, uint32_t)
{
  // Not implemented: the native backend has no profile-aware external-target rendering connection.
  return {};
}
const WebEngineProfileApi profileApi{1, sizeof(WebEngineProfileApi), StoreArguments, ReadArguments, InitializeEngine,
  DefaultProfile, CreateProfile, DestroyProfile, ProfileContext, ProfileCookies, CreateView, CreateDirectRenderingView};
}
} // namespace Plugin
} // namespace DALI_NAMESPACE

extern "C" DALI_EXPORT_API const Dali::WebEngineProfileApi* GetWebEngineProfileApi(uint32_t version)
{
  return version == 1 ? &Dali::Plugin::profileApi : nullptr;
}

extern "C" DALI_EXPORT_API Dali::WebEngineContext* GetWebEngineContext(bool incognito)
{
  return Dali::Plugin::WebEngineManager::Get().GetContext(incognito);
}

extern "C" DALI_EXPORT_API Dali::WebEngineCookieManager* GetWebEngineCookieManager(bool incognito)
{
  return Dali::Plugin::WebEngineManager::Get().GetCookieManager(incognito);
}
