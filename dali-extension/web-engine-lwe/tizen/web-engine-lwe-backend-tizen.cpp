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

#include "web-engine-lwe-backend-tizen.h"

#include <LWEWebView.h>

#include <dali/devel-api/adaptor-framework/application-devel.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/adaptor-framework/native-image.h>
#include <dali/public-api/adaptor-framework/window.h>
#include <dali/public-api/object/any.h>
#include <dali/public-api/signals/callback.h>

#include <tbm_dummy_display.h>
#include <tbm_surface_internal.h>
#include <unistd.h>
#include <vconf/vconf.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <utility>

namespace DALI_NAMESPACE
{
namespace Plugin
{
namespace
{
#ifndef OVER_TIZEN_VERSION_9
#define DB_NAME_LOCAL_STORAGE "LWE_localStorage.db"
#define DB_NAME_COOKIES "LWE_Cookies.db"
#define DB_NAME_CACHE "LWE_Cache.db"

class MutexLocker
{
public:
  explicit MutexLocker(pthread_mutex_t& mutex)
  : mMutex(mutex)
  {
    pthread_mutex_lock(&mMutex);
  }

  ~MutexLocker()
  {
    pthread_mutex_unlock(&mMutex);
  }

private:
  pthread_mutex_t& mMutex;
};
#endif

constexpr int               TBM_SURFACE_QUEUE_LENGTH = 3;
// Upper bound on waiting for DALi to report that a replaced surface is no
// longer read. Only reached when no visible window renders, e.g. while the
// application is being hidden.
constexpr uint32_t          RETIRED_SURFACE_TIMEOUT_MS = 100u;
PFNEGLCREATESYNCKHRPROC     gEglCreateSyncKHR        = nullptr;
PFNEGLDESTROYSYNCKHRPROC    gEglDestroySyncKHR       = nullptr;
PFNEGLCLIENTWAITSYNCKHRPROC gEglClientWaitSyncKHR    = nullptr;

std::string GetLanguage()
{
  char* language = vconf_get_str(VCONFKEY_LANGSET);
  if(!language)
  {
    DALI_LOG_ERROR("WebEngineLwe: failed to get VCONFKEY_LANGSET\n");
    return {};
  }

  std::string result(language);
  free(language);
  return result;
}

std::string GetTimezone()
{
  char* timezone = vconf_get_str(VCONFKEY_SETAPPL_TIMEZONE_ID);
  if(!timezone)
  {
    DALI_LOG_ERROR("WebEngineLwe: failed to get VCONFKEY_SETAPPL_TIMEZONE_ID\n");
    return {};
  }

  std::string result(timezone);
  free(timezone);
  return result;
}
} // unnamed namespace

WebEngineLweBackendTizen::WebEngineLweBackendTizen()
#ifndef OVER_TIZEN_VERSION_9
: mOutputWidth(0u),
  mOutputHeight(0u),
  mOutputStride(0u),
  mOutputBuffer(nullptr),
  mTbmSurface(nullptr),
  mUpdateBufferTrigger(Dali::MakeCallback(this, &WebEngineLweBackendTizen::LegacyUpdateBuffer)),
  mWebContainer(nullptr),
#else
: mWebContainer(nullptr),
#endif
  mNativeImage(Dali::NativeImage::New(0u, 0u, Dali::NativeImage::COLOR_DEPTH_DEFAULT)),
  mNativeDisplay(nullptr),
  mEglDisplay(EGL_NO_DISPLAY),
  mEglConfig(nullptr),
  mEglSurface(EGL_NO_SURFACE),
  mEglContext(EGL_NO_CONTEXT),
  mEglSync(nullptr),
  mTbmQueue(nullptr),
  mLastDrawnTbmSurface(nullptr),
  mIdleTbmSurface(nullptr),
  mRetiredFrameId(0),
  mAliveToken(std::make_shared<bool>(true)),
  mRenderingWaitsForRetiredSurface(false),
  mLweRenderingRequested(false),
  mInImageUpdateState(false),
  mInIdleState(false),
  mIdleReleasePending(false),
  mDestroying(false)
{
#ifndef OVER_TIZEN_VERSION_9
  pthread_mutex_init(&mOutputBufferMutex, nullptr);
#endif
}

WebEngineLweBackendTizen::~WebEngineLweBackendTizen()
{
  Destroy();
#ifndef OVER_TIZEN_VERSION_9
  pthread_mutex_destroy(&mOutputBufferMutex);
#endif
}

LWE::WebContainer* WebEngineLweBackendTizen::Create(uint32_t width, uint32_t height, uint32_t argc, char** argv)
{
#ifdef OVER_TIZEN_VERSION_9
  for(uint32_t index = 0u; index < argc; ++index)
  {
    if(argv[index])
    {
      if(std::strstr(argv[index], "--prefer-updated-version"))
      {
        LWE::LWE::SetVersionPreference(true);
      }
      else if(std::strstr(argv[index], "--prefer-platform-version"))
      {
        LWE::LWE::SetVersionPreference(false);
      }
    }
  }
#else
  (void)argc;
  (void)argv;
#endif
  return Create(width, height, GetLanguage(), GetTimezone());
}

LWE::WebContainer* WebEngineLweBackendTizen::Create(uint32_t           width,
                                                    uint32_t           height,
                                                    const std::string& locale,
                                                    const std::string& timezoneId)
{
  mDestroying            = false;
  mLweRenderingRequested = false;
  mInImageUpdateState    = false;
  mInIdleState           = false;
  mIdleReleasePending    = false;

  mRenderingWaitsForRetiredSurface = false;
  if(!mRetiredSurfaceTimer)
  {
    mRetiredSurfaceTimer = Dali::Timer::New(RETIRED_SURFACE_TIMEOUT_MS);
    mRetiredSurfaceTimer.TickSignal().Connect(this, &WebEngineLweBackendTizen::OnRetiredSurfaceTimeout);
  }

#ifndef OVER_TIZEN_VERSION_9
  mOutputWidth  = width;
  mOutputHeight = height;
  mOutputStride = width * sizeof(uint32_t);
  mOutputBuffer = nullptr;
#endif

  InitRenderingContext();

  if(!LWE::LWE::IsInitialized())
  {
    const std::string dataPath = Dali::DevelApplication::GetDataPath();
#ifdef OVER_TIZEN_VERSION_9
    LWE::LWE::Initialize((dataPath + "/StarFishStorage").c_str());
#else
    LWE::LWE::Initialize((dataPath + DB_NAME_LOCAL_STORAGE).c_str(),
                         (dataPath + DB_NAME_COOKIES).c_str(),
                         (dataPath + DB_NAME_CACHE).c_str());
#endif
  }

  gEglCreateSyncKHR     = reinterpret_cast<PFNEGLCREATESYNCKHRPROC>(eglGetProcAddress("eglCreateSyncKHR"));
  gEglDestroySyncKHR    = reinterpret_cast<PFNEGLDESTROYSYNCKHRPROC>(eglGetProcAddress("eglDestroySyncKHR"));
  gEglClientWaitSyncKHR = reinterpret_cast<PFNEGLCLIENTWAITSYNCKHRPROC>(eglGetProcAddress("eglClientWaitSyncKHR"));

#ifdef OVER_TIZEN_VERSION_9
  LWE::WebContainer::WebContainerArguments arguments{
    .width            = static_cast<unsigned>(width),
    .height           = static_cast<unsigned>(height),
    .devicePixelRatio = 1.0,
    .defaultFontName  = "serif",
    .locale           = locale.data(),
    .timezoneID       = timezoneId.data(),
  };

  LWE::WebContainer::RendererGLConfiguration configuration;
  configuration.onMakeCurrent = [this](LWE::WebContainer*)
  {
    if(!eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext))
    {
      DALI_LOG_ERROR("WebEngineLwe: eglMakeCurrent failed: %d\n", static_cast<int>(eglGetError()));
    }
  };
  configuration.onSwapBuffers = [this](LWE::WebContainer*, bool mayNeedSync)
  {
    if(!eglSwapBuffers(mEglDisplay, mEglSurface))
    {
      DALI_LOG_ERROR("WebEngineLwe: eglSwapBuffers failed: %d\n", static_cast<int>(eglGetError()));
    }
    mEglSync = gEglCreateSyncKHR ? gEglCreateSyncKHR(mEglDisplay, EGL_SYNC_FENCE_KHR, nullptr) : nullptr;
    eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    mLweRenderingRequested = false;
    TryUpdateImage(mayNeedSync);
  };
  configuration.onCreateSharedContext = [this](LWE::WebContainer*) -> uintptr_t
  {
    const EGLint attributes[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_NONE};
    return reinterpret_cast<uintptr_t>(eglCreateContext(mEglDisplay, mEglConfig, mEglContext, attributes));
  };
  configuration.onDestroyContext = [this](LWE::WebContainer*, uintptr_t context)
  {
    return eglDestroyContext(mEglDisplay, reinterpret_cast<EGLContext>(context));
  };
  configuration.onClearCurrentContext = [this](LWE::WebContainer*)
  {
    return eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  };
  configuration.onMakeCurrentWithContext = [this](LWE::WebContainer*, uintptr_t context)
  {
    return eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, reinterpret_cast<EGLContext>(context));
  };
  configuration.onGetProcAddress = [](LWE::WebContainer*, const char* name) -> void*
  {
    return reinterpret_cast<void*>(eglGetProcAddress(name));
  };
  configuration.onIsSupportedExtension = [](LWE::WebContainer*, const char* name)
  {
    const char* extensions = eglQueryString(eglGetCurrentDisplay(), EGL_EXTENSIONS);
    return extensions && std::strstr(extensions, name);
  };

