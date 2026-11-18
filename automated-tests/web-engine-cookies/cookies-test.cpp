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

#include "tizen-web-engine-cookie-manager.h"
#include <cassert>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

using Manager = Dali::Plugin::TizenWebEngineCookieManager;
auto Capture(const NativeCookies& native)
{
  return [callback = native.callback, data = native.data]
  {
    if(callback)
    {
#ifdef BACKEND_ecore
      callback(data);
#else
      // Model the observed WV first-argument user data without making an
      // incompatible C++ function-pointer call in the test itself.
      callback(data, reinterpret_cast<void*>(0xdeadbeef));
#endif
    }
  };
}

int main()
{
  NativeCookies native;
  auto manager = std::make_unique<Manager>(&native);
  int calls = 0;
  manager->ChangesWatch([&] { ++calls; });
  auto queued = Capture(native);
  queued();
  assert(calls == 1 && native.registrations == 1);
  manager->ChangesWatch([&] { calls += 10; });
  queued();
  assert(calls == 11 && native.registrations == 1);
  manager->ChangesWatch({});
  queued();
  assert(calls == 11 && native.removals == 1);
  manager->ChangesWatch([&] { calls += 100; });
  queued(); // An old token must not target a new registration.
  assert(calls == 11);
  auto current = Capture(native);
  current();
  assert(calls == 111);
  manager.reset();
  current();
  assert(calls == 111 && native.removals == 2);

  // An executing callback retains its captures through concurrent teardown.
  manager = std::make_unique<Manager>(&native);
  std::mutex mutex;
  std::condition_variable condition;
  bool entered = false;
  bool finish = false;
  auto lifetime = std::make_shared<int>(42);
  std::weak_ptr<int> weak = lifetime;
  manager->ChangesWatch([&, lifetime]
  {
    std::unique_lock<std::mutex> lock(mutex);
    entered = true;
    condition.notify_one();
    condition.wait(lock, [&] { return finish; });
    assert(*lifetime == 42);
  });
  lifetime.reset();
  auto inFlight = Capture(native);
  std::thread worker(inFlight);
  {
    std::unique_lock<std::mutex> lock(mutex);
    condition.wait(lock, [&] { return entered; });
  }
  manager.reset();
  assert(!weak.expired());
  {
    std::lock_guard<std::mutex> lock(mutex);
    finish = true;
  }
  condition.notify_one();
  worker.join();
  assert(weak.expired());
  inFlight();

  NativeCookies closing;
  manager = std::make_unique<Manager>(&closing);
  auto onRelease = std::shared_ptr<Manager>(manager.get(), [](Manager* dying) { dying->ChangesWatch([] {}); });
  manager->ChangesWatch([onRelease] {});
  onRelease.reset();
  manager.reset();
  assert(closing.registrations == 1 && closing.removals == 1 && !closing.callback);

  NativeCookies other;
  Manager first(&native);
  Manager second(&other);
  int secondCalls = 0;
  first.ChangesWatch([&] { ++calls; });
  second.ChangesWatch([&] { ++secondCalls; });
#ifndef BACKEND_ecore
  auto staleCallback = native.callback;
  auto staleData = native.data;
#endif
  first.ChangesWatch({});
#ifndef BACKEND_ecore
  // A delayed notification must not dispatch another watch, even if the
  // unused argument happens to contain that watch's live token.
  staleCallback(staleData, other.data);
  assert(secondCalls == 0 && calls == 111);
#endif
  Capture(other)();
  assert(secondCalls == 1 && calls == 111);
}
