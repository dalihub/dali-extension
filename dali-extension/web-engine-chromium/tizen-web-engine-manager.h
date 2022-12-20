#ifndef DALI_PLUGIN_WEB_ENGINE_MANAGER_H
#define DALI_PLUGIN_WEB_ENGINE_MANAGER_H

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

// EXTERNAL INCLUDES
#include <Ecore_Evas.h>
#include <Evas.h>

#include <dali/devel-api/adaptor-framework/web-engine-plugin.h>
#include <dali/public-api/signals/slot-delegate.h>

#include <map>

namespace Dali
{
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

  void Add(Evas_Object* webView, Dali::WebEnginePlugin* engine);

  void Remove(Evas_Object* webView);

  Dali::WebEnginePlugin* Find(Evas_Object* webView);

  Evas_Object* Find(Dali::WebEnginePlugin* plugin);

private:
  WebEngineManager();

  ~WebEngineManager();

  // FIXME: ewk_shutdown() should be called only when app is terminated.
  //        The singleton instance of WebEngineManager can be destructed before app is terminated.
  //        So it has been fixed that ewk_shutdown() is only called in OnTerminated().
  void OnDestructed();

  void OnTerminated();

  SlotDelegate<WebEngineManager>                 mSlotDelegate;
  Ecore_Evas*                                    mWindow;
  std::map<Evas_Object*, Dali::WebEnginePlugin*> mWebEngines;
  bool                                           mWebEngineManagerAvailable;

  // FIXME: ewk_shutdown() should be called only when app is terminated.
  //        The singleton instance of WebEngineManager can be destructed before app is terminated.
  //        So it has been fixed that ewk_shutdown() is only called in OnTerminated().
  bool                                           initialized = false;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_WEB_ENGINE_MANAGER_H
