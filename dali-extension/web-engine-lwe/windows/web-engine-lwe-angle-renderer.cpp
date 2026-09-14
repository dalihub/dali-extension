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

#include "web-engine-lwe-angle-renderer.h"

#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <dali/integration-api/debug.h>

#include <algorithm>
#include <cstring>
#include <limits>
#include <utility>

namespace DALI_NAMESPACE
{
namespace Plugin
{
namespace
{
constexpr wchar_t WINDOW_CLASS_NAME[] = L"DaliLweAngleRendererWindow";
// vcpkg's Khronos EGL headers do not include ANGLE's eglext_angle.h.
constexpr EGLenum PLATFORM_ANGLE = 0x3202;
constexpr EGLint PLATFORM_ANGLE_TYPE = 0x3203;
constexpr EGLint PLATFORM_ANGLE_TYPE_D3D11 = 0x3208;
constexpr EGLint PLATFORM_ANGLE_DEVICE_TYPE = 0x3209;
constexpr EGLint PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE = 0x320A;
constexpr EGLint PLATFORM_ANGLE_DEVICE_TYPE_WARP = 0x320B;

bool RegisterRendererWindowClass()
{
  static const bool registered = []()
  {
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_OWNDC;
    windowClass.lpfnWndProc = DefWindowProcW;
    windowClass.hInstance = GetModuleHandleW(nullptr);
    windowClass.lpszClassName = WINDOW_CLASS_NAME;
    return RegisterClassExW(&windowClass) != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
  }();
  return registered;
}

// Initialize runs on DALi's event thread, where an EGL context may already be
// current. Restore it before returning, including when initialization fails.
class CurrentContextScope
{
public:
  CurrentContextScope()
  : mDisplay(eglGetCurrentDisplay()),
    mContext(eglGetCurrentContext()),
    mDraw(eglGetCurrentSurface(EGL_DRAW)),
    mRead(eglGetCurrentSurface(EGL_READ)),
    mApi(eglQueryAPI())
  {
  }

