/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include "tizen-web-engine-lwe.h"

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/application-devel.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-back-forward-list-item.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-back-forward-list.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-context.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-cookie-manager.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-hit-test.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-security-origin.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-settings.h>
#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/events/touch-event.h>

#include <unistd.h>
#include <vconf/vconf.h>

using namespace LWE;

// The plugin factories
extern "C" DALI_EXPORT_API Dali::WebEnginePlugin* CreateWebEnginePlugin(void)
{
  return new Dali::Plugin::TizenWebEngineLWE;
}

extern "C" DALI_EXPORT_API void DestroyWebEnginePlugin(Dali::WebEnginePlugin* plugin)
{
  if(plugin != NULL)
  {
    delete plugin;
  }
}

namespace
{
#ifndef OVER_TIZEN_VERSION_9
#define DB_NAME_LOCAL_STORAGE "LWE_localStorage.db"
#define DB_NAME_COOKIES "LWE_Cookies.db"
#define DB_NAME_CACHE "LWE_Cache.db"

class Locker
{
public:
  Locker(pthread_mutex_t& lock)
  : m_lock(lock)
  {
    pthread_mutex_lock(&m_lock);
  }

  ~Locker()
  {
    pthread_mutex_unlock(&m_lock);
  }

protected:
  pthread_mutex_t m_lock;
};
#endif
} // namespace

