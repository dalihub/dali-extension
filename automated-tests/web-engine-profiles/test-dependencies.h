#pragma once

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

#include <cassert>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <vector>

#define DALI_LOG_ERROR(...) ((void)0)

namespace Test
{
struct Context
{
  int wrappers = 0;
  bool owned = false;
};
inline Context defaultContext;
inline int ownedContexts = 0;
inline int unrefs = 0;
inline bool shutdown = false;
inline std::vector<int> events;
inline Context* CreateContext()
{
  ++ownedContexts;
  return new Context{0, true};
}
inline void Unref(Context* context)
{
  assert(context->owned && context->wrappers == 0 && !shutdown);
  --ownedContexts;
  ++unrefs;
  events.push_back(2);
  delete context;
}
inline void Shutdown()
{
  assert(ownedContexts == 0 && defaultContext.wrappers == 0);
  events.push_back(3);
  shutdown = true;
}
}

using Ewk_Context = Test::Context;
using wv_context_h = Test::Context*;
struct Evas_Object
{
};
using wv_view_h = Evas_Object*;
struct Ecore_Evas
{
};
inline Ecore_Evas* ecore_evas_new(const char*, int, int, int, int, int)
{
  return new Ecore_Evas;
}
inline void ecore_evas_free(Ecore_Evas* window)
{
  delete window;
}
inline int elm_init(int, char**)
{
  return 1;
}
inline void elm_shutdown()
{
}
inline void ewk_set_arguments(int, char**)
{
}
inline int wv_set_arguments(int, const char**)
{
  return 0;
}
inline int ewk_init()
{
  return 1;
}
inline int wv_init()
{
  return 1;
}
inline void ewk_shutdown()
{
  Test::Shutdown();
}
inline void wv_shutdown()
{
  Test::Shutdown();
}
inline Ewk_Context* ewk_context_default_get()
{
  return &Test::defaultContext;
}
inline wv_context_h wv_context_default_get()
{
  return &Test::defaultContext;
}
inline Ewk_Context* ewk_context_new_with_injected_bundle_path(const char*)
{
  return Test::CreateContext();
}
inline Ewk_Context* ewk_context_new_with_injected_bundle_path_in_incognito_mode(const char*)
{
  return Test::CreateContext();
}
inline wv_context_h wv_context_new_with_injected_bundle_path(const char*)
{
  return Test::CreateContext();
}
inline wv_context_h wv_context_new_with_injected_bundle_path_in_incognito_mode(const char*)
{
  return Test::CreateContext();
}
inline void ewk_context_unref(Ewk_Context* context)
{
  Test::Unref(context);
}
inline void wv_context_unref(wv_context_h context)
{
  Test::Unref(context);
}
inline Ewk_Context* ewk_context_cookie_manager_get(Ewk_Context* context)
{
  return context;
}
inline wv_context_h wv_context_cookie_manager_get(wv_context_h context)
{
  return context;
}
inline void ewk_context_max_refresh_rate_set(Ewk_Context*, int)
{
}
inline void wv_context_max_refresh_rate_set(wv_context_h, int)
{
}

namespace Dali
{
class WebEngineContext
{
public:
  virtual ~WebEngineContext() = default;
};
class WebEngineCookieManager
{
public:
  virtual ~WebEngineCookieManager() = default;
};
class WebEnginePlugin
{
public:
  virtual ~WebEnginePlugin() = default;
  virtual void Destroy() = 0;
};
enum class WebEngineArgumentsResult
{
  SUCCESS,
  INVALID_ARGUMENT,
  ALREADY_INITIALIZED,
  BACKEND_UNAVAILABLE
};
struct WebEngineProfileApi
{
  uint32_t version;
  uint32_t structSize;
  WebEngineArgumentsResult (*storeArguments)(uint32_t, const char* const*);
  void (*readArguments)(uint32_t*, const char* const**);
  bool (*initialize)();
  void* (*getDefaultProfile)();
  void* (*createProfile)(bool, const char*);
  void (*destroyProfile)(void*);
  WebEngineContext* (*getContext)(void*);
  WebEngineCookieManager* (*getCookieManager)(void*);
  bool (*createView)(WebEnginePlugin*, void*, uint32_t, uint32_t);
};
template<class T> class SlotDelegate
{
public:
  explicit SlotDelegate(T* object) : object(object)
  {
  }
  T* object;
};
class LifecycleController
{
public:
  static LifecycleController& Get()
  {
    static LifecycleController instance;
    return instance;
  }
  LifecycleController& TerminateSignal()
  {
    return *this;
  }
  template<class T> void Connect(SlotDelegate<T>& slot, void (T::*method)())
  {
    terminate = [object = slot.object, method] { (object->*method)(); };
  }
  void Emit()
  {
    terminate();
  }
  std::function<void()> terminate;
};
namespace Plugin
{
class TizenWebEngineContext : public WebEngineContext
{
public:
  explicit TizenWebEngineContext(Test::Context* context) : context(context)
  {
    ++context->wrappers;
  }
  ~TizenWebEngineContext() override
  {
    --context->wrappers;
  }
  Test::Context* context;
};
class TizenWebEngineCookieManager : public WebEngineCookieManager
{
public:
  explicit TizenWebEngineCookieManager(Test::Context* context) : context(context)
  {
    ++context->wrappers;
  }
  ~TizenWebEngineCookieManager() override
  {
    --context->wrappers;
  }
  Test::Context* context;
};
class TizenWebEngineChromium : public WebEnginePlugin
{
public:
  template<class Profile> bool CreateWithProfile(uint32_t, uint32_t, const Profile&)
  {
    return true;
  }
  void Destroy() override
  {
  }
};
}
}
