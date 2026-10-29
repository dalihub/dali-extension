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

#include "web-engine-lwe-egl-renderer.h"

#include "web-engine-lwe-buffer-image.h"

#include <GLES2/gl2.h>
#include <X11/Xlib.h>

#include <cstdio>
#include <stdexcept>
#include <thread>
#include <utility>

namespace
{
using Renderer = DALI_NAMESPACE::Plugin::WebEngineLweEglRenderer;

void require(bool value, const char* message)
{
  if(!value)
  {
    throw std::runtime_error(message);
  }
}

void checkFrame(Renderer& renderer, uint32_t width, uint32_t height)
{
  std::vector<uint8_t> pixels;
  renderer.setFrameCallback([&](std::vector<uint8_t>&& frame, uint32_t frameWidth, uint32_t frameHeight)
  {
    require(frameWidth == width && frameHeight == height, "Frame dimensions do not match the surface");
    pixels = std::move(frame);
  });
  glViewport(0, 0, width, height);
  glDisable(GL_SCISSOR_TEST);
  glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_SCISSOR_TEST);
  glScissor(0, height / 2, width, height - height / 2);
  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_SCISSOR_TEST);

  // Readback must capture the pbuffer even if LWE left another FBO bound.
  GLuint framebuffer = 0;
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glPixelStorei(GL_PACK_ALIGNMENT, 8);
  require(renderer.swapBuffers(), "Frame capture failed");
  renderer.setFrameCallback({});
  GLint binding = 0;
  GLint alignment = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &binding);
  glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
  require(binding == static_cast<GLint>(framebuffer) && alignment == 8, "Readback changed compositor GL state");
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &framebuffer);
  require(pixels.size() == static_cast<size_t>(width) * height * 4u, "Invalid pixel buffer size");
  for(uint32_t row = 0; row < height; ++row)
  {
    for(uint32_t column = 0; column < width; ++column)
    {
      const size_t offset = (static_cast<size_t>(row) * width + column) * 4u;
      const bool red = row < height - height / 2;
      require(pixels[offset] == (red ? 255 : 0) && pixels[offset + 1] == 0 &&
              pixels[offset + 2] == (red ? 0 : 255) && pixels[offset + 3] == 255,
              "RGBA color or top-down row order is wrong");
    }
  }
}
void checkBufferUpload()
{
  Dali::IntrusivePtr<Dali::Plugin::WebEngineLweBufferImage> image(new Dali::Plugin::WebEngineLweBufferImage(3, 5));
  GLuint texture = 0;
  GLuint framebuffer = 0;
  glGenTextures(1, &texture);
  glGenFramebuffers(1, &framebuffer);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  require(image->CreateResource(), "Buffer image initialization failed");
  for(uint8_t channel : {uint8_t(37), uint8_t(219)})
  {
    image->setFrame(std::vector<uint8_t>(3u * 5u * 4u, channel));
    require(image->SourceChanged(), "Frame update was lost");
    glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
    require(image->TargetTexture() == GL_NO_ERROR, "Buffer texture upload failed");
    require(!image->SourceChanged(), "Completed upload remained dirty");
    GLint alignment = 0;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    require(alignment == 8, "Upload changed unpack alignment");
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    require(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Uploaded texture is incomplete");
    uint8_t pixel[4]{};
    glReadPixels(1, 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    require(pixel[0] == channel && pixel[1] == channel && pixel[2] == channel && pixel[3] == channel,
            "Uploaded frame contents are wrong");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  GLuint secondTexture = 0;
  glGenTextures(1, &secondTexture);
  glBindTexture(GL_TEXTURE_2D, secondTexture);
  require(image->TargetTexture() == GL_NO_ERROR, "Uploading to another texture failed");
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, secondTexture, 0);
  uint8_t secondPixel[4]{};
  glReadPixels(1, 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, secondPixel);
  require(secondPixel[0] == 219, "Another texture did not receive the latest frame");
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteTextures(1, &secondTexture);
  image->DestroyResource();
  glDeleteTextures(1, &texture);
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  require(image->CreateResource() && image->SourceChanged() && image->TargetTexture() == GL_NO_ERROR,
          "Buffer image could not restore its latest frame");
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
  uint8_t pixel[4]{};
  glReadPixels(1, 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
  require(pixel[0] == 219, "Resource recreation lost the CPU frame");
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  image->DestroyResource();
  glDeleteFramebuffers(1, &framebuffer);
  glDeleteTextures(1, &texture);
}
} // unnamed namespace

int main()
{
  try
  {
    XInitThreads();
    Renderer first;
    require(first.initialize(3, 5), "EGL initialization failed");
    require(first.makeCurrent(), "Could not bind first renderer");
    checkFrame(first, 3, 5);
    require(first.getProcAddress("glCreateShader") != nullptr, "Missing GLES procedure");
    require(first.getProcAddress("eglQueryString") != nullptr, "Missing EGL procedure");
    require(first.getProcAddress(nullptr) == nullptr, "Invalid procedure name accepted");
    require(!first.isSupportedExtension("GL_") && !first.isSupportedExtension(" "), "Partial extension name accepted");

    const EGLDisplay display = eglGetCurrentDisplay();
    const EGLContext context = eglGetCurrentContext();
    Renderer invalid;
    require(!invalid.initialize(UINT32_MAX, 1), "Oversized surface unexpectedly succeeded");
    require(eglGetCurrentDisplay() == display && eglGetCurrentContext() == context,
            "Failed initialization changed another renderer's current context");
    Renderer second;
    require(second.initialize(5, 3), "Second renderer initialization failed");
    require(eglGetCurrentDisplay() == display && eglGetCurrentContext() == context,
            "Initialization changed another renderer's current context");
    require(second.makeCurrent(), "Could not bind second renderer");
    checkFrame(second, 5, 3);
    require(first.makeCurrent(), "Could not restore first renderer");
    second.shutdown();
    require(eglGetCurrentContext() == context, "shutdown detached another renderer's context");
    checkFrame(first, 3, 5);

    checkBufferUpload();

    first.resize(7, 3);
    // A pending resize must not change the dimensions of an already drawn frame.
    checkFrame(first, 3, 5);
    require(first.makeCurrent(), "resize failed");
    checkFrame(first, 7, 3);
    first.resize(0, 0);
    require(first.makeCurrent(), "Zero-size surface failed");
    checkFrame(first, 1, 1);

    const uintptr_t shared = first.createSharedContext();
    require(shared != UINTPTR_MAX, "Shared context creation failed");
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    require(first.clearCurrentContext(), "Could not release main context");
    require(first.makeCurrentWithContext(shared), "Could not bind shared context");
    require(glIsTexture(texture) == GL_TRUE, "Contexts do not share texture objects");
    require(first.clearCurrentContext() && first.makeCurrent(), "Could not restore main context");
    require(first.destroyContext(shared), "Shared context destruction failed");
    glDeleteTextures(1, &texture);
    require(!first.makeCurrentWithContext(UINTPTR_MAX) && !first.destroyContext(0), "Invalid context accepted");
    first.shutdown();
    first.shutdown();
    require(!first.makeCurrent() && !first.swapBuffers(), "Renderer remained usable after shutdown");
    require(first.initialize(2, 2) && first.makeCurrent(), "Reinitialization failed");
    checkFrame(first, 2, 2);
    require(first.clearCurrentContext(), "Could not release context for render thread");
    std::exception_ptr workerError;
    std::thread worker([&]()
    {
      try
      {
        require(first.makeCurrent(), "Could not move context to the render thread");
        checkFrame(first, 2, 2);
        require(first.clearCurrentContext(), "Could not release render-thread context");
      }
      catch(...)
      {
        workerError = std::current_exception();
      }
    });
    worker.join();
    if(workerError)
    {
      std::rethrow_exception(workerError);
    }
    std::puts("EGL renderer tests passed");
    return 0;
  }
  catch(const std::exception& error)
  {
    std::fprintf(stderr, "%s\n", error.what());
    return 1;
  }
}