namespace Dali
{
namespace Plugin
{
namespace
{
LWE::KeyValue KeyStringToKeyValue(const char* DALIKeyString, bool isShiftPressed)
{
  LWE::KeyValue keyValue = LWE::KeyValue::UnidentifiedKey;
  if(strcmp("Left", DALIKeyString) == 0)
  {
    keyValue = LWE::KeyValue::ArrowLeftKey;
  }
  else if(strcmp("Right", DALIKeyString) == 0)
  {
    keyValue = LWE::KeyValue::ArrowRightKey;
  }
  else if(strcmp("Up", DALIKeyString) == 0)
  {
    keyValue = LWE::KeyValue::ArrowUpKey;
  }
  else if(strcmp("Down", DALIKeyString) == 0)
  {
    keyValue = LWE::KeyValue::ArrowDownKey;
  }
  else if(strcmp("space", DALIKeyString) == 0)
  {
    keyValue = LWE::KeyValue::SpaceKey;
  }
  else if(strcmp("Return", DALIKeyString) == 0)
  {
    keyValue = LWE::KeyValue::EnterKey;
  }
  else if(strcmp("BackSpace", DALIKeyString) == 0)
  {
    keyValue = LWE::KeyValue::BackspaceKey;
  }
  else if(strcmp("Escape", DALIKeyString) == 0)
  {
    keyValue = LWE::KeyValue::EscapeKey;
  }
  else if(strcmp("minus", DALIKeyString) == 0)
  {
    if(isShiftPressed)
    {
      keyValue = LWE::KeyValue::MinusMarkKey;
    }
    else
    {
      keyValue = LWE::KeyValue::UnderScoreMarkKey;
    }
  }
  else if(strcmp("equal", DALIKeyString) == 0)
  {
    if(isShiftPressed)
    {
      keyValue = LWE::KeyValue::PlusMarkKey;
    }
    else
    {
      keyValue = LWE::KeyValue::EqualitySignKey;
    }
  }
  else if(strcmp("bracketleft", DALIKeyString) == 0)
  {
    if(isShiftPressed)
    {
      keyValue = LWE::KeyValue::LeftCurlyBracketMarkKey;
    }
    else
    {
      keyValue = LWE::KeyValue::LeftSquareBracketKey;
    }
  }
  else if(strcmp("bracketright", DALIKeyString) == 0)
  {
    if(isShiftPressed)
    {
      keyValue = LWE::KeyValue::RightCurlyBracketMarkKey;
    }
    else
    {
      keyValue = LWE::KeyValue::RightSquareBracketKey;
    }
  }
  else if(strcmp("semicolon", DALIKeyString) == 0)
  {
    if(isShiftPressed)
    {
      keyValue = LWE::KeyValue::ColonMarkKey;
    }
    else
    {
      keyValue = LWE::KeyValue::SemiColonMarkKey;
    }
  }
  else if(strcmp("apostrophe", DALIKeyString) == 0)
  {
    if(isShiftPressed)
    {
      keyValue = LWE::KeyValue::DoubleQuoteMarkKey;
    }
    else
    {
      keyValue = LWE::KeyValue::SingleQuoteMarkKey;
    }
  }
  else if(strcmp("comma", DALIKeyString) == 0)
  {
    if(isShiftPressed)
    {
      keyValue = LWE::KeyValue::LessThanMarkKey;
    }
    else
    {
      keyValue = LWE::KeyValue::CommaMarkKey;
    }
  }
  else if(strcmp("period", DALIKeyString) == 0)
  {
    if(isShiftPressed)
    {
      keyValue = LWE::KeyValue::GreaterThanSignKey;
    }
    else
    {
      keyValue = LWE::KeyValue::PeriodKey;
    }
  }
  else if(strcmp("slash", DALIKeyString) == 0)
  {
    if(isShiftPressed)
    {
      keyValue = LWE::KeyValue::QuestionMarkKey;
    }
    else
    {
      keyValue = LWE::KeyValue::SlashKey;
    }
  }
  else if(strlen(DALIKeyString) == 1)
  {
    char ch = DALIKeyString[0];
    if(ch >= '0' && ch <= '9')
    {
      if(isShiftPressed)
      {
        switch(ch)
        {
          case '1':
          {
            keyValue = LWE::KeyValue::ExclamationMarkKey;
            break;
          }
          case '2':
          {
            keyValue = LWE::KeyValue::AtMarkKey;
            break;
          }
          case '3':
          {
            keyValue = LWE::KeyValue::SharpMarkKey;
            break;
          }
          case '4':
          {
            keyValue = LWE::KeyValue::DollarMarkKey;
            break;
          }
          case '5':
          {
            keyValue = LWE::KeyValue::PercentMarkKey;
            break;
          }
          case '6':
          {
            keyValue = LWE::KeyValue::CaretMarkKey;
            break;
          }
          case '7':
          {
            keyValue = LWE::KeyValue::AmpersandMarkKey;
            break;
          }
          case '8':
          {
            keyValue = LWE::KeyValue::AsteriskMarkKey;
            break;
          }
          case '9':
          {
            keyValue = LWE::KeyValue::LeftParenthesisMarkKey;
            break;
          }
          case '0':
          {
            keyValue = LWE::KeyValue::RightParenthesisMarkKey;
            break;
          }
        }
      }
      else
      {
        keyValue = (LWE::KeyValue)(LWE::KeyValue::Digit0Key + ch - '0');
      }
    }
    else if(ch >= 'a' && ch <= 'z')
    {
      int kv = LWE::KeyValue::LowerAKey + ch - 'a';
      if(isShiftPressed)
      {
        kv -= ('z' - 'a');
        kv -= 7;
      }
      keyValue = (LWE::KeyValue)kv;
    }
  }
  return keyValue;
}

template<typename Callback, typename... Args>
void ExecuteCallback(Callback callback, Args... args)
{
  if(callback)
  {
    callback(args...);
  }
}

template<typename Callback, typename Arg>
void ExecuteCallback(Callback callback, std::unique_ptr<Arg> arg)
{
  if(callback)
  {
    callback(std::move(arg));
  }
}

template<typename Callback, typename Arg>
void ExecuteCallback(Callback callback, Arg*& arg)
{
  if(callback)
  {
    callback(arg);
  }
}

template<typename Ret, typename Callback, typename... Args>
Ret ExecuteCallbackReturn(Callback callback, Args... args)
{
  Ret returnVal = Ret();
  if(callback)
  {
    returnVal = callback(args...);
  }
  return returnVal;
}

template<typename Ret, typename Callback, typename Arg>
Ret ExecuteCallbackReturn(Callback callback, std::unique_ptr<Arg> arg)
{
  Ret returnVal = Ret();
  if(callback)
  {
    returnVal = callback(std::move(arg));
  }
  return returnVal;
}

} // Anonymous namespace

static constexpr int               gTbmSurfaceQueueLength = 3;
static PFNEGLCREATESYNCKHRPROC     gEglCreateSyncKHR;
static PFNEGLDESTROYSYNCKHRPROC    gEglDestroySyncKHR;
static PFNEGLCLIENTWAITSYNCKHRPROC gEglClientWaitSyncKHR;

TizenWebEngineLWE::TizenWebEngineLWE()
: mUrl(""),
  mIsMouseLbuttonDown(false),
  mCanGoBack(false),
  mCanGoForward(false),
#ifndef OVER_TIZEN_VERSION_9
  mOutputWidth(0),
  mOutputHeight(0),
  mOutputStride(0),
  mOutputBuffer(nullptr),
  mTbmSurface(nullptr),
  mUpdateBufferTrigger(MakeCallback(this, &TizenWebEngineLWE::LegacyUpdateBuffer)),
#endif
  mInDestroyingLWEInstance(false),
  mWebContainer(NULL),
  mDaliImageSrc(NativeImageSource::New(0, 0, NativeImageSource::COLOR_DEPTH_DEFAULT)),
  mNativeDisplay(NULL),
  mEglDisplay(EGL_NO_DISPLAY),
  mEglConfig(NULL),
  mEglSurface(EGL_NO_SURFACE),
  mEglContext(EGL_NO_CONTEXT),
  mEglSync(nullptr),
  mTbmQueue(nullptr),
  mLastDrawnTbmSurface(nullptr),
  mIdleTbmSurface(nullptr),
  mLWERenderingRequested(false),
  mInImageUpdateState(false),
  mInIdleState(false),
  mFirstRenderEnded(false),
  mFrameRenderedCallback(nullptr),
  mLoadStartedCallback(nullptr),
  mLoadFinishedCallback(nullptr)
{
#ifndef OVER_TIZEN_VERSION_9
  pthread_mutex_init(&mOutputBufferMutex, NULL);
#endif
}

TizenWebEngineLWE::~TizenWebEngineLWE()
{
#ifndef OVER_TIZEN_VERSION_9
  pthread_mutex_destroy(&mOutputBufferMutex);
#endif
  Destroy();
}

static std::string Langset()
{
  char* langset = vconf_get_str(VCONFKEY_LANGSET);
  if(!langset)
  {
    DALI_LOG_ERROR("TizenWebEngineLWE: system settings fail to get value: langset");
    return std::string();
  }

  std::string ls = langset;
  free(langset);

  return ls;
}

static std::string Timezone()
{
  char* tz = vconf_get_str(VCONFKEY_SETAPPL_TIMEZONE_ID);
  if(!tz)
  {
    DALI_LOG_ERROR("TizenWebEngineLWE: system settings fail to get value: VCONFKEY_SETAPPL_TIMEZONE_ID");
    return std::string();
  }

  std::string s = tz;
  free(tz);

  return s;
}

void TizenWebEngineLWE::Create(uint32_t width, uint32_t height, uint32_t argc, char** argv)
{
#ifdef OVER_TIZEN_VERSION_9
  for(uint32_t idx = 0; idx < argc; ++idx)
  {
    if(argv[idx])
    {
      if(strstr(argv[idx], "--prefer-updated-version"))
      {
        LWE::LWE::SetVersionPreference(true);
      }
      else if(strstr(argv[idx], "--prefer-platform-version"))
      {
        LWE::LWE::SetVersionPreference(false);
      }
    }
  }
#endif
  Create(width, height, Langset(), Timezone());
}

void TizenWebEngineLWE::Create(uint32_t width, uint32_t height, const std::string& locale, const std::string& timezoneId)
{
#ifndef OVER_TIZEN_VERSION_9
  mOutputWidth  = width;
  mOutputHeight = height;
  mOutputStride = width * sizeof(uint32_t);
  mOutputBuffer = (uint8_t*)malloc(width * height * sizeof(uint32_t));

  mOnRenderedHandler = [this](LWE::WebContainer* c, const LWE::WebContainer::RenderResult& renderResult)
  {
    size_t w = mOutputWidth;
    size_t h = mOutputHeight;
    if(renderResult.updatedWidth != w || renderResult.updatedHeight != h)
    {
      return;
    }
    Locker   l(mOutputBufferMutex);
    uint8_t* dstBuffer;
    size_t   dstStride;

    tbm_surface_info_s tbmSurfaceInfo;
    if(tbm_surface_map(mTbmSurface, TBM_SURF_OPTION_READ | TBM_SURF_OPTION_WRITE, &tbmSurfaceInfo) != TBM_SURFACE_ERROR_NONE)
    {
      DALI_LOG_ERROR("Fail to map tbm_surface\n");
    }

    DALI_ASSERT_ALWAYS(tbmSurfaceInfo.format == TBM_FORMAT_ARGB8888 && "Unsupported TizenWebEngineLWE tbm format");
    dstBuffer = tbmSurfaceInfo.planes[0].ptr;
    dstStride = tbmSurfaceInfo.planes[0].stride;

    uint32_t srcStride = static_cast<uint32_t>(renderResult.updatedWidth * sizeof(uint32_t));
    uint8_t* srcBuffer = static_cast<uint8_t*>(renderResult.updatedBufferAddress);

    if(dstStride == srcStride)
    {
      memcpy(dstBuffer, srcBuffer, tbmSurfaceInfo.planes[0].size);
    }
    else
    {
      for(auto y = renderResult.updatedY; y < (renderResult.updatedHeight + renderResult.updatedY); y++)
      {
        auto start = renderResult.updatedX;
        memcpy(dstBuffer + (y * dstStride) + (start * 4), srcBuffer + (y * srcStride) + (start * 4), srcStride);
      }
    }

    if(tbm_surface_unmap(mTbmSurface) != TBM_SURFACE_ERROR_NONE)
    {
      DALI_LOG_ERROR("Fail to unmap tbm_surface\n");
    }
    mUpdateBufferTrigger.Trigger();
  };
#endif
  mOnReceivedError = [](LWE::WebContainer* container, LWE::ResourceError error) {};

  mOnPageStartedHandler = [this](LWE::WebContainer* container, const std::string& url)
  {
    DALI_LOG_RELEASE_INFO("#LoadStarted : %s\n", url.c_str());
    ExecuteCallback(mLoadStartedCallback, url);
  };

  mOnPageFinishedHandler = [this](LWE::WebContainer* container, const std::string& url)
  {
    DALI_LOG_RELEASE_INFO("#LoadFinished : %s\n", url.c_str());
    ExecuteCallback(mLoadFinishedCallback, url);
  };

  mOnLoadResourceHandler = [](LWE::WebContainer* container, const std::string& url) {};

  mFirstRenderSignal.Connect(this, &TizenWebEngineLWE::OnFirstRender);

  InitRenderingContext();

  if(!LWE::LWE::IsInitialized())
  {
    std::string dataPath = DevelApplication::GetDataPath();
#ifdef OVER_TIZEN_VERSION_9
    LWE::LWE::Initialize((dataPath + "/StarFishStorage").c_str());
#else
    LWE::LWE::Initialize((dataPath + DB_NAME_LOCAL_STORAGE).c_str(),
                         (dataPath + DB_NAME_COOKIES).c_str(),
                         (dataPath + DB_NAME_CACHE).c_str());
#endif

    gEglCreateSyncKHR     = (PFNEGLCREATESYNCKHRPROC)eglGetProcAddress("eglCreateSyncKHR");
    gEglDestroySyncKHR    = (PFNEGLDESTROYSYNCKHRPROC)eglGetProcAddress("eglDestroySyncKHR");
    gEglClientWaitSyncKHR = (PFNEGLCLIENTWAITSYNCKHRPROC)eglGetProcAddress("eglClientWaitSyncKHR");
  }

#ifdef OVER_TIZEN_VERSION_9
  LWE::WebContainer::WebContainerArguments args{
    .width            = static_cast<unsigned>(width),
    .height           = static_cast<unsigned>(height),
    .devicePixelRatio = 1.0,
    .defaultFontName  = "serif",
    .locale           = locale.data(),
    .timezoneID       = timezoneId.data(),
  };

  LWE::WebContainer::RendererGLConfiguration config;
  config.onMakeCurrent = [&](LWE::WebContainer*)
  {
    if(!eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext))
    {
      DALI_LOG_ERROR("TizenWebEngineLWE: eglMakeCurrent error %d", (int)eglGetError());
    }
  };
  config.onSwapBuffers = [this](LWE::WebContainer*, bool mayNeedsSync)
  {
    if(!eglSwapBuffers(mEglDisplay, mEglSurface))
    {
      DALI_LOG_ERROR("TizenWebEngineLWE: eglSwapBuffers error %d", (int)eglGetError());
    }
    mEglSync = gEglCreateSyncKHR(mEglDisplay, EGL_SYNC_FENCE_KHR, NULL);
    eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    mLWERenderingRequested = false;
    TryUpdateImage(mayNeedsSync);
  };

