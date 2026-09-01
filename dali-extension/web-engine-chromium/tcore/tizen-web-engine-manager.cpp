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

#include <dali/devel-api/adaptor-framework/lifecycle-controller.h>
#include <dali/integration-api/debug.h>

#include <wv_main.h>
#include <wv_main_internal.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

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

  SetArguments(0, nullptr);

  // WV backend: wv_init() owns the whole engine bring-up (including the
  // offscreen canvas that wv_view_create() draws into), so this plugin needs
  // no Ecore_Evas window of its own.
  wv_init();

  Dali::LifecycleController::Get().TerminateSignal().Connect(mSlotDelegate, &WebEngineManager::OnTerminated);

  DALI_LOG_RELEASE_INFO("#WebEngineManager is created fully.\n");
}

void WebEngineManager::SetArguments(int argc, char** argv)
{
  static bool isArgumentsSet = false;
  if(isArgumentsSet)
  {
    return;
  }
  isArgumentsSet = true;

  // On TCORE these two switches are required; on ECORE the engine takes its
  // normal EFL path and neither is passed.
  //
  // --enable-tcore: switches chromium's UI message pump to the GLib backend,
  // which is required because our main loop is tizen_core, not ecore_main_loop.
  //
  // --enable-wv-standalone: makes wv_init() skip elm_init()/ewk_init()/
  // wv_view_init() and route wv_view_create() to the standalone WvViewImpl
  // instead of the EWK offscreen path. The EWK path is blocked on the device
  // tcore chromium build: its GPU thread opens a second tizen-core-wl
  // connection, so virtualized-share eglCreateContext fails with
  // EGL_BAD_CONTEXT (GPU path), and EflSurfaceCanvas::PresentCanvas is a
  // NOTIMPLEMENTED stub (--disable-gpu SW path) - frames never reach
  // "offscreen,frame,rendered" either way.
  //
  // The storage is static because CommandLineEfl keeps the char** we hand it
  // and replays it into content::ContentMainParams later.
  static std::vector<std::string> arguments;
  static std::vector<const char*> argumentVector;

  arguments.assign(argv && argc > 0 ? argv : nullptr, argv && argc > 0 ? argv + argc : nullptr);
  if(arguments.empty())
  {
    arguments.emplace_back("dali-webview");
  }
  arguments.emplace_back("--enable-tcore");
  arguments.emplace_back("--enable-wv-standalone");

  argumentVector.reserve(arguments.size());
  for(auto& argument : arguments)
  {
    argumentVector.push_back(argument.c_str());
  }

  int result = wv_set_arguments(static_cast<int>(argumentVector.size()), argumentVector.data());
  if(result != TIZEN_ERROR_NONE)
  {
    DALI_LOG_ERROR("wv_set_arguments() failed with error: %d\n", result);
  }
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
    catch(std::bad_weak_ptr const& ex)
    {
      DALI_LOG_ERROR("WebEngineManager::~WebEngineManager() - std::bad_weak_ptr caught: %s\n", ex.what());
    }
    catch(std::system_error const& ex)
    {
      DALI_LOG_ERROR("WebEngineManager::~WebEngineManager() - std::system_error caught: %s\n", ex.what());
    }
    catch(std::invalid_argument const& ex)
    {
      DALI_LOG_RELEASE_INFO("Failed to destroy web engine:%s!\n", ex.what());
    }
  }
}

void WebEngineManager::SetContext(wv_context_h context, bool isIncognito)
{
  ContextType contextType     = isIncognito ? ContextType::INCOGNITO : ContextType::NORMAL;
  uint8_t     uintContextType = static_cast<uint8_t>(contextType);
  mWebEngineContexts[uintContextType].reset(new TizenWebEngineContext(context, isIncognito));

  wv_cookie_manager_h manager = wv_context_cookie_manager_get(context);
  mWebEngineCookieManagers[uintContextType].reset(new TizenWebEngineCookieManager(manager));
}