  ~CurrentContextScope()
  {
    eglBindAPI(mApi);
    if(mDisplay != EGL_NO_DISPLAY)
    {
      eglMakeCurrent(mDisplay, mDraw, mRead, mContext);
    }
  }

private:
  EGLDisplay mDisplay;
  EGLContext mContext;
  EGLSurface mDraw;
  EGLSurface mRead;
  EGLenum mApi;
};
} // unnamed namespace

WebEngineLweAngleRenderer::~WebEngineLweAngleRenderer()
{
  Shutdown();
}

bool WebEngineLweAngleRenderer::Initialize(uint32_t width, uint32_t height)
{
  if(mWindow || !RegisterRendererWindowClass())
  {
    return false;
  }

  CurrentContextScope restoreContext;
  Resize(width, height);
  // The window only supplies a private HDC; pixels are rendered to a pbuffer.
  mWindow = CreateWindowExW(WS_EX_TOOLWINDOW, WINDOW_CLASS_NAME, L"", WS_POPUP,
                            0, 0, 1, 1, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
  if(!mWindow || !(mDeviceContext = ::GetDC(mWindow)))
  {
    DALI_LOG_ERROR("WebEngineLwe: failed to create ANGLE native display: %lu\n", GetLastError());
    Shutdown();
    return false;
  }

  auto getPlatformDisplay = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
  if(getPlatformDisplay)
  {
    for(EGLint deviceType : {PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE, PLATFORM_ANGLE_DEVICE_TYPE_WARP})
    {
      const EGLint attributes[] = {
        PLATFORM_ANGLE_TYPE, PLATFORM_ANGLE_TYPE_D3D11,
        PLATFORM_ANGLE_DEVICE_TYPE, deviceType,
        EGL_NONE};
      if(InitializeDisplay(getPlatformDisplay(PLATFORM_ANGLE, mDeviceContext, attributes)))
      {
        return true;
      }
      DALI_LOG_ERROR("WebEngineLwe: ANGLE device %x initialization failed: EGL %x\n", deviceType, eglGetError());
      DestroyDisplay();
    }
  }
  DALI_LOG_ERROR("WebEngineLwe: failed to initialize ANGLE D3D11 renderer\n");
  Shutdown();
  return false;
}

bool WebEngineLweAngleRenderer::InitializeDisplay(EGLDisplay display)
{
  mDisplay = display;
  if(mDisplay == EGL_NO_DISPLAY || !eglInitialize(mDisplay, nullptr, nullptr) || !eglBindAPI(EGL_OPENGL_ES_API))
  {
    return false;
  }

  const EGLint attributes[] = {
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_STENCIL_SIZE, 8,
    EGL_NONE};
  EGLint count = 0;
  if(!eglChooseConfig(mDisplay, attributes, &mConfig, 1, &count) || count != 1)
  {
    return false;
  }
  mContext = CreateContext(EGL_NO_CONTEXT);
  if(mContext == EGL_NO_CONTEXT || !MakeCurrent())
  {
    return false;
  }
  LoadExtensionString();
  return ClearCurrentContext();
}

void WebEngineLweAngleRenderer::DestroyDisplay()
{
  if(mDisplay != EGL_NO_DISPLAY)
  {
    // Never detach DALi's current context when destroying this renderer.
    if(eglGetCurrentDisplay() == mDisplay)
    {
      ClearCurrentContext();
    }
    if(mContext != EGL_NO_CONTEXT)
    {
      eglDestroyContext(mDisplay, mContext);
    }
    if(mSurface != EGL_NO_SURFACE)
    {
      eglDestroySurface(mDisplay, mSurface);
    }
    eglTerminate(mDisplay);
  }
  mDisplay = EGL_NO_DISPLAY;
  mContext = EGL_NO_CONTEXT;
  mSurface = EGL_NO_SURFACE;
  mConfig = nullptr;
  mSurfaceWidth = 0u;
  mSurfaceHeight = 0u;
  mExtensions.clear();
}

void WebEngineLweAngleRenderer::Shutdown()
{
  // The owner must synchronously destroy WebContainer before releasing EGL.
  mFrameCallback = {};
  DestroyDisplay();
  if(mDeviceContext)
  {
    ReleaseDC(mWindow, mDeviceContext);
    mDeviceContext = nullptr;
  }
  if(mWindow)
  {
    DestroyWindow(mWindow);
    mWindow = nullptr;
  }
}

void WebEngineLweAngleRenderer::Resize(uint32_t width, uint32_t height)
{
  mRequestedSize.store((static_cast<uint64_t>(width) << 32u) | height);
}

void WebEngineLweAngleRenderer::SetFrameCallback(FrameCallback callback)
{
  mFrameCallback = std::move(callback);
}

EGLContext WebEngineLweAngleRenderer::CreateContext(EGLContext shareContext)
{
  const EGLint attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  return eglCreateContext(mDisplay, mConfig, shareContext, attributes);
}

bool WebEngineLweAngleRenderer::UpdateSurface()
{
  const uint64_t size = mRequestedSize.load();
  const uint32_t width = std::max(static_cast<uint32_t>(size >> 32u), 1u);
  const uint32_t height = std::max(static_cast<uint32_t>(size), 1u);
  if(mSurface != EGL_NO_SURFACE && width == mSurfaceWidth && height == mSurfaceHeight)
  {
    return true;
  }
  if(width > static_cast<uint32_t>(std::numeric_limits<EGLint>::max()) ||
     height > static_cast<uint32_t>(std::numeric_limits<EGLint>::max()))
  {
    return false;
  }

  const EGLint attributes[] = {EGL_WIDTH, static_cast<EGLint>(width), EGL_HEIGHT, static_cast<EGLint>(height), EGL_NONE};
  EGLSurface surface = eglCreatePbufferSurface(mDisplay, mConfig, attributes);
  if(surface == EGL_NO_SURFACE)
  {
    return false;
  }
  if(!eglMakeCurrent(mDisplay, surface, surface, mContext))
  {
    eglDestroySurface(mDisplay, surface);
    return false;
  }
  if(mSurface != EGL_NO_SURFACE)
  {
    eglDestroySurface(mDisplay, mSurface);
  }
  mSurface = surface;
  mSurfaceWidth = width;
  mSurfaceHeight = height;
  return true;
}

bool WebEngineLweAngleRenderer::MakeCurrent()
{
  return mDisplay != EGL_NO_DISPLAY && mContext != EGL_NO_CONTEXT && UpdateSurface() &&
         eglMakeCurrent(mDisplay, mSurface, mSurface, mContext) == EGL_TRUE;
}

bool WebEngineLweAngleRenderer::SwapBuffers()
{
  // Reading a pbuffer is the presentation step. There is no window swapchain.
  return mContext != EGL_NO_CONTEXT && eglGetCurrentContext() == mContext && CaptureFrame();
}

uintptr_t WebEngineLweAngleRenderer::CreateSharedContext()
{
  if(mContext == EGL_NO_CONTEXT)
  {
    return UINTPTR_MAX;
  }
  EGLContext context = CreateContext(mContext);
  return context == EGL_NO_CONTEXT ? UINTPTR_MAX : reinterpret_cast<uintptr_t>(context);
}

bool WebEngineLweAngleRenderer::DestroyContext(uintptr_t context)
{
  return context != 0u && context != UINTPTR_MAX && mDisplay != EGL_NO_DISPLAY &&
         eglDestroyContext(mDisplay, reinterpret_cast<EGLContext>(context)) == EGL_TRUE;
}

bool WebEngineLweAngleRenderer::ClearCurrentContext()
{
  return mDisplay != EGL_NO_DISPLAY &&
         eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) == EGL_TRUE;
}

bool WebEngineLweAngleRenderer::MakeCurrentWithContext(uintptr_t context)
{
  return context != 0u && context != UINTPTR_MAX && mSurface != EGL_NO_SURFACE &&
         eglMakeCurrent(mDisplay, mSurface, mSurface, reinterpret_cast<EGLContext>(context)) == EGL_TRUE;
}

void* WebEngineLweAngleRenderer::GetProcAddress(const char* name)
{
  if(!name || !*name)
  {
    return nullptr;
  }
  if(void* address = reinterpret_cast<void*>(eglGetProcAddress(name)))
  {
    return address;
  }
  const wchar_t* moduleName = std::strncmp(name, "egl", 3) == 0 ? L"libEGL.dll" : L"libGLESv2.dll";
  HMODULE module = GetModuleHandleW(moduleName);
  return module ? reinterpret_cast<void*>(::GetProcAddress(module, name)) : nullptr;
}

void WebEngineLweAngleRenderer::LoadExtensionString()
{
  mExtensions.clear();
  if(const GLubyte* extensions = glGetString(GL_EXTENSIONS))
  {
    mExtensions.assign(reinterpret_cast<const char*>(extensions));
  }
  if(const char* extensions = eglQueryString(mDisplay, EGL_EXTENSIONS))
  {
    mExtensions.push_back(' ');
    mExtensions.append(extensions);
  }
}

bool WebEngineLweAngleRenderer::IsSupportedExtension(const char* extension) const
{
  if(!extension || !*extension || std::strchr(extension, ' '))
  {
    return false;
  }
  const size_t length = std::strlen(extension);
  size_t position = 0u;
  while((position = mExtensions.find(extension, position)) != std::string::npos)
  {
    const size_t end = position + length;
    if((position == 0u || mExtensions[position - 1u] == ' ') &&
       (end == mExtensions.size() || mExtensions[end] == ' '))
    {
      return true;
    }
    position = end;
  }
  return false;
}

bool WebEngineLweAngleRenderer::CaptureFrame()
{
  if(!mFrameCallback)
  {
    return true;
  }
  const uint32_t width = mSurfaceWidth;
  const uint32_t height = mSurfaceHeight;
  constexpr size_t BYTES_PER_PIXEL = 4u;
  if(width == 0u || height == 0u ||
     static_cast<size_t>(width) > std::numeric_limits<size_t>::max() / BYTES_PER_PIXEL / height)
  {
    return false;
  }
  const size_t stride = static_cast<size_t>(width) * BYTES_PER_PIXEL;
  std::vector<uint8_t> pixels(stride * height);
  // LWE may leave an offscreen FBO bound. Capture the EGL surface, then restore
  // GL state so the compositor's cached state still matches the driver.
  GLint framebuffer = 0;
  GLint packAlignment = 4;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer);
  glGetIntegerv(GL_PACK_ALIGNMENT, &packAlignment);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  while(glGetError() != GL_NO_ERROR)
  {
  }
  glReadPixels(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height), GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
  const GLenum error = glGetError();
  glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(framebuffer));
  glPixelStorei(GL_PACK_ALIGNMENT, packAlignment);
  if(error != GL_NO_ERROR)
  {
    DALI_LOG_ERROR("WebEngineLwe: failed to read ANGLE pbuffer: GL %x\n", error);
    return false;
  }
  for(uint32_t row = 0u; row < height / 2u; ++row)
  {
    const size_t top = static_cast<size_t>(row) * stride;
    const size_t bottom = static_cast<size_t>(height - row - 1u) * stride;
    std::swap_ranges(pixels.begin() + top, pixels.begin() + top + stride, pixels.begin() + bottom);
  }
  mFrameCallback(std::move(pixels), width, height);
  return true;
}

} // namespace Plugin
} // namespace DALI_NAMESPACE