  config.onCreateSharedContext = [this](LWE::WebContainer*) -> uintptr_t
  {
    EGLint     attributes[]  = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_NONE};
    EGLContext sharedContext = eglCreateContext(mEglDisplay, mEglConfig, mEglContext, attributes);
    return reinterpret_cast<uintptr_t>(sharedContext);
  };
  config.onDestroyContext = [this](LWE::WebContainer*, uintptr_t context) -> bool
  {
    return eglDestroyContext(mEglDisplay, reinterpret_cast<EGLContext>(context));
  };
  config.onClearCurrentContext = [this](LWE::WebContainer*) -> bool
  {
    return eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  };
  config.onMakeCurrentWithContext = [this](LWE::WebContainer*, uintptr_t context) -> bool
  {
    return eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, reinterpret_cast<EGLContext>(context));
  };
  config.onGetProcAddress = [this](LWE::WebContainer*, const char* name) -> void*
  {
    return reinterpret_cast<void*>(eglGetProcAddress(name));
  };
  config.onIsSupportedExtension = [this](LWE::WebContainer*, const char* name) -> bool
  {
    const char* extensions = eglQueryString(eglGetCurrentDisplay(), EGL_EXTENSIONS);
    return (extensions != nullptr) ? (strstr(extensions, name) != nullptr) : false;
  };

  mWebContainer = LWE::WebContainer::CreateGL(args, config);

  mWebContainer->RegisterSetNeedsRenderingCallback(
    [this](LWE::WebContainer*, const std::function<void()>& doRenderingFunction)
  {
    if(!mLWERenderingFunction)
    {
      mLWERenderingFunction = doRenderingFunction;
    }

    if(!mLWERenderingRequested.exchange(true))
    {
      PrepareLWERendering();
    }
  });

  mWebContainer->RegisterOnIdleHandler(
    [this](LWE::WebContainer*)
  {
    OnIdle();
  });

  auto settings = mWebContainer->GetSettings();
  settings.SetWebSecurityMode(LWE::WebSecurityMode::Disable);
  mWebContainer->SetSettings(settings);

  mWebContainer->LoadURL("about:blank");
#else
  mWebContainer = LWE::WebContainer::Create(mOutputWidth, mOutputHeight, 1.0, "", locale.data(), timezoneId.data());

  mWebContainer->RegisterPreRenderingHandler(
    [this]() -> LWE::WebContainer::RenderInfo
  {
    if(mOutputBuffer == NULL)
    {
      mOutputBuffer = (uint8_t*)malloc(mOutputWidth * mOutputHeight * sizeof(uint32_t));
      mOutputStride = mOutputWidth * sizeof(uint32_t);
    }

    ::LWE::WebContainer::RenderInfo result;
    result.updatedBufferAddress = mOutputBuffer;
    result.bufferStride         = mOutputStride;

    return result;
  });

  mWebContainer->RegisterOnRenderedHandler(
    [this](LWE::WebContainer* container, const LWE::WebContainer::RenderResult& renderResult)
  {
    mOnRenderedHandler(container, renderResult);
  });
#endif

  mWebContainer->RegisterOnReceivedErrorHandler(
    [this](LWE::WebContainer* container, LWE::ResourceError error)
  {
    mCanGoBack    = container->CanGoBack();
    mCanGoForward = container->CanGoForward();
    mOnReceivedError(container, error);
  });
  mWebContainer->RegisterOnPageStartedHandler(
    [this](LWE::WebContainer* container, const std::string& url)
  {
    mUrl          = url;
    mCanGoBack    = container->CanGoBack();
    mCanGoForward = container->CanGoForward();
    mOnPageStartedHandler(container, url);
  });
  mWebContainer->RegisterOnPageLoadedHandler(
    [this](LWE::WebContainer* container, const std::string& url)
  {
    mUrl          = url;
    mCanGoBack    = container->CanGoBack();
    mCanGoForward = container->CanGoForward();
    mOnPageFinishedHandler(container, url);
  });
  mWebContainer->RegisterOnLoadResourceHandler(
    [this](LWE::WebContainer* container, const std::string& url)
  {
    mUrl          = url;
    mCanGoBack    = container->CanGoBack();
    mCanGoForward = container->CanGoForward();
    mOnLoadResourceHandler(container, url);
  });
}