Dali::WebEngineContext* WebEngineManager::GetContext(bool isIncognito)
{
  ContextType contextType     = isIncognito ? ContextType::INCOGNITO : ContextType::NORMAL;
  uint8_t     uintContextType = static_cast<uint8_t>(contextType);
  return mWebEngineContexts[uintContextType].get();
}

Dali::WebEngineCookieManager* WebEngineManager::GetCookieManager(bool isIncognito)
{
  ContextType contextType     = isIncognito ? ContextType::INCOGNITO : ContextType::NORMAL;
  uint8_t     uintContextType = static_cast<uint8_t>(contextType);
  return mWebEngineCookieManagers[uintContextType].get();
}

void WebEngineManager::Add(wv_view_h webView, Dali::WebEnginePlugin* engine, bool isIncognito)
{
  ContextType contextType     = isIncognito ? ContextType::INCOGNITO : ContextType::NORMAL;
  uint8_t     uintContextType = static_cast<uint8_t>(contextType);
  mWebEngines[uintContextType][webView] = engine;
}

void WebEngineManager::Remove(wv_view_h webView, bool isIncognito)
{
  ContextType contextType     = isIncognito ? ContextType::INCOGNITO : ContextType::NORMAL;
  uint8_t     uintContextType = static_cast<uint8_t>(contextType);
  auto&       table           = mWebEngines[uintContextType];
  auto        iter            = table.find(webView);
  if(iter != table.end())
  {
    table.erase(iter);
  }

  // when some web views are in incognito mode, and the last one would be destroyed,
  // callbacks of WV context need be reset here.
  if(isIncognito && mWebEngineContexts[uintContextType] != nullptr && table.size() == 0)
  {
    TizenWebEngineContext* context = static_cast<TizenWebEngineContext*>(mWebEngineContexts[uintContextType].get());
    context->UnregisterContextCallbacks();
  }
}

Dali::WebEnginePlugin* WebEngineManager::Find(wv_view_h webView)
{
  for(uint8_t index = 0; index < ContextTypeCount; index++)
  {
    auto iter = mWebEngines[index].find(webView);
    if(iter != mWebEngines[index].end())
    {
      return iter->second;
    }
  }
  return nullptr;
}

wv_view_h WebEngineManager::Find(Dali::WebEnginePlugin* plugin)
{
  for(uint8_t index = 0; index < ContextTypeCount; index++)
  {
    for(auto it = mWebEngines[index].begin(); it != mWebEngines[index].end(); it++)
    {
      if(it->second == plugin)
      {
        return it->first;
      }
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

  // App is terminated. Now web engine is not available any more.
  mWebEngineManagerAvailable = false;

  for(uint8_t index = 0; index < ContextTypeCount; index++)
  {
    for(auto it = mWebEngines[index].begin(); it != mWebEngines[index].end(); it++)
    {
      // Destroy WebEngine
      auto webEnginePlugin = it->second;
      if(webEnginePlugin)
      {
        webEnginePlugin->Destroy();
      }
    }
    mWebEngines[index].clear();
  }
  mWebEngines = {};

  // Release context and cookie manager before wv_shutdown.
  mWebEngineContexts       = {};
  mWebEngineCookieManagers = {};

  wv_shutdown();
  DALI_LOG_RELEASE_INFO("#WebEngineManager is destroyed fully.\n");
}

} // namespace Plugin
} // namespace Dali

extern "C" DALI_EXPORT_API Dali::WebEngineContext* GetWebEngineContext(bool isIncognito)
{
  return Dali::Plugin::WebEngineManager::Get().GetContext(isIncognito);
}

extern "C" DALI_EXPORT_API Dali::WebEngineCookieManager* GetWebEngineCookieManager(bool isIncognito)
{
  return Dali::Plugin::WebEngineManager::Get().GetCookieManager(isIncognito);
}
