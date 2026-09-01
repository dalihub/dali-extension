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

#include <dali/devel-api/adaptor-framework/web-engine/web-engine-plugin.h>
#include <dali/public-api/signals/slot-delegate.h>

#include <wv_context.h>

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

  /**
   * @brief Hands chromium its command line, with the tcore switches appended.
   *
   * base::CommandLine can only be initialised once per process and the first
   * caller wins, so every path that wants to pass arguments must come through
   * here - otherwise an application's own argv would land first and silently
   * drop the tcore switches. Subsequent calls are ignored.
   *
   * @param[in] argc The application's argument count (0 when it has none)
   * @param[in] argv The application's argument vector (may be nullptr)
   */
  static void SetArguments(int argc, char** argv);

  WebEngineManager(WebEngineManager const&) = delete;

  void operator=(WebEngineManager const&) = delete;

  void SetContext(wv_context_h context, bool isIncognito);

  Dali::WebEngineContext* GetContext(bool isIncognito = false);

  Dali::WebEngineCookieManager* GetCookieManager(bool isIncognito = false);

  void Add(wv_view_h webView, Dali::WebEnginePlugin* engine, bool isIncognito);

  void Remove(wv_view_h webView, bool isIncognito);

  Dali::WebEnginePlugin* Find(wv_view_h o);

  wv_view_h Find(Dali::WebEnginePlugin* plugin);

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

  SlotDelegate<WebEngineManager>                                            mSlotDelegate;
  std::array<std::unique_ptr<WebEngineContext>, ContextTypeCount>           mWebEngineContexts;
  std::array<std::unique_ptr<WebEngineCookieManager>, ContextTypeCount>     mWebEngineCookieManagers;
  std::array<std::map<wv_view_h, Dali::WebEnginePlugin*>, ContextTypeCount> mWebEngines;
  bool                                                                      mWebEngineManagerAvailable;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_MANAGER_H