void TizenWebEngineLWE::TryRendering()
{
  if(mInDestroyingLWEInstance)
  {
    return;
  }

  if(mTbmQueue)
  {
    if((size_t)tbm_surface_queue_get_width(mTbmQueue) != mWebContainer->Width() ||
       (size_t)tbm_surface_queue_get_height(mTbmQueue) != mWebContainer->Height())
    {
      DALI_LOG_DEBUG_INFO("TizenWebEngineLWE: resize rendering surface");
      DestroyRenderingSurface();
      InitRenderingSurface();
    }
  }
  else
  {
    InitRenderingSurface();
  }

  OnActive();

  // Some devices needs short delay.
  unsigned           waitCount    = 0;
  constexpr unsigned maxWaitCount = 10; // 1ms
  while(!tbm_surface_queue_can_dequeue(mTbmQueue, 0) && waitCount < maxWaitCount)
  {
    usleep(100); // sleep 0.1ms
    waitCount++;
  }

  if(tbm_surface_queue_can_dequeue(mTbmQueue, 0))
  {
    mLWERenderingFunction();
  }
  else
  {
    mWebContainer->AddIdleCallback([](void* data)
    {
      TizenWebEngineLWE* lv = (TizenWebEngineLWE*)data;
      lv->TryRendering(); },
                                   this);
  }
}

void TizenWebEngineLWE::TryUpdateImage(bool needsSync)
{
  if(mInDestroyingLWEInstance)
  {
    return;
  }

  mInImageUpdateState = true;
  if(!eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext))
  {
    DALI_LOG_ERROR("TizenWebEngineLWE: eglMakeCurrent error %d", (int)eglGetError());
  }

  if(mEglSync)
  {
    auto checkState = gEglClientWaitSyncKHR(mEglDisplay, mEglSync, 0, needsSync ? EGL_FOREVER_KHR : 1000 * 1000);
    if(checkState == EGL_TIMEOUT_EXPIRED_KHR)
    {
      eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
      // Still busy
      mWebContainer->AddIdleCallback([](void* data)
      {
        TizenWebEngineLWE* lv = (TizenWebEngineLWE*)data;
        lv->TryUpdateImage(false); },
                                     this);
      return;
    }
    gEglDestroySyncKHR(mEglDisplay, mEglSync);
    mEglSync = nullptr;
  }

  eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

  // Some devices needs short delay.
  unsigned           waitCount    = 0;
  constexpr unsigned maxWaitCount = 10; // 1ms
  while(!tbm_surface_queue_can_dequeue(mTbmQueue, 0) && waitCount < maxWaitCount)
  {
    usleep(100); // sleep 0.1ms
    waitCount++;
  }

  bool uploadImageCalled = false;

  if(tbm_surface_queue_can_acquire(mTbmQueue, 0))
  {
    if(!mFirstRenderEnded)
    {
      mFirstRenderEnded = true;
      mFirstRenderSignal.Emit();
    }

    if(mLastDrawnTbmSurface)
    {
      tbm_surface_queue_release(mTbmQueue, mLastDrawnTbmSurface);
      mLastDrawnTbmSurface = nullptr;
    }

    if(tbm_surface_queue_acquire(mTbmQueue, &mLastDrawnTbmSurface) != TBM_SURFACE_QUEUE_ERROR_NONE)
    {
      DALI_LOG_ERROR("TizenWebEngineLWE: tbm_surface_queue_acquire failed!\n");
    }
    else
    {
      uploadImageCalled = true;
      UpdateImage(mLastDrawnTbmSurface);
      mInImageUpdateState = false;
    }
  }

  if(!uploadImageCalled)
  {
    DALI_LOG_DEBUG_INFO("TizenWebEngineLWE: tbm_surface_queue_can_acquire == false, retry!");
    mWebContainer->AddIdleCallback([](void* data)
    {
      TizenWebEngineLWE* lv = (TizenWebEngineLWE*)data;
      lv->TryUpdateImage(false); },
                                   this);
  }
}

void TizenWebEngineLWE::PrepareLWERendering()
{
  if(mInImageUpdateState)
  {
    mWebContainer->AddIdleCallback([](void* data)
    {
      TizenWebEngineLWE* lv = (TizenWebEngineLWE*)data;
      lv->PrepareLWERendering(); },
                                   this);
    return;
  }
  mWebContainer->AddIdleCallback([](void* data)
  {
    TizenWebEngineLWE* lv = (TizenWebEngineLWE*)data;
    lv->TryRendering(); },
                                 this);
}

void TizenWebEngineLWE::Destroy()
{
  DestroyRenderingContext();

  if(!mWebContainer)
  {
    return;
  }

#ifndef OVER_TIZEN_VERSION_9
  if(mTbmSurface != NULL && tbm_surface_destroy(mTbmSurface) != TBM_SURFACE_ERROR_NONE)
  {
    DALI_LOG_ERROR("Failed to destroy tbm_surface\n");
  }
#endif

  DestroyInstance();
}