  mWebContainer = LWE::WebContainer::CreateGL(arguments, configuration);
  DALI_ASSERT_ALWAYS(mWebContainer && "Failed to create LWE GL WebContainer");
  mWebContainer->RegisterSetNeedsRenderingCallback(
    [this](LWE::WebContainer*, const std::function<void()>& render)
  {
    // A new rendering request supersedes any deferred idle release.
    mIdleReleasePending = false;
    if(!mLweRenderingFunction)
    {
      mLweRenderingFunction = render;
    }
    if(!mLweRenderingRequested.exchange(true))
    {
      PrepareLweRendering();
    }
  });
  mWebContainer->RegisterOnIdleHandler([this](LWE::WebContainer*)
  {
    OnIdle();
  });
  mWebContainer->LoadURL("about:blank");
#else
  mWebContainer = LWE::WebContainer::Create(mOutputWidth,
                                            mOutputHeight,
                                            1.0,
                                            "",
                                            locale.data(),
                                            timezoneId.data());
  DALI_ASSERT_ALWAYS(mWebContainer && "Failed to create LWE WebContainer");

  mWebContainer->RegisterPreRenderingHandler([this]() -> LWE::WebContainer::RenderInfo
  {
    if(!mOutputBuffer)
    {
      mOutputBuffer = static_cast<uint8_t*>(malloc(mOutputWidth * mOutputHeight * sizeof(uint32_t)));
      mOutputStride = mOutputWidth * sizeof(uint32_t);
    }

    LWE::WebContainer::RenderInfo result;
    result.updatedBufferAddress = mOutputBuffer;
    result.bufferStride         = mOutputStride;
    return result;
  });

