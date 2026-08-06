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

#ifndef DALI_EXTENSION_WEB_ENGINE_LWE_BACKEND_H
#define DALI_EXTENSION_WEB_ENGINE_LWE_BACKEND_H

#include <dali/public-api/adaptor-framework/native-image.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace LWE
{
class WebContainer;
}

namespace Dali
{
namespace Plugin
{
/**
 * Platform boundary for the LWE plugin.
 *
 * LWE navigation and DALi WebEngine semantics live in WebEngineLwe. A backend
 * owns only process initialization, WebContainer creation, rendering resources,
 * and event-thread dispatch required by the host platform.
 */
class WebEngineLweBackend
{
public:
  using Task                  = std::function<void()>;
  using FrameRenderedCallback = std::function<void()>;

  virtual ~WebEngineLweBackend() = default;

  virtual LWE::WebContainer* Create(uint32_t           width,
                                    uint32_t           height,
                                    const std::string& locale,
                                    const std::string& timezoneId) = 0;

  virtual LWE::WebContainer* Create(uint32_t width,
                                    uint32_t height,
                                    uint32_t argc,
                                    char**   argv) = 0;

  virtual void Destroy() = 0;

  virtual void SetSize(uint32_t width, uint32_t height) = 0;

  virtual void UpdateDisplayArea(uint32_t width, uint32_t height) = 0;

  virtual Dali::NativeImagePtr GetNativeImage() = 0;

  virtual void SetFrameRenderedCallback(FrameRenderedCallback callback) = 0;

  /**
   * Execute a task on DALi's event thread. Backends whose LWE callbacks already
   * run there may execute it immediately.
   */
  virtual void DispatchToEventThread(Task task) = 0;
};

/**
 * Implemented by exactly one platform source file in each plugin build.
 */
std::unique_ptr<WebEngineLweBackend> CreateWebEngineLweBackend();

/**
 * Clears cookies when the platform LWE SDK exposes a cookie manager.
 */
void ClearWebEngineLweCookies();

} // namespace Plugin
} // namespace Dali

#endif // DALI_EXTENSION_WEB_ENGINE_LWE_BACKEND_H
