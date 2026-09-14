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

#include "web-engine-lwe-backend-win.h"
#include "web-engine-lwe-angle-renderer.h"

#include <LWEWebView.h>

#include <dali/devel-api/adaptor-framework/application-devel.h>
#include <dali/devel-api/adaptor-framework/native-image-devel.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/adaptor-framework/native-image.h>
#include <dali/public-api/images/pixel.h>
#include <dali/public-api/signals/callback.h>

#include <utility>

namespace DALI_NAMESPACE
{
namespace Plugin
{
namespace
{
constexpr Dali::Pixel::Format RENDER_BUFFER_PIXEL_FORMAT = Dali::Pixel::RGBA8888;
}

WebEngineLweBackendWin::WebEngineLweBackendWin()
: mWebContainer(nullptr),
  mReadyWidth(0u),
  mReadyHeight(0u),
  mFrameUploadPending(false),
  mImageWidth(0u),
  mImageHeight(0u),
  mAcceptTasks(false)
{
}

WebEngineLweBackendWin::~WebEngineLweBackendWin()
{
  Destroy();
}

void WebEngineLweBackendWin::EnsureInitialized()
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

  // DALi's SDL loop and LWE's Win32 loop must not consume each other's
  // messages. LWE therefore owns a separate thread and all DALi work is
  // marshalled back through EventThreadCallback.
  LWE::LWE::Initialize(storagePath.c_str(), LWE::InitializeOption::PreferSeparateThread);
}

LWE::WebContainer* WebEngineLweBackendWin::Create(uint32_t width, uint32_t height, uint32_t, char**)
{
  return Create(width, height, "en-US", "UTC");
}

LWE::WebContainer* WebEngineLweBackendWin::Create(uint32_t width,
  uint32_t height,
  const std::string& locale,
  const std::string& timezoneId)
{
  EnsureInitialized();

  mAcceptTasks = true;
  mEventThreadCallback = std::make_shared<Dali::EventThreadCallback>(
    Dali::MakeCallback(this, &WebEngineLweBackendWin::ProcessEventTasks));

  mRenderer = std::make_unique<WebEngineLweAngleRenderer>();
  mRenderer->SetFrameCallback(
    [this](std::vector<uint8_t>&& pixels, uint32_t frameWidth, uint32_t frameHeight)
  {
    HandleFrame(std::move(pixels), frameWidth, frameHeight);
  });
  DALI_ASSERT_ALWAYS(mRenderer->Initialize(width, height) && "Failed to initialize LWE ANGLE renderer");

  WebEngineLweAngleRenderer* renderer = mRenderer.get();
  LWE::WebContainer::RendererGLConfiguration configuration;
  configuration.onMakeCurrent = [renderer](LWE::WebContainer*)
  {
    // LWE's callback cannot report failure; do not let it draw with a stale
    // surface or another view's context after an EGL failure.
    DALI_ASSERT_ALWAYS(renderer->MakeCurrent() && "Failed to make LWE ANGLE context current");
  };
  configuration.onSwapBuffers = [renderer](LWE::WebContainer*, bool)
  {
    if(!renderer->SwapBuffers())
    {
      DALI_LOG_ERROR("WebEngineLwe: failed to capture ANGLE frame\n");
    }
  };
  configuration.onCreateSharedContext = [renderer](LWE::WebContainer*) -> uintptr_t
  {
    return renderer->CreateSharedContext();
  };
  configuration.onDestroyContext = [renderer](LWE::WebContainer*, uintptr_t context)
  {
    return renderer->DestroyContext(context);
  };
  configuration.onClearCurrentContext = [renderer](LWE::WebContainer*)
  {
    return renderer->ClearCurrentContext();
  };
  configuration.onMakeCurrentWithContext = [renderer](LWE::WebContainer*, uintptr_t context)
  {
    return renderer->MakeCurrentWithContext(context);
  };
  configuration.onGetProcAddress = [renderer](LWE::WebContainer*, const char* name)
  {
    return renderer->GetProcAddress(name);
  };
  configuration.onIsSupportedExtension = [renderer](LWE::WebContainer*, const char* extension)
  {
    return renderer->IsSupportedExtension(extension);
  };

  const LWE::WebContainer::WebContainerArguments arguments = {
    width,
    height,
    1.0f,
    "sans-serif",
    locale.c_str(),
    timezoneId.c_str()};
  mWebContainer = LWE::WebContainer::CreateGL(arguments, configuration);
  DALI_ASSERT_ALWAYS(mWebContainer && "Failed to create LWE OpenGL WebContainer");

  return mWebContainer;
}

void WebEngineLweBackendWin::HandleFrame(std::vector<uint8_t>&& pixels, uint32_t width, uint32_t height)
{
  bool requestUpload = false;
  {
    std::lock_guard<std::mutex> lock(mFrameMutex);
    mReadyBuffer = std::move(pixels);
    mReadyWidth  = width;
    mReadyHeight = height;
    if(!mFrameUploadPending)
    {
      mFrameUploadPending = true;
      requestUpload       = true;
    }
  }
  if(requestUpload)
  {
    DispatchToEventThread([this]()
    {
      UploadFrame();
    });
  }
}

void WebEngineLweBackendWin::Destroy()
{
  mAcceptTasks = false;
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
    mReadyWidth         = 0u;
    mReadyHeight        = 0u;
    mFrameUploadPending = false;
  }
}

void WebEngineLweBackendWin::SetSize(uint32_t width, uint32_t height)
{
  if(mRenderer)
  {
    mRenderer->Resize(width, height);
  }
  if(mWebContainer)
  {
    mWebContainer->ResizeTo(width, height);
  }
}

void WebEngineLweBackendWin::UpdateDisplayArea(uint32_t width, uint32_t height)
{
  SetSize(width, height);
}

Dali::NativeImagePtr WebEngineLweBackendWin::GetNativeImage()
{
  return mNativeImage;
}

void WebEngineLweBackendWin::SetFrameRenderedCallback(FrameRenderedCallback callback)
{
  mFrameRenderedCallback = std::move(callback);
}

void WebEngineLweBackendWin::DispatchToEventThread(Task task)
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

void WebEngineLweBackendWin::ProcessEventTasks()
{
  std::deque<Task> tasks;
  {
    std::lock_guard<std::mutex> lock(mTaskMutex);
    tasks.swap(mTasks);
  }

  if(!mAcceptTasks)
  {
    return;
  }

  for(auto& task : tasks)
  {
    if(task)
    {
      task();
    }
  }
}

void WebEngineLweBackendWin::UploadFrame()
{
  std::vector<uint8_t> pixels;
  uint32_t             width  = 0u;
  uint32_t             height = 0u;
  {
    std::lock_guard<std::mutex> lock(mFrameMutex);
    width  = mReadyWidth;
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
    mNativeImage = Dali::NativeImage::New(width, height, Dali::NativeImage::COLOR_DEPTH_32);
    mImageWidth  = width;
    mImageHeight = height;
  }

  Dali::DevelNativeImage::SetPixels(*mNativeImage, pixels.data(), RENDER_BUFFER_PIXEL_FORMAT);
  if(mFrameRenderedCallback)
  {
    mFrameRenderedCallback();
  }
}

std::unique_ptr<WebEngineLweBackend> CreateWebEngineLweBackend()
{
  return std::make_unique<WebEngineLweBackendWin>();
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