void TizenWebEngineLWE::InitRenderingContext()
{
  if(mNativeDisplay != NULL)
  {
    return;
  }

  mNativeDisplay = reinterpret_cast<EGLNativeDisplayType>(tbm_dummy_display_create());
  if(NULL == mNativeDisplay)
  {
    DALI_LOG_ERROR("TizenWebEngineLWE: mNativeDisplay NULL");
    exit(-1);
  }
  mEglDisplay = eglGetDisplay(mNativeDisplay);
  if(mEglDisplay == EGL_NO_DISPLAY && eglGetError() != EGL_SUCCESS)
  {
    DALI_LOG_ERROR("TizenWebEngineLWE: mEglDisplay NULL");
    exit(-1);
  }

  EGLBoolean ret = EGL_FALSE;
  ret            = eglInitialize(mEglDisplay, NULL, NULL);
  if(ret != EGL_TRUE && eglGetError() != EGL_SUCCESS)
  {
    DALI_LOG_ERROR("TizenWebEngineLWE: eglInitialize Failed");
    exit(-1);
  }

  EGLint numConfigs;
  ret = eglGetConfigs(mEglDisplay, NULL, 0, &numConfigs);
  if(ret != EGL_TRUE && eglGetError() != EGL_SUCCESS)
  {
    DALI_LOG_ERROR("TizenWebEngineLWE: eglGetConfigs Failed");
    exit(-1);
  }

  const EGLint EglConfAttribs[] =
    {
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_DEPTH_SIZE, 0, EGL_STENCIL_SIZE, 8, EGL_SAMPLE_BUFFERS, 1, // MSAA x4
      EGL_SAMPLES,
      4, // MSAA x4
      EGL_NONE};

  ret = eglChooseConfig(mEglDisplay, EglConfAttribs, &mEglConfig, 1, &numConfigs);
  if(ret != EGL_TRUE && numConfigs == 0)
  {
    DALI_LOG_ERROR("TizenWebEngineLWE: eglChooseConfig Failed");
    exit(-1);
  }

  const EGLint EglContextAttribs[] =
    {
      EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
  mEglContext = eglCreateContext(mEglDisplay, mEglConfig, EGL_NO_CONTEXT, EglContextAttribs);
  if(mEglContext == EGL_NO_CONTEXT)
  {
    DALI_LOG_ERROR("TizenWebEngineLWE: eglCreateContext Failed");
    exit(-1);
  }

  OnActive();
}

void TizenWebEngineLWE::DestroyRenderingContext()
{
  DestroyRenderingSurface();

  if(mEglDisplay != EGL_NO_DISPLAY)
  {
    if(mEglContext != EGL_NO_CONTEXT)
    {
      eglDestroyContext(mEglDisplay, mEglContext);
      mEglContext = EGL_NO_CONTEXT;
    }

    eglTerminate(mEglDisplay);
    mEglDisplay = EGL_NO_DISPLAY;
  }

  if(mNativeDisplay != NULL)
  {
    tbm_dummy_display_destroy(reinterpret_cast<tbm_dummy_display*>(mNativeDisplay));
    mNativeDisplay = NULL;
  }
}

void TizenWebEngineLWE::InitRenderingSurface()
{
  if(mEglSurface != EGL_NO_SURFACE)
  {
    return;
  }

  mTbmQueue = tbm_surface_queue_create(gTbmSurfaceQueueLength, std::max(mWebContainer->Width(), static_cast<size_t>(1u)), std::max(mWebContainer->Height(), static_cast<size_t>(1u)), TBM_FORMAT_BGRA8888, TBM_BO_DEFAULT);

  mEglSurface = eglCreateWindowSurface(mEglDisplay, mEglConfig, reinterpret_cast<EGLNativeWindowType>(mTbmQueue), NULL);
  if(mEglSurface == EGL_NO_SURFACE)
  {
    DALI_LOG_ERROR("TizenWebEngineLWE: eglCreateWindowSurface Failed %d", eglGetError());
    exit(-1);
  }
}

void TizenWebEngineLWE::DestroyRenderingSurface()
{
  eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

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

void TizenWebEngineLWE::OnIdle()
{
  if(mInIdleState)
  {
    return;
  }

  mInIdleState = true;

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

void TizenWebEngineLWE::OnActive()
{
  if(!mInIdleState)
  {
    return;
  }
  mInIdleState      = false;
  mFirstRenderEnded = false;

  InitRenderingSurface();
}

void TizenWebEngineLWE::OnFirstRender()
{
  if(mIdleTbmSurface)
  {
    tbm_surface_internal_unref(mIdleTbmSurface);
    mIdleTbmSurface = nullptr;
  }
}

#ifndef OVER_TIZEN_VERSION_9
void TizenWebEngineLWE::LegacyUpdateBuffer()
{
  UpdateImage(mTbmSurface);
}
#endif

void TizenWebEngineLWE::UpdateImage(tbm_surface_h image)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  if((int)mWebContainer->Width() == tbm_surface_get_width(image) ||
     (int)mWebContainer->Height() == tbm_surface_get_height(image))
  {
#ifndef OVER_TIZEN_VERSION_9
    {
      Locker l(mOutputBufferMutex);
#endif
      Any source(image);
      mDaliImageSrc->SetSource(source);
      Dali::Stage::GetCurrent().KeepRendering(0.0f);
#ifndef OVER_TIZEN_VERSION_9
    }
#endif

    if(mFrameRenderedCallback)
    {
      ExecuteCallback(mFrameRenderedCallback);
    }
  }
  else
  {
    DALI_LOG_DEBUG_INFO("TizenWebEngineLWE: Image size not matched with WebContainer size\n");
  }
}

// NOT IMPLEMENTED
class NullWebEngineSettings : public Dali::WebEngineSettings
{
public:
  void AllowMixedContents(bool allowed) override
  {
  }
  void EnableSpatialNavigation(bool enabled) override
  {
  }
  uint32_t GetDefaultFontSize() const override
  {
    return 0;
  }
  void SetDefaultFontSize(uint32_t size) override
  {
  }
  void EnableWebSecurity(bool enabled) override
  {
  }
  void EnableCacheBuilder(bool enabled) override
  {
  }
  void UseScrollbarThumbFocusNotifications(bool used) override
  {
  }
  void EnableDoNotTrack(bool enabled) override
  {
  }
  void AllowFileAccessFromExternalUrl(bool allowed) override
  {
  }
  bool IsJavaScriptEnabled() const override
  {
    return false;
  }
  void EnableJavaScript(bool enabled) override
  {
  }
  bool IsAutoFittingEnabled() const override
  {
    return false;
  }
  void EnableAutoFitting(bool enabled) override
  {
  }
  bool ArePluginsEnabled() const override
  {
    return false;
  }
  void EnablePlugins(bool enabled) override
  {
  }
  bool IsPrivateBrowsingEnabled() const override
  {
    return false;
  }
  void EnablePrivateBrowsing(bool enabled) override
  {
  }
  bool IsLinkMagnifierEnabled() const override
  {
    return false;
  }
  void EnableLinkMagnifier(bool enabled) override
  {
  }
  bool IsKeypadWithoutUserActionUsed() const override
  {
    return false;
  }
  void UseKeypadWithoutUserAction(bool used) override
  {
  }
  bool IsAutofillPasswordFormEnabled() const override
  {
    return false;
  }
  void EnableAutofillPasswordForm(bool enabled) override
  {
  }
  bool IsFormCandidateDataEnabled() const override
  {
    return false;
  }
  void EnableFormCandidateData(bool enabled) override
  {
  }
  bool IsTextSelectionEnabled() const override
  {
    return false;
  }
  void EnableTextSelection(bool enabled) override
  {
  }
  bool IsTextAutosizingEnabled() const override
  {
    return false;
  }
  void EnableTextAutosizing(bool enabled) override
  {
  }
  bool IsArrowScrollEnabled() const override
  {
    return false;
  }
  void EnableArrowScroll(bool enabled) override
  {
  }
  bool IsClipboardEnabled() const override
  {
    return false;
  }
  void EnableClipboard(bool enabled) override
  {
  }
  bool IsImePanelEnabled() const override
  {
    return false;
  }
  void EnableImePanel(bool enabled) override
  {
  }
  void AllowScriptsOpenWindows(bool allowed) override
  {
  }
  bool AreImagesLoadedAutomatically() const override
  {
    return false;
  }
  void AllowImagesLoadAutomatically(bool automatic) override
  {
  }
  std::string GetDefaultTextEncodingName() const override
  {
    return std::string();
  }
  void SetDefaultTextEncodingName(const std::string& defaultTextEncodingName) override
  {
  }
  bool SetViewportMetaTag(bool enable) override
  {
    return false;
  }
  bool SetForceZoom(bool enable) override
  {
    return false;
  }
  bool IsZoomForced() const override
  {
    return false;
  }
  bool SetTextZoomEnabled(bool enable) override
  {
    return false;
  }
  bool IsTextZoomEnabled() const override
  {
    return false;
  }
  void SetExtraFeature(const std::string& feature, bool enable) override
  {
  }
  bool IsExtraFeatureEnabled(const std::string& feature) const override
  {
    return false;
  }
  void SetImeStyle(int style) override
  {
  }
  int GetImeStyle() const override
  {
    return 0;
  }
  void SetDefaultAudioInputDevice(const std::string& deviceId) const override
  {
    // NOT IMPLEMENTED
  }
  void EnableDragAndDrop(bool enable) override
  {
    // NOT IMPLEMENTED
  }
};

Dali::WebEngineSettings& TizenWebEngineLWE::GetSettings() const
{
  // NOT IMPLEMENTED
  static NullWebEngineSettings settings;
  return settings;
}

// NOT IMPLEMENTED
class NullWebEngineBackForwardListItem : public Dali::WebEngineBackForwardListItem
{
public:
  std::string GetUrl() const override
  {
    return std::string();
  }
  std::string GetTitle() const override
  {
    return std::string();
  }
  std::string GetOriginalUrl() const override
  {
    return std::string();
  }
};

// NOT IMPLEMENTED
class NullWebEngineBackForwardList : public Dali::WebEngineBackForwardList
{
public:
  NullWebEngineBackForwardList(WebEngineBackForwardListItem* pItem)
  : mItem(pItem)
  {
  }
  std::unique_ptr<Dali::WebEngineBackForwardListItem> GetCurrentItem() const override
  {
    return NULL;
  }
  std::unique_ptr<Dali::WebEngineBackForwardListItem> GetPreviousItem() const override
  {
    return NULL;
  }
  std::unique_ptr<Dali::WebEngineBackForwardListItem> GetNextItem() const override
  {
    return NULL;
  }
  std::unique_ptr<Dali::WebEngineBackForwardListItem> GetItemAtIndex(uint32_t index) const override
  {
    return NULL;
  }
  uint32_t GetItemCount() const override
  {
    return 1;
  }
  std::vector<std::unique_ptr<Dali::WebEngineBackForwardListItem>> GetBackwardItems(int limit)
  {
    std::vector<std::unique_ptr<Dali::WebEngineBackForwardListItem>> vec;
    return vec;
  }
  std::vector<std::unique_ptr<Dali::WebEngineBackForwardListItem>> GetForwardItems(int limit)
  {
    std::vector<std::unique_ptr<Dali::WebEngineBackForwardListItem>> vec;
    return vec;
  }

private:
  WebEngineBackForwardListItem* mItem;
};

Dali::WebEngineBackForwardList& TizenWebEngineLWE::GetBackForwardList() const
{
  // NOT IMPLEMENTED
  static NullWebEngineBackForwardListItem item;
  static NullWebEngineBackForwardList     list(&item);
  return list;
}

void TizenWebEngineLWE::DestroyInstance()
{
  if(!mWebContainer)
  {
    return;
  }
  mInDestroyingLWEInstance = true;
  mWebContainer->Destroy();
  mInDestroyingLWEInstance = false;
  mWebContainer            = NULL;
}

Dali::NativeImageSourcePtr TizenWebEngineLWE::GetNativeImageSource()
{
  return mDaliImageSrc;
}

void TizenWebEngineLWE::ChangeOrientation(int orientation)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::LoadUrl(const std::string& url)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->LoadURL(url);
}

std::string TizenWebEngineLWE::GetTitle() const
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  return mWebContainer->GetTitle();
}

