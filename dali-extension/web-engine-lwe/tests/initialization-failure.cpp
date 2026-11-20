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

#include "web-engine-lwe.h"

#include <cassert>
#include <memory>

using namespace Dali;
using namespace Dali::Plugin;

namespace
{
struct Resources
{
  unsigned creates{0};
  unsigned releases{0};
  bool allocated{false};
  bool callbackRegistered{false};
};

// Simulates failure after a backend has allocated resources but before it can
// return a WebContainer. No LWE process or graphics context is started.
class FailingBackend : public WebEngineLweBackend
{
public:
  explicit FailingBackend(Resources& resources)
  : mResources(resources)
  {
  }

  LWE::WebContainer* Create(uint32_t, uint32_t, const std::string&, const std::string&) override
  {
    return FailCreation();
  }

  LWE::WebContainer* Create(uint32_t, uint32_t, uint32_t, char**) override
  {
    return FailCreation();
  }

  void Destroy() override
  {
    if(mResources.allocated)
    {
      ++mResources.releases;
      mResources.allocated = false;
    }
  }

  void SetFrameRenderedCallback(FrameRenderedCallback callback) override
  {
    mResources.callbackRegistered = static_cast<bool>(callback);
  }

  void SetSize(uint32_t, uint32_t) override {}
  void UpdateDisplayArea(uint32_t, uint32_t) override {}
  NativeImagePtr GetNativeImage() override { return {}; }
  void DispatchToEventThread(Task task) override { task(); }

private:
  LWE::WebContainer* FailCreation()
  {
    assert(mResources.callbackRegistered);
    assert(!mResources.allocated);
    ++mResources.creates;
    mResources.allocated = true;
    return nullptr;
  }

  Resources& mResources;
};
}

int main()
{
  Resources resources;
  {
    WebEngineLwe engine(std::make_unique<FailingBackend>(resources));
    for(unsigned attempt = 0; attempt < 2; ++attempt)
    {
      const bool localeResult = engine.InitializeNativeImageRendering(100u, 100u, "", "");
      assert(!localeResult);
      assert(!resources.allocated && !resources.callbackRegistered);
      engine.Create(100u, 100u, 0u, nullptr);
      assert(!resources.allocated && !resources.callbackRegistered);
    }
    // Legacy callers share the same cleanup path, although they cannot receive bool.
    engine.Create(100u, 100u, "", "");
    assert(!resources.allocated && !resources.callbackRegistered);
    engine.Create(100u, 100u, 0u, nullptr);
    assert(!resources.allocated && !resources.callbackRegistered);
    engine.Destroy();
    engine.Destroy();
    assert(resources.creates == 6u && resources.releases == 6u);
  }
  assert(resources.creates == resources.releases);
}
