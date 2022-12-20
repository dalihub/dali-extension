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

Ecore_Evas* WebEngineManager::GetWindow()
{
  return mWindow;
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

WebEngineManager::WebEngineManager()
: mSlotDelegate(this)
, mWebEngineManagerAvailable(true)
{
  if (!initialized)
  {
    elm_init(0, 0);
    ewk_init();
    initialized = true;
  }

  mWindow = ecore_evas_new( "wayland_egl", 0, 0, 1, 1, 0 );
  LifecycleController::Get().TerminateSignal().Connect(mSlotDelegate, &WebEngineManager::OnTerminated);
}

WebEngineManager::~WebEngineManager()
{
  if(mWebEngineManagerAvailable)
  {
    // Call OnDestructed directly.
    try
    {
      OnDestructed();
    }
    catch(std::invalid_argument const& ex)
    {
      DALI_LOG_RELEASE_INFO("Failed to destroy web engine:%s!\n", ex.what());
    }
  }
}

// FIXME: ewk_shutdown() should be called only when app is terminated.
//        The singleton instance of WebEngineManager can be destructed before app is terminated.
//        So it has been fixed that ewk_shutdown() is only called in OnTerminated().
void WebEngineManager::OnDestructed()
{
  // Ignore duplicated termination
  if(DALI_UNLIKELY(!mWebEngineManagerAvailable))
  {
    return;
  }

  // App is terminated. Now web engine is not available anymore.
  mWebEngineManagerAvailable = false;

  for (auto it = mWebEngines.begin(); it != mWebEngines.end(); it++)
  {
    if (it->second)
    {
      it->second->Destroy();
    }
  }
  mWebEngines.clear();
  ecore_evas_free(mWindow);
  DALI_LOG_RELEASE_INFO("#WebEngineManager is destructed.\n");
}

void WebEngineManager::OnTerminated()
{
  // Ignore duplicated termination
  if(DALI_UNLIKELY(!mWebEngineManagerAvailable))
  {
    return;
  }

  // App is terminated. Now web engine is not available anymore.
  mWebEngineManagerAvailable = false;

  for (auto it = mWebEngines.begin(); it != mWebEngines.end(); it++)
  {
    if (it->second)
    {
      it->second->Destroy();
    }
  }
  mWebEngines.clear();
  ecore_evas_free(mWindow);

  if (initialized)
  {
    ewk_shutdown();
    elm_shutdown();
    initialized = false;
  }

  DALI_LOG_RELEASE_INFO("#WebEngineManager is destroyed fully.\n");
}

} // namespace Plugin
} // namespace Dali