Dali::PixelData TizenWebEngineLWE::GetFavicon() const
{
  // NOT IMPLEMENTED
  return Dali::PixelData();
}

std::string TizenWebEngineLWE::GetUrl() const
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  return mUrl;
}

void TizenWebEngineLWE::LoadHtmlString(const std::string& str)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->LoadData(str);
}

bool TizenWebEngineLWE::LoadHtmlStringOverrideCurrentEntry(const std::string& html, const std::string& basicUri, const std::string& unreachableUrl)
{
  // NOT IMPLEMENTED
  return false;
}

bool TizenWebEngineLWE::LoadContents(const int8_t* contents, uint32_t contentSize, const std::string& mimeType, const std::string& encoding, const std::string& baseUri)
{
  // NOT IMPLEMENTED
  return false;
}

void TizenWebEngineLWE::Reload()
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->Reload();
}

bool TizenWebEngineLWE::ReloadWithoutCache()
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->ClearCache();
  mWebContainer->Reload();
  return true;
}

void TizenWebEngineLWE::StopLoading()
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->StopLoading();
}

void TizenWebEngineLWE::Suspend()
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::Resume()
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::SuspendNetworkLoading()
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::ResumeNetworkLoading()
{
  // NOT IMPLEMENTED
}

bool TizenWebEngineLWE::AddCustomHeader(const std::string& name, const std::string& value)
{
  // NOT IMPLEMENTED
  return false;
}

bool TizenWebEngineLWE::RemoveCustomHeader(const std::string& name)
{
  // NOT IMPLEMENTED
  return false;
}

uint32_t TizenWebEngineLWE::StartInspectorServer(uint32_t port)
{
  // NOT IMPLEMENTED
  return 0;
}

bool TizenWebEngineLWE::StopInspectorServer()
{
  // NOT IMPLEMENTED
  return false;
}

void TizenWebEngineLWE::ScrollBy(int32_t deltaX, int32_t deltaY)
{
  // NOT IMPLEMENTED
}

bool TizenWebEngineLWE::ScrollEdgeBy(int32_t deltaX, int32_t deltaY)
{
  // NOT IMPLEMENTED
  return false;
}

void TizenWebEngineLWE::SetScrollPosition(int32_t x, int32_t y)
{
  // NOT IMPLEMENTED
}

Dali::Vector2 TizenWebEngineLWE::GetScrollPosition() const
{
  // NOT IMPLEMENTED
  return Dali::Vector2::ZERO;
}

Dali::Vector2 TizenWebEngineLWE::GetScrollSize() const
{
  // NOT IMPLEMENTED
  return Dali::Vector2::ZERO;
}

Dali::Vector2 TizenWebEngineLWE::GetContentSize() const
{
  // NOT IMPLEMENTED
  return Dali::Vector2::ZERO;
}

void TizenWebEngineLWE::GoBack()
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->GoBack();
}

void TizenWebEngineLWE::GoForward()
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->GoForward();
}

bool TizenWebEngineLWE::CanGoBack()
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  return mCanGoBack;
}

bool TizenWebEngineLWE::CanGoForward()
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  return mCanGoForward;
}

void TizenWebEngineLWE::EvaluateJavaScript(const std::string& script, std::function<void(const std::string&)> resultHandler)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  // LWE don't support empty std::function
  if(!resultHandler)
  {
    resultHandler = [](const std::string&) {};
  }
  mWebContainer->EvaluateJavaScript(script, resultHandler);
}

