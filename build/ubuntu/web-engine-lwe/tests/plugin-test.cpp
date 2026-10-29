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

#include <dali/devel-api/adaptor-framework/web-engine/web-engine-plugin.h>
#include <dali/public-api/adaptor-framework/application.h>
#include <dali/public-api/adaptor-framework/timer.h>
#include <LWEWebView.h>

#include "web-engine-lwe-egl-renderer.h"
#include <GLES2/gl2.h>

#include <cstdio>
#include <memory>
#include <stdexcept>

extern "C" Dali::WebEnginePlugin* CreateWebEnginePlugin();
extern "C" void DestroyWebEnginePlugin(Dali::WebEnginePlugin* plugin);

namespace
{
using PluginPtr = std::unique_ptr<Dali::WebEnginePlugin, decltype(&DestroyWebEnginePlugin)>;

const char* RED_PAGE = "<!doctype html><html><head><title>Ubuntu LWE</title></head><body style='margin:0;background:#ff0000'></body></html>";
const char* BLUE_PAGE = "<!doctype html><html><body style='margin:0;background:#0000ff'></body></html>";

class PluginTest : public Dali::ConnectionTracker
{
public:
  explicit PluginTest(Dali::Application& application)
  : mApplication(application)
  {
    mApplication.InitSignal().Connect(this, &PluginTest::onInit);
  }

  ~PluginTest()
  {
    mFirst.reset();
    mSecond.reset();
    mProbe.shutdown();
    if(LWE::LWE::IsInitialized())
    {
      LWE::LWE::Finalize();
    }
  }

  int result() const
  {
    return mResult;
  }

private:
  void onInit(Dali::Application)
  {
    mApplication.GetWindow().Hide();
    if(!mProbe.initialize(1, 1))
    {
      std::fprintf(stderr, "EGL probe initialization failed\n");
      mApplication.Quit();
      return;
    }
    mFirst.reset(CreateWebEnginePlugin());
    mSecond.reset(CreateWebEnginePlugin());
    mFirst->RegisterFrameRenderedCallback([this]() { ++mFrames; });
    mSecond->RegisterFrameRenderedCallback([this]() { ++mFrames; });
    mFirst->Create(640, 480, "en-US", "UTC");
    mFirst->SetSize(321, 241);
    mFirst->LoadHtmlString(RED_PAGE);
    mSecond->Create(640, 480, "en-US", "UTC");
    mSecond->SetSize(259, 131);
    mSecond->LoadHtmlString(BLUE_PAGE);
    mTimer = Dali::Timer::New(100);
    mTimer.TickSignal().Connect(this, &PluginTest::onTick);
    mTimer.Start();
  }

  bool matches(Dali::WebEnginePlugin& plugin, uint32_t width, uint32_t height, uint8_t red, uint8_t green, uint8_t blue)
  {
    auto image = plugin.GetNativeImageInterface();
    if(!image || image->GetWidth() != width || image->GetHeight() != height)
    {
      return false;
    }
    if(!mProbe.makeCurrent())
    {
      throw std::runtime_error("Could not bind EGL probe");
    }
    GLuint texture = 0;
    GLuint framebuffer = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    image->CreateResource();
    const uint32_t error = image->TargetTexture();
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    uint8_t pixel[4]{};
    const bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    if(complete)
    {
      glReadPixels(width / 2, height / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    }
    image->DestroyResource();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &texture);
    mProbe.clearCurrentContext();
    if(error != GL_NO_ERROR || !complete)
    {
      throw std::runtime_error("LWE frame texture upload failed");
    }
    return pixel[0] == red && pixel[1] == green && pixel[2] == blue && pixel[3] == 255;
  }

  bool onTick()
  {
    try
    {
      if(++mTicks > 200)
      {
        throw std::runtime_error("Timed out waiting for the expected LWE frame");
      }
      if(mStage == 0 && matches(*mFirst, 321, 241, 255, 0, 0) && matches(*mSecond, 259, 131, 0, 0, 255))
      {
        std::puts("Two WebViews rendered independent HTML frames");
        mSecond->Destroy();
        mSecond->Destroy();
        mSecond.reset();
        mFirst->SetSize(257, 129);
        mFirst->EvaluateJavaScript("document.body.style.backgroundColor='#00ff00'", {});
        mStage = 1;
      }
      else if(mStage == 1 && matches(*mFirst, 257, 129, 0, 255, 0))
      {
        std::puts("JavaScript update and resize survived destruction of the other WebView");
        mFirst->Destroy();
        mFirst->Destroy();
        mFirst->Create(640, 480, "en-US", "UTC");
        mFirst->SetSize(301, 177);
        mFirst->LoadHtmlString(BLUE_PAGE);
        mStage = 2;
      }
      else if(mStage == 2 && matches(*mFirst, 301, 177, 0, 0, 255))
      {
        if(mFrames == 0)
        {
          throw std::runtime_error("No frame callbacks reached the DALi event thread");
        }
        mStage = 3;
        mFirst->RegisterFrameRenderedCallback([this]()
        {
          mFirst.reset();
          mStage = 4;
        });
        mFirst->EvaluateJavaScript("document.body.style.backgroundColor='#ff0000'", {});
      }
      else if(mStage == 4)
      {
        std::printf("LWE plugin tests passed (%u frame callbacks, including destruction inside a frame callback)\n", mFrames);
        mResult = 0;
        mApplication.Quit();
        return false;
      }
    }
    catch(const std::exception& error)
    {
      std::fprintf(stderr, "Stage %d: %s\n", mStage, error.what());
      mApplication.Quit();
      return false;
    }
    return true;
  }

  Dali::Application& mApplication;
  Dali::Timer mTimer;
  PluginPtr mFirst{nullptr, DestroyWebEnginePlugin};
  PluginPtr mSecond{nullptr, DestroyWebEnginePlugin};
  Dali::Plugin::WebEngineLweEglRenderer mProbe;
  int mStage{0};
  int mResult{1};
  unsigned int mTicks{0};
  unsigned int mFrames{0};
};
} // namespace

int main(int argc, char** argv)
{
  Dali::Application application = Dali::Application::New(&argc, &argv);
  PluginTest test(application);
  application.MainLoop();
  return test.result();
}
