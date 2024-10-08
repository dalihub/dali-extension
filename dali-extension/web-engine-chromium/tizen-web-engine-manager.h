#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_MANAGER_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_MANAGER_H

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

// EXTERNAL INCLUDES
#include <Ecore_Evas.h>
#include <Evas.h>

#include <dali/devel-api/adaptor-framework/web-engine/web-engine-plugin.h>
#include <dali/public-api/signals/slot-delegate.h>

#include <ewk_context.h>

#include <map>
#include <memory>

namespace Dali
{
class WebEngineContext;
class WebEngineCookieManager;

namespace Plugin
{
/**
 * @brief A class for managing multiple web views
 */
class WebEngineManager
{
public:
  static WebEngineManager& Get();

  static bool IsAvailable();

  WebEngineManager(WebEngineManager const&) = delete;

  void operator=(WebEngineManager const&) = delete;

  Ecore_Evas* GetWindow();

  void SetContext(Ewk_Context* context);

  Dali::WebEngineContext* GetContext();

  Dali::WebEngineCookieManager* GetCookieManager();

  void Add(Evas_Object* webView, Dali::WebEnginePlugin* engine);

  void Remove(Evas_Object* webView);

  Dali::WebEnginePlugin* Find(Evas_Object* o);

  Evas_Object* Find(Dali::WebEnginePlugin* plugin);

private:
  WebEngineManager();

  ~WebEngineManager();

  void OnTerminated();

  SlotDelegate<WebEngineManager>                 mSlotDelegate;
  std::unique_ptr<WebEngineContext>              mWebEngineContext;
  std::unique_ptr<WebEngineCookieManager>        mWebEngineCookieManager;
  Ecore_Evas*                                    mWindow;
  std::map<Evas_Object*, Dali::WebEnginePlugin*> mWebEngines;
  bool                                           mWebEngineManagerAvailable;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_MANAGER_H