  mWebContainer->RegisterOnRenderedHandler(
    [this](LWE::WebContainer*, const LWE::WebContainer::RenderResult& renderResult)
  {
    if(renderResult.updatedWidth != mOutputWidth || renderResult.updatedHeight != mOutputHeight || !mTbmSurface)
    {
      return;
    }

    {
      MutexLocker        lock(mOutputBufferMutex);
      tbm_surface_info_s surfaceInfo;
      if(tbm_surface_map(mTbmSurface, TBM_SURF_OPTION_READ | TBM_SURF_OPTION_WRITE, &surfaceInfo) != TBM_SURFACE_ERROR_NONE)
      {
        DALI_LOG_ERROR("WebEngineLwe: failed to map tbm_surface\n");
        return;
      }

      DALI_ASSERT_ALWAYS(surfaceInfo.format == TBM_FORMAT_ARGB8888 && "Unsupported LWE TBM format");
      const uint32_t sourceStride = static_cast<uint32_t>(renderResult.updatedWidth * sizeof(uint32_t));
      auto*          source       = static_cast<uint8_t*>(renderResult.updatedBufferAddress);
      auto*          destination  = static_cast<uint8_t*>(surfaceInfo.planes[0].ptr);
      const size_t   targetStride = surfaceInfo.planes[0].stride;

      if(targetStride == sourceStride)
      {
        std::memcpy(destination, source, surfaceInfo.planes[0].size);
      }
      else
      {
        for(size_t y = 0u; y < renderResult.updatedHeight; ++y)
        {
          std::memcpy(destination + y * targetStride, source + y * sourceStride, sourceStride);
        }
      }

      if(tbm_surface_unmap(mTbmSurface) != TBM_SURFACE_ERROR_NONE)
      {
        DALI_LOG_ERROR("WebEngineLwe: failed to unmap tbm_surface\n");
      }
    }
    mUpdateBufferTrigger.Trigger();
  });

