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

#include "web-engine-lwe-backend-ubuntu.h"
#include <LWEWebView.h>

#include <dali/devel-api/adaptor-framework/application-devel.h>
#include "web-engine-lwe-buffer-image.h"
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/adaptor-framework/native-image.h>
#include <dali/public-api/images/pixel.h>
#include <dali/public-api/signals/callback.h>

#include <algorithm>
#include <cstdint>
#include <utility>

// Xlib defines None and Status; include EGL after LWE and DALi headers.
#include "web-engine-lwe-egl-renderer.h"

namespace DALI_NAMESPACE
{
namespace Plugin
{
namespace
{
constexpr uint32_t SAFE_INITIAL_WEB_VIEW_WIDTH = 1920u;
constexpr uint32_t SAFE_INITIAL_WEB_VIEW_HEIGHT = 1080u;

// LWE's mask rendering allocates width * height * 4 byte CPU buffers. These
// limits keep malformed WebView geometry from exhausting the process before
// Starfish can render a frame. They still allow an 8K-by-8K WebView.
constexpr uint32_t MAX_WEB_VIEW_DIMENSION = 8192u;
constexpr uint64_t MAX_WEB_VIEW_PIXELS = 64ull * 1024ull * 1024ull;

bool isSupportedWebViewSize(uint32_t width, uint32_t height)
{
  if(width == 0u || height == 0u)
  {
    return false;
  }

  if(width > MAX_WEB_VIEW_DIMENSION || height > MAX_WEB_VIEW_DIMENSION)
  {
    return false;
  }

  return static_cast<uint64_t>(width) * height <= MAX_WEB_VIEW_PIXELS;
}
}

WebEngineLweBackendUbuntu::WebEngineLweBackendUbuntu()
: mWebContainer(nullptr),
  mReadyWidth(0u),
  mReadyHeight(0u),
  mFrameUploadPending(false),
  mImageWidth(0u),
  mImageHeight(0u),
  mAcceptTasks(false)
{
}

WebEngineLweBackendUbuntu::~WebEngineLweBackendUbuntu()
{
  Destroy();
}

void WebEngineLweBackendUbuntu::ensureInitialized()
{
  if(LWE::LWE::IsInitialized())
  {
    return;
  }

  std::string storagePath = Dali::DevelApplication::GetDataPath();
  if(storagePath.empty())
  {
    storagePath = ".";
  }
  if(storagePath.back() != '/' && storagePath.back() != '\\')
  {
    storagePath += '/';
  }
  storagePath += "StarFishStorage";

  // Isolate LWE's GLib context from DALi's event loop. Notify DALi through
  // EventThreadCallback; no DALi handle is mutated on the LWE thread.
  LWE::LWE::Initialize(storagePath.c_str(), LWE::InitializeOption::PreferSeparateThread);
}

LWE::WebContainer* WebEngineLweBackendUbuntu::Create(uint32_t width, uint32_t height, uint32_t, char**)
{
  return Create(width, height, "en-US", "UTC");
}

LWE::WebContainer* WebEngineLweBackendUbuntu::Create(uint32_t width,
  uint32_t height,
  const std::string& locale,
  const std::string& timezoneId)
{
  if(width > MAX_WEB_VIEW_DIMENSION || height > MAX_WEB_VIEW_DIMENSION ||
     static_cast<uint64_t>(width) * height > MAX_WEB_VIEW_PIXELS)
  {
    DALI_LOG_ERROR("WebEngineLwe: unsupported initial size %ux%u\n", width, height);
    return nullptr;
  }

  Destroy();
  // LWE can produce invalid mask geometry when content is loaded in a very
  // small viewport before layout supplies the WebView's final display area.
  const uint32_t initialWidth = std::max(width, SAFE_INITIAL_WEB_VIEW_WIDTH);
  const uint32_t initialHeight = std::max(height, SAFE_INITIAL_WEB_VIEW_HEIGHT);

  ensureInitialized();

  mTaskGeneration = std::make_shared<std::atomic_bool>(true);
  mAcceptTasks = true;
  mEventThreadCallback = std::make_shared<Dali::EventThreadCallback>(
    Dali::MakeCallback(this, &WebEngineLweBackendUbuntu::processEventTasks));

  mRenderer = std::make_unique<WebEngineLweEglRenderer>();
  mRenderer->setFrameCallback(
    [this](std::vector<uint8_t>&& pixels, uint32_t frameWidth, uint32_t frameHeight)
  {
    handleFrame(std::move(pixels), frameWidth, frameHeight);
  });
  if(!mRenderer->initialize(initialWidth, initialHeight))
  {
    Destroy();
    return nullptr;
  }

  WebEngineLweEglRenderer* renderer = mRenderer.get();
  LWE::WebContainer::RendererGLConfiguration configuration;
  configuration.onMakeCurrent = [renderer](LWE::WebContainer*)
  {
    // LWE's callback cannot report failure; do not let it draw with a stale
    // surface or another view's context after an EGL failure.
    DALI_ASSERT_ALWAYS(renderer->makeCurrent() && "Failed to make LWE EGL context current");
  };
  configuration.onSwapBuffers = [renderer](LWE::WebContainer*, bool)
  {
    if(!renderer->swapBuffers())
    {
      DALI_LOG_ERROR("WebEngineLwe: failed to capture EGL frame\n");
    }
  };
  configuration.onCreateSharedContext = [renderer](LWE::WebContainer*) -> uintptr_t
  {
    return renderer->createSharedContext();
  };
  configuration.onDestroyContext = [renderer](LWE::WebContainer*, uintptr_t context)
  {
    return renderer->destroyContext(context);
  };
  configuration.onClearCurrentContext = [renderer](LWE::WebContainer*)
  {
    return renderer->clearCurrentContext();
  };
  configuration.onMakeCurrentWithContext = [renderer](LWE::WebContainer*, uintptr_t context)
  {
    return renderer->makeCurrentWithContext(context);
  };
  configuration.onGetProcAddress = [renderer](LWE::WebContainer*, const char* name)
  {
    return renderer->getProcAddress(name);
  };
  configuration.onIsSupportedExtension = [renderer](LWE::WebContainer*, const char* extension)
  {
    return renderer->isSupportedExtension(extension);
  };

  const LWE::WebContainer::WebContainerArguments arguments = {
    initialWidth,
    initialHeight,
    1.0f,
    "sans-serif",
    locale.c_str(),
    timezoneId.c_str()};

  mWebContainer = LWE::WebContainer::CreateGL(arguments, configuration);
  if(!mWebContainer)
  {
    DALI_LOG_ERROR("WebEngineLwe: failed to create OpenGL WebContainer\n");
    Destroy();
    return nullptr;
  }

  return mWebContainer;
}

void WebEngineLweBackendUbuntu::handleFrame(std::vector<uint8_t>&& pixels, uint32_t width, uint32_t height)
{
  bool requestUpload = false;
  {
    std::lock_guard<std::mutex> lock(mFrameMutex);
    mReadyBuffer = std::move(pixels);
    mReadyWidth = width;
    mReadyHeight = height;
    if(!mFrameUploadPending)
    {
      mFrameUploadPending = true;
      requestUpload = true;
    }
  }
  if(requestUpload)
  {
    DispatchToEventThread([this]()
    {
      uploadFrame();
    });
  }
}

void WebEngineLweBackendUbuntu::Destroy()
{
  mAcceptTasks = false;
  if(mTaskGeneration)
  {
    *mTaskGeneration = false;
  }
  if(mWebContainer)
  {
    mWebContainer->Destroy();
    mWebContainer = nullptr;
  }
  mRenderer.reset();

  {
    std::lock_guard<std::mutex> lock(mTaskMutex);
    mTasks.clear();
    mEventThreadCallback.reset();
  }

  {
    std::lock_guard<std::mutex> lock(mFrameMutex);
    mReadyBuffer.clear();
    mReadyWidth = 0u;
    mReadyHeight = 0u;
    mFrameUploadPending = false;
  }
  mNativeImage.Reset();
  mImageWidth = 0u;
  mImageHeight = 0u;
}

void WebEngineLweBackendUbuntu::SetSize(uint32_t width, uint32_t height)
{
  if(!isSupportedWebViewSize(width, height))
  {
    DALI_LOG_ERROR("WebEngineLwe: ignored unsupported resize %ux%u (maximum %ux%u, %llu pixels)\n",
                   width,
                   height,
                   MAX_WEB_VIEW_DIMENSION,
                   MAX_WEB_VIEW_DIMENSION,
                   static_cast<unsigned long long>(MAX_WEB_VIEW_PIXELS));
    return;
  }

  if(mRenderer)
  {
    mRenderer->resize(width, height);
  }
  if(mWebContainer)
  {
    mWebContainer->ResizeTo(width, height);
  }
}

void WebEngineLweBackendUbuntu::UpdateDisplayArea(uint32_t width, uint32_t height)
{
  SetSize(width, height);
}

Dali::NativeImagePtr WebEngineLweBackendUbuntu::GetNativeImage()
{
  return {};
}

Dali::NativeImageInterfacePtr WebEngineLweBackendUbuntu::GetNativeImageInterface()
{
  return mNativeImage;
}

void WebEngineLweBackendUbuntu::SetFrameRenderedCallback(FrameRenderedCallback callback)
{
  mFrameRenderedCallback = std::move(callback);
}

void WebEngineLweBackendUbuntu::DispatchToEventThread(Task task)
{
  if(!task || !mAcceptTasks)
  {
    return;
  }

  std::shared_ptr<Dali::EventThreadCallback> eventThreadCallback;
  {
    std::lock_guard<std::mutex> lock(mTaskMutex);
    if(!mAcceptTasks)
    {
      return;
    }
    mTasks.emplace_back(std::move(task));
    eventThreadCallback = mEventThreadCallback;
  }
  if(eventThreadCallback)
  {
    eventThreadCallback->Trigger();
  }
}

void WebEngineLweBackendUbuntu::processEventTasks()
{
  // A callback may destroy or recreate the owning WebView. Keep only local
  // state after invoking it, and cancel the remaining tasks of that generation.
  auto generation = mTaskGeneration;
  auto eventThreadCallback = mEventThreadCallback;
  std::deque<Task> tasks;
  {
    std::lock_guard<std::mutex> lock(mTaskMutex);
    tasks.swap(mTasks);
  }

  for(auto& task : tasks)
  {
    if(!generation || !*generation)
    {
      break;
    }
    if(task)
    {
      task();
    }
  }
}

void WebEngineLweBackendUbuntu::uploadFrame()
{
  std::vector<uint8_t> pixels;
  uint32_t width = 0u;
  uint32_t height = 0u;
  {
    std::lock_guard<std::mutex> lock(mFrameMutex);
    width = mReadyWidth;
    height = mReadyHeight;
    pixels.swap(mReadyBuffer);
    mFrameUploadPending = false;
  }

  if(width == 0u || height == 0u || pixels.size() != static_cast<size_t>(width) * height * BYTES_PER_PIXEL)
  {
    return;
  }

  if(!mNativeImage || mImageWidth != width || mImageHeight != height)
  {
    mNativeImage = new WebEngineLweBufferImage(width, height);
    mImageWidth = width;
    mImageHeight = height;
  }

  static_cast<WebEngineLweBufferImage*>(mNativeImage.Get())->setFrame(std::move(pixels));
  // Buffer changes do not enqueue a scene update. Wake DALi before notifying
  // the consumer, including when it reuses its existing native-image visual.
  if(Dali::Adaptor::IsAvailable())
  {
    Dali::Adaptor::Get().RequestProcessEventsAndUpdate();
  }
  auto callback = mFrameRenderedCallback;
  if(callback)
  {
    callback();
  }
}

std::unique_ptr<WebEngineLweBackend> CreateWebEngineLweBackend()
{
  return std::make_unique<WebEngineLweBackendUbuntu>();
}

void ClearWebEngineLweCookies()
{
  if(LWE::LWE::IsInitialized())
  {
    if(auto* cookieManager = LWE::CookieManager::GetInstance())
    {
      cookieManager->ClearCookies();
    }
  }
}

} // namespace Plugin
} //namespace DALI_NAMESPACE
