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

#ifndef DALI_EXTENSION_WEB_ENGINE_LWE_BACKEND_TIZEN_H
#define DALI_EXTENSION_WEB_ENGINE_LWE_BACKEND_TIZEN_H

#include "../common/web-engine-lwe-backend.h"

#include <dali/devel-api/adaptor-framework/event-thread-callback.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <tbm_surface.h>
#include <tbm_surface_queue.h>

#include <atomic>
#include <cstddef>
#include <functional>
#include <pthread.h>

namespace Dali
{
namespace Plugin
{
class WebEngineLweBackendTizen : public WebEngineLweBackend
{
public:
  WebEngineLweBackendTizen();
  ~WebEngineLweBackendTizen() override;

  LWE::WebContainer* Create(uint32_t width, uint32_t height, const std::string& locale, const std::string& timezoneId) override;
  LWE::WebContainer* Create(uint32_t width, uint32_t height, uint32_t argc, char** argv) override;
  void Destroy() override;
  void SetSize(uint32_t width, uint32_t height) override;
  void UpdateDisplayArea(uint32_t width, uint32_t height) override;
  Dali::NativeImagePtr GetNativeImage() override;
  void SetFrameRenderedCallback(FrameRenderedCallback callback) override;
  void DispatchToEventThread(Task task) override;

private:
#ifndef OVER_TIZEN_VERSION_9
  void LegacyUpdateBuffer();
#endif

  void InitRenderingContext();
  void DestroyRenderingContext();
  void InitRenderingSurface();
  void DestroyRenderingSurface();
  void TryRendering();
  void TryUpdateImage(bool needsSync);
  void PrepareLweRendering();
  void OnIdle();
  void OnActive();
  void OnFirstRender();
  void UpdateImage(tbm_surface_h image);

private:
#ifndef OVER_TIZEN_VERSION_9
  size_t   mOutputWidth;
  size_t   mOutputHeight;
  size_t   mOutputStride;
  uint8_t* mOutputBuffer;

  pthread_mutex_t mOutputBufferMutex;
  tbm_surface_h   mTbmSurface;

  Dali::EventThreadCallback mUpdateBufferTrigger;
#endif

  LWE::WebContainer*   mWebContainer;
  Dali::NativeImagePtr mNativeImage;

  EGLNativeDisplayType mNativeDisplay;
  EGLDisplay           mEglDisplay;
  EGLConfig            mEglConfig;
  EGLSurface           mEglSurface;
  EGLContext           mEglContext;
  EGLSyncKHR           mEglSync;

  tbm_surface_queue_h mTbmQueue;
  tbm_surface_h       mLastDrawnTbmSurface;
  tbm_surface_h       mIdleTbmSurface;

  std::function<void()> mLweRenderingFunction;
  std::atomic_bool      mLweRenderingRequested;
  std::atomic_bool      mInImageUpdateState;
  std::atomic_bool      mInIdleState;
  std::atomic_bool      mFirstRenderEnded;
  std::atomic_bool      mDestroying;

  FrameRenderedCallback mFrameRenderedCallback;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_EXTENSION_WEB_ENGINE_LWE_BACKEND_TIZEN_H