  SetSize(width, height);
#endif

  return mWebContainer;
}

void WebEngineLweBackendTizen::Destroy()
{
  if(!mWebContainer && mEglDisplay == EGL_NO_DISPLAY)
  {
    return;
  }

  mDestroying = true;
  if(mWebContainer)
  {
    mWebContainer->Destroy();
    mWebContainer = nullptr;
  }
  DestroyRenderingContext();

#ifndef OVER_TIZEN_VERSION_9
  if(mTbmSurface)
  {
    if(tbm_surface_destroy(mTbmSurface) != TBM_SURFACE_ERROR_NONE)
    {
      DALI_LOG_ERROR("WebEngineLwe: failed to destroy tbm_surface\n");
    }
    mTbmSurface = nullptr;
  }
  free(mOutputBuffer);
  mOutputBuffer = nullptr;
#endif

  mLweRenderingFunction  = {};
  mLweRenderingRequested = false;
  mInImageUpdateState    = false;
  mInIdleState           = false;
  mIdleReleasePending    = false;

  // The queue is gone, so the retired surfaces only hold their own reference.
  // Their pending frame callbacks find nothing left to release.
  for(auto& retired : mRetiredSurfaces)
  {
    tbm_surface_internal_unref(retired.surface);
  }
  mRetiredSurfaces.clear();
  mRenderingWaitsForRetiredSurface = false;
  if(mRetiredSurfaceTimer)
  {
    mRetiredSurfaceTimer.Stop();
  }

  if(mIdleTbmSurface)
  {
    tbm_surface_internal_unref(mIdleTbmSurface);
    mIdleTbmSurface = nullptr;
  }

  mDestroying = false;
}

void WebEngineLweBackendTizen::SetSize(uint32_t width, uint32_t height)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  if(mWebContainer->Width() != width || mWebContainer->Height() != height)
  {
    mWebContainer->ResizeTo(width, height);
  }

#ifndef OVER_TIZEN_VERSION_9
  if(mOutputWidth != width || mOutputHeight != height || !mTbmSurface)
  {
    mOutputWidth  = width;
    mOutputHeight = height;
    mOutputStride = width * sizeof(uint32_t);

    tbm_surface_h previousSurface = mTbmSurface;
    mTbmSurface                   = tbm_surface_create(width, height, TBM_FORMAT_ARGB8888);
    mNativeImage->SetSource(Dali::Any(mTbmSurface));
    if(previousSurface && tbm_surface_destroy(previousSurface) != TBM_SURFACE_ERROR_NONE)
    {
      DALI_LOG_ERROR("WebEngineLwe: failed to destroy previous tbm_surface\n");
    }

    auto* previousBuffer = mOutputBuffer;
    mOutputBuffer        = static_cast<uint8_t*>(malloc(mOutputWidth * mOutputHeight * sizeof(uint32_t)));
    free(previousBuffer);
  }
#endif
}

void WebEngineLweBackendTizen::UpdateDisplayArea(uint32_t width, uint32_t height)
{
  mNativeImage = Dali::NativeImage::New(0u, 0u, Dali::NativeImage::COLOR_DEPTH_DEFAULT);
  SetSize(width, height);
}

Dali::NativeImagePtr WebEngineLweBackendTizen::GetNativeImage()
{
  return mNativeImage;
}

