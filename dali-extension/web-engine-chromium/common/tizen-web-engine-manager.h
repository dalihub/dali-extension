#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_MANAGER_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_MANAGER_H

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

// EXTERNAL INCLUDES
#include <Ecore_Evas.h>
#include <Evas.h>

#include <dali/devel-api/adaptor-framework/web-engine/web-engine-plugin.h>
#include <dali/public-api/signals/slot-delegate.h>

#include <ewk_context.h>

#include <array>
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

  void SetContext(Ewk_Context* context, bool isIncognito);

  Dali::WebEngineContext* GetContext(bool isIncognito = false);

  Dali::WebEngineCookieManager* GetCookieManager(bool isIncognito = false);

  void Add(Evas_Object* webView, Dali::WebEnginePlugin* engine, bool isIncognito);

  void Remove(Evas_Object* webView, bool isIncognito);

  Dali::WebEnginePlugin* Find(Evas_Object* o);

  Evas_Object* Find(Dali::WebEnginePlugin* plugin);

private:
  WebEngineManager();

  ~WebEngineManager();

  void OnTerminated();

  enum class ContextType
  {
    NORMAL = 0,
    INCOGNITO,
    TYPE_COUNT,
  };

  static constexpr uint8_t ContextTypeCount = static_cast<uint8_t>(ContextType::TYPE_COUNT);

  SlotDelegate<WebEngineManager>                                               mSlotDelegate;
  std::array<std::unique_ptr<WebEngineContext>, ContextTypeCount>              mWebEngineContexts;
  std::array<std::unique_ptr<WebEngineCookieManager>, ContextTypeCount>        mWebEngineCookieManagers;
  std::array<std::map<Evas_Object*, Dali::WebEnginePlugin*>, ContextTypeCount> mWebEngines;
  Ecore_Evas*                                                                  mWindow;
  bool                                                                         mWebEngineManagerAvailable;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_MANAGER_H
