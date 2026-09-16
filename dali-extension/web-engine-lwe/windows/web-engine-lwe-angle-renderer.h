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

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
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
class WebEngineLweAngleRenderer
{
public:
  using FrameCallback = std::function<void(std::vector<uint8_t>&&, uint32_t, uint32_t)>;

  WebEngineLweAngleRenderer() = default;
  ~WebEngineLweAngleRenderer();
  WebEngineLweAngleRenderer(const WebEngineLweAngleRenderer&) = delete;
  WebEngineLweAngleRenderer& operator=(const WebEngineLweAngleRenderer&) = delete;

  bool Initialize(uint32_t width, uint32_t height);
  void Shutdown();
  void Resize(uint32_t width, uint32_t height);
  void SetFrameCallback(FrameCallback callback);

  bool MakeCurrent();
  bool SwapBuffers();
  uintptr_t CreateSharedContext();
  bool DestroyContext(uintptr_t context);
  bool ClearCurrentContext();
  bool MakeCurrentWithContext(uintptr_t context);
  void* GetProcAddress(const char* name);
  bool IsSupportedExtension(const char* extension) const;

private:
  bool InitializeDisplay(EGLDisplay display);
  void DestroyDisplay();
  EGLContext CreateContext(EGLContext shareContext);
  bool UpdateSurface();
  void LoadExtensionString();
  bool CaptureFrame();

private:
  // A distinct native display prevents eglTerminate from invalidating another
  // web view's display or the display owned by DALi's ANGLE backend.
  HWND mWindow{nullptr};
  HDC mDeviceContext{nullptr};
  EGLDisplay mDisplay{EGL_NO_DISPLAY};
  EGLConfig mConfig{nullptr};
  EGLContext mContext{EGL_NO_CONTEXT};
  EGLSurface mSurface{EGL_NO_SURFACE};

  // Only Resize runs on the event thread while LWE is rendering. Publish both
  // dimensions together; all EGL surface work belongs to the render thread.
  std::atomic<uint64_t> mRequestedSize{0u};
  uint32_t mSurfaceWidth{0u};
  uint32_t mSurfaceHeight{0u};
  std::string mExtensions;
  FrameCallback mFrameCallback;
};

} // namespace Plugin
} // namespace DALI_NAMESPACE