void TizenWebEngineLWE::AddJavaScriptMessageHandler(const std::string& exposedObjectName, std::function<void(const std::string&)> handler)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  // LWE don't support empty std::function
  if(!handler)
  {
    handler = [](const std::string&) {};
  }
  mWebContainer->AddJavaScriptInterface(exposedObjectName, "postMessage", [handler](const std::string& data) -> std::string
  {
    handler(data);
    return ""; });
}

void TizenWebEngineLWE::AddJavaScriptEntireMessageHandler(const std::string& exposedObjectName, Dali::WebEnginePlugin::JavaScriptEntireMessageHandlerCallback handler)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterJavaScriptAlertCallback(Dali::WebEnginePlugin::JavaScriptAlertCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::JavaScriptAlertReply()
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterJavaScriptConfirmCallback(Dali::WebEnginePlugin::JavaScriptConfirmCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::JavaScriptConfirmReply(bool confirmed)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterJavaScriptPromptCallback(Dali::WebEnginePlugin::JavaScriptPromptCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::JavaScriptPromptReply(const std::string& result)
{
  // NOT IMPLEMENTED
}

std::unique_ptr<Dali::WebEngineHitTest> TizenWebEngineLWE::CreateHitTest(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode)
{
  // NOT IMPLEMENTED
  return nullptr;
}

bool TizenWebEngineLWE::CreateHitTestAsynchronously(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode, Dali::WebEnginePlugin::WebEngineHitTestCreatedCallback callback)
{
  // NOT IMPLEMENTED
  return false;
}

void TizenWebEngineLWE::ClearHistory()
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->ClearHistory();
  mCanGoBack = mWebContainer->CanGoBack();
}

void TizenWebEngineLWE::ClearAllTilesResources()
{
  // NOT IMPLEMENTED
}

std::string TizenWebEngineLWE::GetUserAgent() const
{
  // NOT IMPLEMENTED
  return std::string();
}

void TizenWebEngineLWE::SetUserAgent(const std::string& userAgent)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::EnableMouseEvents(bool enabled)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::EnableKeyEvents(bool enabled)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::SetSize(uint32_t width, uint32_t height)
{
  DALI_ASSERT_ALWAYS(mWebContainer);

  if(mWebContainer->Width() != width || mWebContainer->Height() != height)
  {
    mWebContainer->ResizeTo(width, height);
  }

#ifndef OVER_TIZEN_VERSION_9
  if(mOutputWidth != (size_t)width || mOutputHeight != (size_t)height)
  {
    mOutputWidth  = width;
    mOutputHeight = height;
    mOutputStride = width * sizeof(uint32_t);

    tbm_surface_h prevTbmSurface = mTbmSurface;
    mTbmSurface                  = tbm_surface_create(width, height, TBM_FORMAT_ARGB8888);
    Dali::Any source(mTbmSurface);
    mDaliImageSrc->SetSource(source);
    if(prevTbmSurface != NULL && tbm_surface_destroy(prevTbmSurface) != TBM_SURFACE_ERROR_NONE)
    {
      DALI_LOG_ERROR("Failed to destroy tbm_surface\n");
    }

    auto oldOutputBuffer = mOutputBuffer;
    mOutputBuffer        = (uint8_t*)malloc(mOutputWidth * mOutputHeight * sizeof(uint32_t));
    mOutputStride        = mOutputWidth * sizeof(uint32_t);
    mWebContainer->ResizeTo(mOutputWidth, mOutputHeight);

    if(oldOutputBuffer)
    {
      free(oldOutputBuffer);
    }
  }
#endif
}

void TizenWebEngineLWE::SetDocumentBackgroundColor(Dali::Vector4 color)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::ClearTilesWhenHidden(bool cleared)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::SetTileCoverAreaMultiplier(float multiplier)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::EnableCursorByClient(bool enabled)
{
  // NOT IMPLEMENTED
}

std::string TizenWebEngineLWE::GetSelectedText() const
{
  // NOT IMPLEMENTED
  return std::string();
}

void TizenWebEngineLWE::DispatchMouseDownEvent(float x, float y)
{
  DALI_ASSERT_ALWAYS(mWebContainer);

  mWebContainer->DispatchMouseDownEvent(LWE::MouseButtonValue::LeftButton, LWE::MouseButtonsValue::LeftButtonDown, x, y);
}

void TizenWebEngineLWE::DispatchMouseUpEvent(float x, float y)
{
  DALI_ASSERT_ALWAYS(mWebContainer);

  mWebContainer->DispatchMouseUpEvent(LWE::MouseButtonValue::NoButton, LWE::MouseButtonsValue::NoButtonDown, x, y);
}

void TizenWebEngineLWE::DispatchMouseMoveEvent(float x, float y, bool isLButtonPressed, bool isRButtonPressed)
{
  DALI_ASSERT_ALWAYS(mWebContainer);

  mWebContainer->DispatchMouseMoveEvent(
    isLButtonPressed ? LWE::MouseButtonValue::LeftButton
                     : LWE::MouseButtonValue::NoButton,
    isLButtonPressed ? LWE::MouseButtonsValue::LeftButtonDown
                     : LWE::MouseButtonsValue::NoButtonDown,
    x,
    y);
}

bool TizenWebEngineLWE::SendTouchEvent(const TouchEvent& touch)
{
  size_t pointCount = touch.GetPointCount();
  if(pointCount == 1)
  {
    // Single touch event
    Dali::PointState::Type pointState = touch.GetState(0);
    const Dali::Vector2&   screen     = touch.GetLocalPosition(0);

    if(pointState == Dali::PointState::DOWN)
    {
      DispatchMouseDownEvent(screen.x, screen.y);
      mIsMouseLbuttonDown = true;
    }
    else if(pointState == Dali::PointState::UP)
    {
      DispatchMouseUpEvent(screen.x, screen.y);
      mIsMouseLbuttonDown = false;
    }
    else
    {
      DispatchMouseMoveEvent(screen.x, screen.y, mIsMouseLbuttonDown, false);
    }
  }

  return false;
}

void TizenWebEngineLWE::DispatchKeyDownEvent(LWE::KeyValue keyCode)
{
  DALI_ASSERT_ALWAYS(mWebContainer);

  mWebContainer->DispatchKeyDownEvent(keyCode);
}

void TizenWebEngineLWE::DispatchKeyPressEvent(LWE::KeyValue keyCode)
{
  DALI_ASSERT_ALWAYS(mWebContainer);

  mWebContainer->DispatchKeyPressEvent(keyCode);
}

void TizenWebEngineLWE::DispatchKeyUpEvent(LWE::KeyValue keyCode)
{
  DALI_ASSERT_ALWAYS(mWebContainer);

  mWebContainer->DispatchKeyUpEvent(keyCode);
}

bool TizenWebEngineLWE::SendKeyEvent(const Dali::KeyEvent& event)
{
  LWE::KeyValue keyValue = LWE::KeyValue::UnidentifiedKey;
  if(32 < event.GetKeyString().c_str()[0] && 127 > event.GetKeyString().c_str()[0])
  {
    keyValue = static_cast<LWE::KeyValue>(event.GetKeyString().c_str()[0]);
  }
  else
  {
    keyValue = KeyStringToKeyValue(event.GetKeyName().c_str(), event.GetKeyModifier() & 1);
  }
  if(event.GetState() == Dali::KeyEvent::DOWN)
  {
    DispatchKeyDownEvent(keyValue);
    DispatchKeyPressEvent(keyValue);
  }
  else if(event.GetState() == Dali::KeyEvent::UP)
  {
    DispatchKeyUpEvent(keyValue);
  }

  return false;
}

void TizenWebEngineLWE::SetFocus(bool focused)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  if(focused)
  {
    mWebContainer->Focus();
  }
  else
  {
    mWebContainer->Blur();
  }
}

void TizenWebEngineLWE::UpdateDisplayArea(Dali::Rect<int32_t> displayArea)
{
  mDaliImageSrc = NativeImageSource::New(0, 0, NativeImageSource::COLOR_DEPTH_DEFAULT);
  SetSize(displayArea.width, displayArea.height);
}

bool TizenWebEngineLWE::SetImePositionAndAlignment(Dali::Vector2 position, int alignment)
{
  // NOT IMPLEMENTED
  return false;
}

void TizenWebEngineLWE::SetCursorThemeName(const std::string themeName)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::SetPageZoomFactor(float zoomFactor)
{
  // NOT IMPLEMENTED
}

float TizenWebEngineLWE::GetPageZoomFactor() const
{
  // NOT IMPLEMENTED
  return 0.0f;
}

void TizenWebEngineLWE::SetTextZoomFactor(float zoomFactor)
{
  // NOT IMPLEMENTED
}

float TizenWebEngineLWE::GetTextZoomFactor() const
{
  // NOT IMPLEMENTED
  return 0.0f;
}

float TizenWebEngineLWE::GetLoadProgressPercentage() const
{
  // NOT IMPLEMENTED
  return 0.0f;
}

void TizenWebEngineLWE::SetScaleFactor(float scaleFactor, Dali::Vector2 point)
{
  // NOT IMPLEMENTED
}

float TizenWebEngineLWE::GetScaleFactor() const
{
  // NOT IMPLEMENTED
  return 0.0f;
}

void TizenWebEngineLWE::ActivateAccessibility(bool activated)
{
  // NOT IMPLEMENTED
}

Accessibility::Address TizenWebEngineLWE::GetAccessibilityAddress()
{
  // NOT IMPLEMENTED
  return {};
}

bool TizenWebEngineLWE::SetVisibility(bool visible)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  if(visible)
  {
    mWebContainer->Resume();
  }
  else
  {
    mWebContainer->Pause();
  }
  return true;
}