void WebEngineLweBackendTizen::SetFrameRenderedCallback(FrameRenderedCallback callback)
{
  mFrameRenderedCallback = std::move(callback);
}

void WebEngineLweBackendTizen::DispatchToEventThread(Task task)
{
  if(task)
  {
    task();
  }
}

void WebEngineLweBackendTizen::InitRenderingContext()
{
  if(mNativeDisplay)
  {
    return;
  }

  mNativeDisplay = reinterpret_cast<EGLNativeDisplayType>(tbm_dummy_display_create());
  DALI_ASSERT_ALWAYS(mNativeDisplay && "Failed to create TBM dummy display");

  mEglDisplay = eglGetDisplay(mNativeDisplay);
  DALI_ASSERT_ALWAYS(mEglDisplay != EGL_NO_DISPLAY && "Failed to get EGL display");
  DALI_ASSERT_ALWAYS(eglInitialize(mEglDisplay, nullptr, nullptr) == EGL_TRUE && "Failed to initialize EGL");

  EGLint configCount = 0;
  DALI_ASSERT_ALWAYS(eglGetConfigs(mEglDisplay, nullptr, 0, &configCount) == EGL_TRUE && "Failed to query EGL configs");

  const EGLint configAttributes[] = {
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, 0,
    EGL_STENCIL_SIZE, 8,
    EGL_SAMPLE_BUFFERS, 1,
    EGL_SAMPLES, 4,
    EGL_NONE};

  DALI_ASSERT_ALWAYS(eglChooseConfig(mEglDisplay, configAttributes, &mEglConfig, 1, &configCount) == EGL_TRUE && configCount > 0 && "Failed to choose EGL config");

  const EGLint contextAttributes[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
  mEglContext                      = eglCreateContext(mEglDisplay, mEglConfig, EGL_NO_CONTEXT, contextAttributes);
  DALI_ASSERT_ALWAYS(mEglContext != EGL_NO_CONTEXT && "Failed to create EGL context");
}

void WebEngineLweBackendTizen::DestroyRenderingContext()
{
  DestroyRenderingSurface();

  if(mEglDisplay != EGL_NO_DISPLAY)
  {
    if(mEglSync)
    {
      if(gEglDestroySyncKHR)
      {
        gEglDestroySyncKHR(mEglDisplay, mEglSync);
      }
      mEglSync = nullptr;
    }
    if(mEglContext != EGL_NO_CONTEXT)
    {
      eglDestroyContext(mEglDisplay, mEglContext);
      mEglContext = EGL_NO_CONTEXT;
    }
    eglTerminate(mEglDisplay);
    mEglDisplay = EGL_NO_DISPLAY;
  }

  if(mNativeDisplay)
  {
    tbm_dummy_display_destroy(reinterpret_cast<tbm_dummy_display*>(mNativeDisplay));
    mNativeDisplay = nullptr;
  }
}

void WebEngineLweBackendTizen::InitRenderingSurface()
{
  if(mEglSurface != EGL_NO_SURFACE || !mWebContainer)
  {
    return;
  }

  mTbmQueue = tbm_surface_queue_create(TBM_SURFACE_QUEUE_LENGTH,
                                       std::max(mWebContainer->Width(), static_cast<size_t>(1u)),
                                       std::max(mWebContainer->Height(), static_cast<size_t>(1u)),
                                       TBM_FORMAT_BGRA8888,
                                       TBM_BO_DEFAULT);
  DALI_ASSERT_ALWAYS(mTbmQueue && "Failed to create TBM surface queue");

  mEglSurface = eglCreateWindowSurface(mEglDisplay,
                                       mEglConfig,
                                       reinterpret_cast<EGLNativeWindowType>(mTbmQueue),
                                       nullptr);
  DALI_ASSERT_ALWAYS(mEglSurface != EGL_NO_SURFACE && "Failed to create EGL window surface");
}

void WebEngineLweBackendTizen::DestroyRenderingSurface()
{
  if(mEglDisplay != EGL_NO_DISPLAY)
  {
    eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  }

  if(mLastDrawnTbmSurface)
  {
    tbm_surface_internal_ref(mLastDrawnTbmSurface);
    if(mIdleTbmSurface)
    {
      tbm_surface_internal_unref(mIdleTbmSurface);
    }
    mIdleTbmSurface      = mLastDrawnTbmSurface;
    mLastDrawnTbmSurface = nullptr;
  }

  // Surfaces DALi may still be reading outlive the queue through the reference
  // RetireSurface() took; they are only unreferenced once DALi is done.
  for(auto& retired : mRetiredSurfaces)
  {
    if(retired.queue == mTbmQueue)
    {
      retired.queue = nullptr;
    }
  }
  mRenderingWaitsForRetiredSurface = false;

  if(mEglSurface != EGL_NO_SURFACE)
  {
    eglDestroySurface(mEglDisplay, mEglSurface);
    mEglSurface = EGL_NO_SURFACE;
  }

  if(mTbmQueue)
  {
    tbm_surface_queue_destroy(mTbmQueue);
    mTbmQueue = nullptr;
  }
}

void WebEngineLweBackendTizen::TryRendering()
{
  if(mDestroying || !mWebContainer)
  {
    return;
  }

  if(mTbmQueue &&
     (static_cast<size_t>(tbm_surface_queue_get_width(mTbmQueue)) != mWebContainer->Width() ||
      static_cast<size_t>(tbm_surface_queue_get_height(mTbmQueue)) != mWebContainer->Height()))
  {
    DestroyRenderingSurface();
  }
  InitRenderingSurface();
  OnActive();

  unsigned waitCount = 0u;
  while(!tbm_surface_queue_can_dequeue(mTbmQueue, 0) && waitCount < 10u)
  {
    usleep(100u);
    ++waitCount;
  }

  if(tbm_surface_queue_can_dequeue(mTbmQueue, 0))
  {
    if(mLweRenderingFunction)
    {
      // Image delivery unbinds the context; restore it before rendering.
      if(eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext) == EGL_FALSE)
      {
        DALI_LOG_ERROR("WebEngineLwe: eglMakeCurrent before rendering failed: %d\n", static_cast<int>(eglGetError()));
        mLweRenderingRequested = false;
        return;
      }
      mLweRenderingFunction();
    }
  }
  else if(HasRetiredSurfaceOf(mTbmQueue))
  {
    // The free buffers are the ones DALi may still be reading. Resume once it
    // reports them released instead of rendering over them.
    mRenderingWaitsForRetiredSurface = true;
  }
  else
  {
    mWebContainer->AddIdleCallback([](void* data)
    {
      static_cast<WebEngineLweBackendTizen*>(data)->TryRendering();
    }, this);
  }
}

