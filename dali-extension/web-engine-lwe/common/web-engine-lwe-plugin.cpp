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

#include "web-engine-lwe-backend.h"
#include "web-engine-lwe-context.h"
#include "web-engine-lwe-cookie-manager.h"
#include "web-engine-lwe.h"

#include "../../web-engine-common/web-engine-arguments.h"
#include <dali/devel-api/adaptor-framework/lifecycle-controller.h>
#include <dali/public-api/signals/slot-delegate.h>
#include <dali/integration-api/debug.h>

namespace DALI_NAMESPACE
{
namespace Plugin
{
namespace
{
class LweRuntime
{
public:
  LweRuntime() : mDelegate(this)
  {
  }
  WebEngineArguments arguments;
  WebEngineLweContext context;
  WebEngineLweCookieManager cookies;
  bool available = false;
  SlotDelegate<LweRuntime> mDelegate;

  void Terminate()
  {
    available = false;
    WebEngineLwe::DestroyAllViews();
  }
};
LweRuntime& Runtime()
{
  static LweRuntime runtime;
  return runtime;
}
WebEngineArgumentsResult StoreArguments(uint32_t argc, const char* const* argv)
{
  return Runtime().arguments.Store(argc, argv);
}
void ReadArguments(uint32_t* argc, const char* const** argv)
{
  Runtime().arguments.Read(argc, argv);
}
void* DefaultProfile()
{
  return InitializeWebEngineLwe() ? &Runtime() : nullptr;
}
void* CreateProfile(bool, const char*)
{
  DALI_LOG_ERROR("LWE does not support independent named contexts, incognito or injected bundles\n");
  return nullptr;
}
void DestroyProfile(void*)
{
}
WebEngineContext* ProfileContext(void*)
{
  return Runtime().available ? &Runtime().context : nullptr;
}
WebEngineCookieManager* ProfileCookies(void*)
{
  return Runtime().available ? &Runtime().cookies : nullptr;
}
bool CreateView(WebEnginePlugin* plugin, void* profile, uint32_t width, uint32_t height)
{
  return profile == &Runtime() && Runtime().available && static_cast<WebEngineLwe*>(plugin)->CreateDefaultView(width, height);
}
const WebEngineProfileApi profileApi{1, sizeof(WebEngineProfileApi), StoreArguments, ReadArguments, InitializeWebEngineLwe,
  DefaultProfile, CreateProfile, DestroyProfile, ProfileContext, ProfileCookies, CreateView};
}

bool InitializeWebEngineLwe()
{
  auto& runtime = Runtime();
  if(runtime.arguments.started)
  {
    return runtime.available;
  }
  runtime.arguments.started = true;
  runtime.arguments.PrepareNative(false);
  runtime.available = InitializeWebEngineLweBackend(runtime.arguments.Count(), runtime.arguments.Data());
  if(runtime.available)
  {
    Dali::LifecycleController::Get().TerminateSignal().Connect(runtime.mDelegate, &LweRuntime::Terminate);
  }
  return runtime.available;
}
} // namespace Plugin
} // namespace DALI_NAMESPACE

extern "C" DALI_EXPORT_API Dali::WebEnginePlugin* CreateWebEnginePlugin()
{
  return new Dali::Plugin::WebEngineLwe(Dali::Plugin::CreateWebEngineLweBackend());
}

extern "C" DALI_EXPORT_API void DestroyWebEnginePlugin(Dali::WebEnginePlugin* plugin)
{
  delete plugin;
}

extern "C" DALI_EXPORT_API Dali::WebEngineContext* GetWebEngineContext(bool incognito)
{
  return !incognito && Dali::Plugin::InitializeWebEngineLwe() ? Dali::Plugin::ProfileContext(nullptr) : nullptr;
}

extern "C" DALI_EXPORT_API Dali::WebEngineCookieManager* GetWebEngineCookieManager(bool incognito)
{
  return !incognito && Dali::Plugin::InitializeWebEngineLwe() ? Dali::Plugin::ProfileCookies(nullptr) : nullptr;
}

extern "C" DALI_EXPORT_API const Dali::WebEngineProfileApi* GetWebEngineProfileApi(uint32_t version)
{
  return version == 1 ? &Dali::Plugin::profileApi : nullptr;
}