bool TizenWebEngineLWE::HighlightText(const std::string& text, Dali::WebEnginePlugin::FindOption options, uint32_t maxMatchCount)
{
  // NOT IMPLEMENTED
  return false;
}

void TizenWebEngineLWE::AddDynamicCertificatePath(const std::string& host, const std::string& certPath)
{
  // NOT IMPLEMENTED
}

Dali::PixelData TizenWebEngineLWE::GetScreenshot(Dali::Rect<int32_t> viewArea, float scaleFactor)
{
  // NOT IMPLEMENTED
  return Dali::PixelData();
}

bool TizenWebEngineLWE::GetScreenshotAsynchronously(Dali::Rect<int32_t> viewArea, float scaleFactor, Dali::WebEnginePlugin::ScreenshotCapturedCallback callback)
{
  // NOT IMPLEMENTED
  return false;
}

bool TizenWebEngineLWE::CheckVideoPlayingAsynchronously(Dali::WebEnginePlugin::VideoPlayingCallback callback)
{
  // NOT IMPLEMENTED
  return false;
}

void TizenWebEngineLWE::RegisterGeolocationPermissionCallback(GeolocationPermissionCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::EnableVideoHole(bool enabled)
{
  // NOT IMPLEMENTED
}

bool TizenWebEngineLWE::SendHoverEvent(const Dali::HoverEvent& event)
{
  // NOT IMPLEMENTED
  return false;
}

bool TizenWebEngineLWE::SendWheelEvent(const Dali::WheelEvent& event)
{
  // NOT IMPLEMENTED
  return false;
}

void TizenWebEngineLWE::ExitFullscreen()
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterFrameRenderedCallback(WebEngineFrameRenderedCallback callback)
{
  mFrameRenderedCallback = callback;
}

void TizenWebEngineLWE::RegisterPageLoadStartedCallback(WebEnginePageLoadCallback callback)
{
  mLoadStartedCallback = callback;
}

void TizenWebEngineLWE::RegisterPageLoadInProgressCallback(WebEnginePageLoadCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterPageLoadFinishedCallback(WebEnginePageLoadCallback callback)
{
  mLoadFinishedCallback = callback;
}

void TizenWebEngineLWE::RegisterPageLoadErrorCallback(WebEnginePageLoadErrorCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterScrollEdgeReachedCallback(WebEngineScrollEdgeReachedCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterOverScrolledCallback(WebEngineOverScrolledCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterUrlChangedCallback(WebEngineUrlChangedCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterFormRepostDecidedCallback(WebEngineFormRepostDecidedCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterConsoleMessageReceivedCallback(WebEngineConsoleMessageReceivedCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterResponsePolicyDecidedCallback(WebEngineResponsePolicyDecidedCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterNavigationPolicyDecidedCallback(WebEngineNavigationPolicyDecidedCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterNewWindowPolicyDecidedCallback(WebEngineNewWindowPolicyDecidedCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterNewWindowCreatedCallback(WebEngineNewWindowCreatedCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterCertificateConfirmedCallback(WebEngineCertificateCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterSslCertificateChangedCallback(WebEngineCertificateCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterHttpAuthHandlerCallback(WebEngineHttpAuthHandlerCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterContextMenuShownCallback(WebEngineContextMenuShownCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterContextMenuHiddenCallback(WebEngineContextMenuHiddenCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterFullscreenEnteredCallback(WebEngineFullscreenEnteredCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterFullscreenExitedCallback(WebEngineFullscreenExitedCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterTextFoundCallback(WebEngineTextFoundCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::GetPlainTextAsynchronously(PlainTextReceivedCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterWebAuthDisplayQRCallback(WebEngineWebAuthDisplayQRCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterWebAuthResponseCallback(WebEngineWebAuthResponseCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterFileChooserRequestedCallback(WebEngineFileChooserRequestedCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::WebAuthenticationCancel()
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterUserMediaPermissionRequestCallback(WebEngineUserMediaPermissionRequestCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterDeviceConnectionChangedCallback(WebEngineDeviceConnectionChangedCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::RegisterDeviceListGetCallback(WebEngineDeviceListGetCallback callback)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::FeedMouseWheel(bool yDirection, int step, int x, int y)
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::SetVideoHole(bool enabled, bool isWaylandWindow)
{
  // NOT IMPLEMENTED
}

} // namespace Plugin
} // namespace Dali
