#include "web-engine-lwe-angle-renderer.h"

#include <GLES2/gl2.h>

#include <cstdio>
#include <stdexcept>
#include <thread>
#include <utility>

namespace
{
using Renderer = DALI_NAMESPACE::Plugin::WebEngineLweAngleRenderer;

void Require(bool value, const char* message)
{
  if(!value)
  {
    throw std::runtime_error(message);
  }
}

void CheckFrame(Renderer& renderer, uint32_t width, uint32_t height)
{
  std::vector<uint8_t> pixels;
  renderer.SetFrameCallback([&](std::vector<uint8_t>&& frame, uint32_t frameWidth, uint32_t frameHeight)
  {
    Require(frameWidth == width && frameHeight == height, "Frame dimensions do not match the surface");
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
  Require(renderer.SwapBuffers(), "Frame capture failed");
  renderer.SetFrameCallback({});
  GLint binding = 0;
  GLint alignment = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &binding);
  glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
  Require(binding == static_cast<GLint>(framebuffer) && alignment == 8, "Readback changed compositor GL state");
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &framebuffer);
  Require(pixels.size() == static_cast<size_t>(width) * height * 4u, "Invalid pixel buffer size");
  for(uint32_t row = 0; row < height; ++row)
  {
    for(uint32_t column = 0; column < width; ++column)
    {
      const size_t offset = (static_cast<size_t>(row) * width + column) * 4u;
      const bool red = row < height - height / 2;
      Require(pixels[offset] == (red ? 255 : 0) && pixels[offset + 1] == 0 &&
              pixels[offset + 2] == (red ? 0 : 255) && pixels[offset + 3] == 255,
              "RGBA color or top-down row order is wrong");
    }
  }
}
} // unnamed namespace

int main()
{
  try
  {
    Renderer first;
    Require(first.Initialize(3, 5), "ANGLE initialization failed");
    Require(first.MakeCurrent(), "Could not bind first renderer");
    CheckFrame(first, 3, 5);
    Require(first.GetProcAddress("glCreateShader") != nullptr, "Missing GLES procedure");
    Require(first.GetProcAddress("eglQueryString") != nullptr, "Missing EGL procedure");
    Require(first.GetProcAddress(nullptr) == nullptr, "Invalid procedure name accepted");
    Require(!first.IsSupportedExtension("GL_") && !first.IsSupportedExtension(" "), "Partial extension name accepted");

    const EGLDisplay display = eglGetCurrentDisplay();
    const EGLContext context = eglGetCurrentContext();
    Renderer invalid;
    Require(!invalid.Initialize(UINT32_MAX, 1), "Oversized surface unexpectedly succeeded");
    Require(eglGetCurrentDisplay() == display && eglGetCurrentContext() == context,
            "Failed initialization changed another renderer's current context");
    Renderer second;
    Require(second.Initialize(5, 3), "Second renderer initialization failed");
    Require(eglGetCurrentDisplay() == display && eglGetCurrentContext() == context,
            "Initialization changed another renderer's current context");
    Require(second.MakeCurrent(), "Could not bind second renderer");
    CheckFrame(second, 5, 3);
    Require(first.MakeCurrent(), "Could not restore first renderer");
    second.Shutdown();
    Require(eglGetCurrentContext() == context, "Shutdown detached another renderer's context");
    CheckFrame(first, 3, 5);

    first.Resize(7, 3);
    // A pending resize must not change the dimensions of an already drawn frame.
    CheckFrame(first, 3, 5);
    Require(first.MakeCurrent(), "Resize failed");
    CheckFrame(first, 7, 3);
    first.Resize(0, 0);
    Require(first.MakeCurrent(), "Zero-size surface failed");
    CheckFrame(first, 1, 1);

    const uintptr_t shared = first.CreateSharedContext();
    Require(shared != UINTPTR_MAX, "Shared context creation failed");
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    Require(first.ClearCurrentContext(), "Could not release main context");
    Require(first.MakeCurrentWithContext(shared), "Could not bind shared context");
    Require(glIsTexture(texture) == GL_TRUE, "Contexts do not share texture objects");
    Require(first.ClearCurrentContext() && first.MakeCurrent(), "Could not restore main context");
    Require(first.DestroyContext(shared), "Shared context destruction failed");
    glDeleteTextures(1, &texture);
    Require(!first.MakeCurrentWithContext(UINTPTR_MAX) && !first.DestroyContext(0), "Invalid context accepted");
    first.Shutdown();
    first.Shutdown();
    Require(!first.MakeCurrent() && !first.SwapBuffers(), "Renderer remained usable after shutdown");
    Require(first.Initialize(2, 2) && first.MakeCurrent(), "Reinitialization failed");
    CheckFrame(first, 2, 2);
    Require(first.ClearCurrentContext(), "Could not release context for render thread");
    std::exception_ptr workerError;
    std::thread worker([&]()
    {
      try
      {
        Require(first.MakeCurrent(), "Could not move context to the render thread");
        CheckFrame(first, 2, 2);
        Require(first.ClearCurrentContext(), "Could not release render-thread context");
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
    std::puts("ANGLE renderer tests passed");
    return 0;
  }
  catch(const std::exception& error)
  {
    std::fprintf(stderr, "%s\n", error.what());
    return 1;
  }
}