void WebEngineLweBackendTizen::TryUpdateImage(bool needsSync)
{
  if(mDestroying || !mWebContainer)
  {
    return;
  }

  mInImageUpdateState = true;
  if(!eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext))
  {
    DALI_LOG_ERROR("WebEngineLwe: eglMakeCurrent failed: %d\n", static_cast<int>(eglGetError()));
  }

  if(mEglSync && gEglClientWaitSyncKHR)
  {
    const auto state = gEglClientWaitSyncKHR(mEglDisplay,
                                             mEglSync,
                                             0,
                                             needsSync ? EGL_FOREVER_KHR : 1000u * 1000u);
    if(state == EGL_TIMEOUT_EXPIRED_KHR)
    {
      eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
      mWebContainer->AddIdleCallback([](void* data)
      {
        static_cast<WebEngineLweBackendTizen*>(data)->TryUpdateImage(false);
      }, this);
      return;
    }
    if(gEglDestroySyncKHR)
    {
      gEglDestroySyncKHR(mEglDisplay, mEglSync);
    }
    mEglSync = nullptr;
  }

  eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

  unsigned waitCount = 0u;
  while(!tbm_surface_queue_can_acquire(mTbmQueue, 0) && waitCount < 10u)
  {
    usleep(100u);
    ++waitCount;
  }

  if(tbm_surface_queue_can_acquire(mTbmQueue, 0))
  {
    tbm_surface_h acquired = nullptr;
    if(tbm_surface_queue_acquire(mTbmQueue, &acquired) == TBM_SURFACE_QUEUE_ERROR_NONE)
    {
      if(UpdateImage(acquired))
      {
        // SetSource() only takes effect in a later DALi frame, and DALi's
        // render thread may still be sampling the surface it replaces. Hand
        // that surface back only once DALi has rendered past it; releasing
        // it here let LWE render over a buffer still on screen.
        if(mLastDrawnTbmSurface)
        {
          tbm_surface_internal_ref(mLastDrawnTbmSurface);
          RetireSurface(mLastDrawnTbmSurface, mTbmQueue);
        }
        mLastDrawnTbmSurface = acquired;
        if(mIdleTbmSurface)
        {
          RetireSurface(mIdleTbmSurface, nullptr);
          mIdleTbmSurface = nullptr;
        }
      }
      else
      {
        // Never shown, so nothing reads it.
        tbm_surface_queue_release(mTbmQueue, acquired);
      }
      mInImageUpdateState = false;
      TryReleaseIdleSurface();
      return;
    }
    DALI_LOG_ERROR("WebEngineLwe: failed to acquire TBM surface\n");
  }

  mWebContainer->AddIdleCallback([](void* data)
  {
    static_cast<WebEngineLweBackendTizen*>(data)->TryUpdateImage(false);
  }, this);
}

