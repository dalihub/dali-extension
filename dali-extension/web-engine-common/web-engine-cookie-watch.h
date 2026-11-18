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


#include <cstdint>
#include <cstdlib>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>

namespace DALI_NAMESPACE
{
namespace Plugin
{
/**
 * Native callbacks may outlive unregister. Pass a non-reused opaque token rather
 * than a manager pointer, and retain an executing callable independently.
 * Registration and removal run on the event thread; dispatch may run elsewhere.
 */
class WebEngineCookieWatch
{
public:
  using Callback = std::function<void()>;

  ~WebEngineCookieWatch()
  {
    Set({});
  }

  void Set(Callback callback)
  {
    auto replacement = callback ? std::make_shared<Callback>(std::move(callback)) : nullptr;
    auto& registry = GetRegistry();
    // Release user captures outside the lock: their destructors may re-enter.
    std::shared_ptr<Callback> previous;
    {
      std::lock_guard<std::mutex> lock(registry.mutex);
      if(mToken)
      {
        previous = std::move(registry.callbacks.at(mToken));
        if(replacement)
        {
          registry.callbacks.at(mToken) = std::move(replacement);
        }
        else
        {
          registry.callbacks.erase(mToken);
          mToken = 0;
        }
      }
      else if(replacement)
      {
        if(registry.lastToken == std::numeric_limits<std::uintptr_t>::max())
        {
          std::abort(); // Never reuse a token that a delayed native call may hold.
        }
        const auto token = ++registry.lastToken;
        registry.callbacks.emplace(token, std::move(replacement));
        mToken = token;
      }
    }
  }

  void* Data() const
  {
    return reinterpret_cast<void*>(mToken);
  }

  static void Dispatch(void* data)
  {
    std::shared_ptr<Callback> callback;
    auto& registry = GetRegistry();
    {
      std::lock_guard<std::mutex> lock(registry.mutex);
      auto found = registry.callbacks.find(reinterpret_cast<std::uintptr_t>(data));
      if(found != registry.callbacks.end())
      {
        callback = found->second;
      }
    }
    if(callback)
    {
      (*callback)();
    }
  }

  WebEngineCookieWatch() = default;
  WebEngineCookieWatch(const WebEngineCookieWatch&) = delete;
  WebEngineCookieWatch& operator=(const WebEngineCookieWatch&) = delete;

private:
  struct Registry
  {
    std::mutex mutex;
    std::uintptr_t lastToken{0};
    std::unordered_map<std::uintptr_t, std::shared_ptr<Callback>> callbacks;
  };

  static Registry& GetRegistry()
  {
    // Keep the registry available to profile destructors during static teardown.
    static Registry* registry = new Registry;
    return *registry;
  }

  std::uintptr_t mToken{0};
};
} // namespace Plugin
} // namespace DALI_NAMESPACE
