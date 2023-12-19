/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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

#include <ewk_main.h>

#include <stdexcept>

namespace Dali
{
namespace Plugin
{

WebEngineManager& WebEngineManager::Get()
{
  static WebEngineManager instance;
  return instance;
}

bool WebEngineManager::IsAvailable()
{
  return Get().mWebEngineManagerAvailable;
}

WebEngineManager::WebEngineManager()
: mSlotDelegate(this),
  mWebEngineManagerAvailable(true)
{
  DALI_LOG_RELEASE_INFO("#WebEngineManager is created.\n");

  elm_init(0, 0);
  ewk_init();
  mWindow = ecore_evas_new("wayland_egl", 0, 0, 1, 1, 0);

  Dali::LifecycleController::Get().TerminateSignal().Connect(mSlotDelegate, &WebEngineManager::OnTerminated);

  DALI_LOG_RELEASE_INFO("#WebEngineManager is created fully.\n");
}

WebEngineManager::~WebEngineManager()
{
  if(mWebEngineManagerAvailable)
  {
    try
    {
      // Call OnTerminated directly.
      OnTerminated();
    }
    catch(std::invalid_argument const& ex)
    {
      DALI_LOG_RELEASE_INFO("Failed to destroy web engine:%s!\n", ex.what());
    }
  }
}

Ecore_Evas* WebEngineManager::GetWindow()
{
  return mWindow;
}

void WebEngineManager::SetContext(Ewk_Context* context)
{
  mWebEngineContext.reset(new TizenWebEngineContext(context));

  Ewk_Cookie_Manager* manager = ewk_context_cookie_manager_get(context);
  mWebEngineCookieManager.reset(new TizenWebEngineCookieManager(manager));
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
  if(iter != mWebEngines.end())
  {
    mWebEngines.erase(iter);
  }
}

Dali::WebEnginePlugin* WebEngineManager::Find(Evas_Object* webView)
{
  auto iter = mWebEngines.find(webView);
  if(iter != mWebEngines.end())
  {
    return iter->second;
  }
  return nullptr;
}

Evas_Object* WebEngineManager::Find(Dali::WebEnginePlugin* plugin)
{
  for(auto it = mWebEngines.begin(); it != mWebEngines.end(); it++)
  {
    if (it->second == plugin)
    {
      return it->first;
    }
  }
  return nullptr;
}

void WebEngineManager::OnTerminated()
{
  // Ignore duplicated termination
  if(DALI_UNLIKELY(!mWebEngineManagerAvailable))
  {
    return;
  }
  DALI_LOG_RELEASE_INFO("#WebEngineManager is destroyed.\n");

  // App is terminated. Now web engine is not available anymore.
  mWebEngineManagerAvailable = false;

  for(auto it = mWebEngines.begin(); it != mWebEngines.end(); it++)
  {
    // Destroy WebEngine
    auto webEnginePlugin = it->second;
    if(webEnginePlugin)
    {
      webEnginePlugin->Destroy();
    }
  }
  mWebEngines.clear();
  ecore_evas_free(mWindow);

  // Release context and cookie manager before ewk_shutdown.
  mWebEngineContext.reset();
  mWebEngineCookieManager.reset();

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