void WebEngineLweBackendTizen::PrepareLweRendering()
{
  if(mDestroying || !mWebContainer)
  {
    return;
  }

  if(mInImageUpdateState)
  {
    mWebContainer->AddIdleCallback([](void* data)
    {
      static_cast<WebEngineLweBackendTizen*>(data)->PrepareLweRendering();
    }, this);
    return;
  }

  mWebContainer->AddIdleCallback([](void* data)
  {
    static_cast<WebEngineLweBackendTizen*>(data)->TryRendering();
  }, this);
}

void WebEngineLweBackendTizen::OnIdle()
{
  mIdleReleasePending = true;
  TryReleaseIdleSurface();
}

void WebEngineLweBackendTizen::TryReleaseIdleSurface()
{
  // LWE idle notifications and rendering retries run on the LWE event loop.
  // Idle does not imply that the external image update has completed.
  if(mDestroying || !mWebContainer || !mIdleReleasePending ||
     mLweRenderingRequested || mInImageUpdateState)
  {
    return;
  }

  mIdleReleasePending = false;
  if(mInIdleState.exchange(true))
  {
    return;
  }

  if(mLastDrawnTbmSurface)
  {
    tbm_surface_internal_ref(mLastDrawnTbmSurface);
    if(mIdleTbmSurface)
    {
      tbm_surface_internal_unref(mIdleTbmSurface);
    }
    mIdleTbmSurface      = mLastDrawnTbmSurface;
    mLastDrawnTbmSurface = nullptr;
  }
  DestroyRenderingSurface();
}

void WebEngineLweBackendTizen::OnActive()
{
  if(!mInIdleState.exchange(false))
  {
    return;
  }
  InitRenderingSurface();
}

void WebEngineLweBackendTizen::RetireSurface(tbm_surface_h surface, tbm_surface_queue_h queue)
{
  // Takes over one reference of the surface from the caller.
  Dali::WindowContainer windows;
  if(Dali::Adaptor::IsAvailable())
  {
    windows = Dali::Adaptor::Get().GetWindows();
  }

  RetiredSurface retired{surface, queue, ++mRetiredFrameId, 0u, std::chrono::steady_clock::now()};
  std::weak_ptr<bool> alive = mAliveToken;
  for(auto& window : windows)
  {
    if(!window.IsVisible())
    {
      continue;
    }
    ++retired.pendingWindowCount;
    auto onRendered = [this, alive](int32_t frameId)
    {
      if(!alive.expired())
      {
        OnRetiredSurfaceRendered(frameId);
      }
    };
    window.AddFrameRenderedCallback(new Dali::CallbackFunctor1<decltype(onRendered), int32_t>(onRendered), retired.frameId);
  }

  mRetiredSurfaces.push_back(retired);
  if(retired.pendingWindowCount == 0u)
  {
    // No window renders, so nothing reads the surface any more.
    ReleaseRetiredSurfaces();
  }
  else if(mRetiredSurfaceTimer && !mRetiredSurfaceTimer.IsRunning())
  {
    mRetiredSurfaceTimer.Start();
  }
}

