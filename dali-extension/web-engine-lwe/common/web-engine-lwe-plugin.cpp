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

extern "C" DALI_EXPORT_API Dali::WebEnginePlugin* CreateWebEnginePlugin()
{
  return new Dali::Plugin::WebEngineLwe(Dali::Plugin::CreateWebEngineLweBackend());
}

extern "C" DALI_EXPORT_API void DestroyWebEnginePlugin(Dali::WebEnginePlugin* plugin)
{
  if(plugin)
  {
    delete plugin;
  }
}

extern "C" DALI_EXPORT_API Dali::WebEngineContext* GetWebEngineContext(bool /* isIncognito */)
{
  static Dali::Plugin::WebEngineLweContext context;
  return &context;
}

extern "C" DALI_EXPORT_API Dali::WebEngineCookieManager* GetWebEngineCookieManager(bool /* isIncognito */)
{
  static Dali::Plugin::WebEngineLweCookieManager cookieManager;
  return &cookieManager;
}
