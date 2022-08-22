/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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

#include <Elementary.h>

#include <dali/devel-api/adaptor-framework/lifecycle-controller.h>
#include <dali/integration-api/debug.h>

#include <ewk_context.h>
#include <ewk_context_internal.h>
#include <ewk_main.h>

namespace Dali
{
namespace Plugin
{

WebEngineManager& WebEngineManager::Get()
{
  static WebEngineManager instance;
  return instance;
}

WebEngineManager::WebEngineManager()
  : mSlotDelegate(this)
{
  elm_init(0, 0);
  ewk_init();
  mWindow = ecore_evas_new("wayland_egl", 0, 0, 1, 1, 0);

  Ewk_Context* context = ewk_context_default_get();
  mWebEngineContext.reset(new TizenWebEngineContext(context));

  Ewk_Cookie_Manager* manager = ewk_context_cookie_manager_get(context);
  mWebEngineCookieManager.reset(new TizenWebEngineCookieManager(manager));
  Dali::LifecycleController::Get().TerminateSignal().Connect(mSlotDelegate, &WebEngineManager::OnTerminated);
}

Ecore_Evas* WebEngineManager::GetWindow()
{
  return mWindow;
}

Dali::WebEngineContext* WebEngineManager::GetContext()
{
  return mWebEngineContext.get();
}

Dali::WebEngineCookieManager* WebEngineManager::GetCookieManager()
{
  return mWebEngineCookieManager.get();
}

void WebEngineManager::Add(Evas_Object* webView, Dali::WebEnginePlugin* engine)
{
  mWebEngines[webView] = engine;
}

void WebEngineManager::Remove(Evas_Object* webView)
{
  auto iter = mWebEngines.find(webView);
  if (iter != mWebEngines.end())
  {
    mWebEngines.erase(iter);
  }
}

Dali::WebEnginePlugin* WebEngineManager::Find(Evas_Object* webView)
{
  auto iter = mWebEngines.find(webView);
  if (iter != mWebEngines.end())
  {
    return iter->second;
  }
  else
  {
    return nullptr;
  }
}

void WebEngineManager::OnTerminated()
{
  for (auto it = mWebEngines.begin(); it != mWebEngines.end(); it++)
  {
    evas_object_del(it->first);
  }
  mWebEngines.clear();
  ecore_evas_free(mWindow);
  ewk_shutdown();
  elm_shutdown();
  DALI_LOG_RELEASE_INFO("#WebEngineManager is destroyed fully.\n");
}

} // namespace Plugin
} // namespace Dali

extern "C" DALI_EXPORT_API Dali::WebEngineContext* GetWebEngineContext()
{
  return Dali::Plugin::WebEngineManager::Get().GetContext();
}

extern "C" DALI_EXPORT_API Dali::WebEngineCookieManager* GetWebEngineCookieManager()
{
  return Dali::Plugin::WebEngineManager::Get().GetCookieManager();
}