void WebEngineLweBackendTizen::OnRetiredSurfaceRendered(int32_t frameId)
{
  for(auto& retired : mRetiredSurfaces)
  {
    if(retired.frameId == frameId && retired.pendingWindowCount > 0u)
    {
      --retired.pendingWindowCount;
      break;
    }
  }
  ReleaseRetiredSurfaces();
}

bool WebEngineLweBackendTizen::OnRetiredSurfaceTimeout()
{
  if(mRetiredSurfaces.empty())
  {
    return false;
  }
  // A window that stops rendering never reports its frame; do not let that
  // stall LWE for good.
  const auto expired = std::chrono::steady_clock::now() - std::chrono::milliseconds(RETIRED_SURFACE_TIMEOUT_MS);
  for(auto& retired : mRetiredSurfaces)
  {
    if(retired.retiredTime > expired)
    {
      break;
    }
    if(retired.pendingWindowCount > 0u)
    {
      DALI_LOG_ERROR("WebEngineLwe: no frame rendered for %d ms, releasing a retired surface\n", static_cast<int>(RETIRED_SURFACE_TIMEOUT_MS));
      retired.pendingWindowCount = 0u;
    }
  }
  ReleaseRetiredSurfaces();
  return !mRetiredSurfaces.empty();
}

void WebEngineLweBackendTizen::ReleaseRetiredSurfaces()
{
  // A later frame being rendered implies the earlier ones were, so surfaces
  // are released in the order they were retired.
  bool released = false;
  while(!mRetiredSurfaces.empty() && mRetiredSurfaces.front().pendingWindowCount == 0u)
  {
    RetiredSurface retired = mRetiredSurfaces.front();
    mRetiredSurfaces.erase(mRetiredSurfaces.begin());
    if(retired.queue && retired.queue == mTbmQueue)
    {
      tbm_surface_queue_release(mTbmQueue, retired.surface);
      released = true;
    }
    tbm_surface_internal_unref(retired.surface);
  }

  if(mRetiredSurfaces.empty() && mRetiredSurfaceTimer)
  {
    mRetiredSurfaceTimer.Stop();
  }

  if(released && mRenderingWaitsForRetiredSurface && mWebContainer)
  {
    mRenderingWaitsForRetiredSurface = false;
    mWebContainer->AddIdleCallback([](void* data)
    {
      static_cast<WebEngineLweBackendTizen*>(data)->TryRendering();
    }, this);
  }
}

bool WebEngineLweBackendTizen::HasRetiredSurfaceOf(tbm_surface_queue_h queue) const
{
  for(const auto& retired : mRetiredSurfaces)
  {
    if(retired.queue && retired.queue == queue)
    {
      return true;
    }
  }
  return false;
}

#ifndef OVER_TIZEN_VERSION_9
void WebEngineLweBackendTizen::LegacyUpdateBuffer()
{
  UpdateImage(mTbmSurface);
}
#endif

bool WebEngineLweBackendTizen::UpdateImage(tbm_surface_h image)
{
  if(!mWebContainer || !image)
  {
    return false;
  }

  if(static_cast<int>(mWebContainer->Width()) != tbm_surface_get_width(image) ||
     static_cast<int>(mWebContainer->Height()) != tbm_surface_get_height(image))
  {
    DALI_LOG_DEBUG_INFO("WebEngineLwe: image and WebContainer sizes do not match\n");
    return false;
  }

#ifndef OVER_TIZEN_VERSION_9
  MutexLocker lock(mOutputBufferMutex);
#endif
  mNativeImage->SetSource(Dali::Any(image));
  if(Dali::Adaptor::IsAvailable())
  {
    Dali::Adaptor::Get().RequestProcessEventsAndUpdate();
  }
  if(mFrameRenderedCallback)
  {
    mFrameRenderedCallback();
  }
  return true;
}

std::unique_ptr<WebEngineLweBackend> CreateWebEngineLweBackend()
{
  return std::make_unique<WebEngineLweBackendTizen>();
}

void ClearWebEngineLweCookies()
{
}

} // namespace Plugin
} //namespace DALI_NAMESPACE
