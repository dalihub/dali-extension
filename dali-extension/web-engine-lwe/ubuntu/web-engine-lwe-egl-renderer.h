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

#pragma once

#include <EGL/egl.h>
#include <dali/public-api/common/dali-common.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace DALI_NAMESPACE
{
namespace Plugin
{
class WebEngineLweEglRenderer
{
public:
  using FrameCallback = std::function<void(std::vector<uint8_t>&&, uint32_t, uint32_t)>;

  WebEngineLweEglRenderer() = default;
  ~WebEngineLweEglRenderer();
  WebEngineLweEglRenderer(const WebEngineLweEglRenderer&) = delete;
  WebEngineLweEglRenderer& operator=(const WebEngineLweEglRenderer&) = delete;

  bool initialize(uint32_t width, uint32_t height);
  void shutdown();
  void resize(uint32_t width, uint32_t height);
  void setFrameCallback(FrameCallback callback);

  bool makeCurrent();
  bool swapBuffers();
  uintptr_t createSharedContext();
  bool destroyContext(uintptr_t context);
  bool clearCurrentContext();
  bool makeCurrentWithContext(uintptr_t context);
  void* getProcAddress(const char* name);
  bool isSupportedExtension(const char* extension) const;

private:
  bool initializeDisplay(EGLDisplay display);
  void destroyDisplay();
  EGLContext createContext(EGLContext shareContext);
  bool updateSurface();
  void loadExtensionString();
  bool captureFrame();

private:
  // Each renderer owns its X connection and EGL display. Terminating it must
  // not invalidate DALi's display or another WebView's graphics resources.
  EGLNativeDisplayType mNativeDisplay{nullptr};
  EGLDisplay mDisplay{EGL_NO_DISPLAY};
  EGLConfig mConfig{nullptr};
  EGLContext mContext{EGL_NO_CONTEXT};
  EGLSurface mSurface{EGL_NO_SURFACE};

  // Only resize runs on the event thread while LWE is rendering. Publish both
  // dimensions together; all EGL surface work belongs to the render thread.
  std::atomic<uint64_t> mRequestedSize{0u};
  uint32_t mSurfaceWidth{0u};
  uint32_t mSurfaceHeight{0u};
  std::string mExtensions;
  FrameCallback mFrameCallback;
};

} // namespace Plugin
} // namespace DALI_NAMESPACE
