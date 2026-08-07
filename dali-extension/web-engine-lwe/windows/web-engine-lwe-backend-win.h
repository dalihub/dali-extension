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

#ifndef DALI_EXTENSION_WEB_ENGINE_LWE_BACKEND_WIN_H
#define DALI_EXTENSION_WEB_ENGINE_LWE_BACKEND_WIN_H

#include "../common/web-engine-lwe-backend.h"

#include <dali/devel-api/adaptor-framework/event-thread-callback.h>

#include <atomic>
#include <deque>
#include <mutex>
#include <vector>

namespace Dali
{
namespace Plugin
{
class WebEngineLweBackendWin : public WebEngineLweBackend
{
public:
  WebEngineLweBackendWin();
  ~WebEngineLweBackendWin() override;

  LWE::WebContainer* Create(uint32_t width, uint32_t height, const std::string& locale, const std::string& timezoneId) override;
  LWE::WebContainer* Create(uint32_t width, uint32_t height, uint32_t argc, char** argv) override;
  void Destroy() override;
  void SetSize(uint32_t width, uint32_t height) override;
  void UpdateDisplayArea(uint32_t width, uint32_t height) override;
  Dali::NativeImagePtr GetNativeImage() override;
  void SetFrameRenderedCallback(FrameRenderedCallback callback) override;
  void DispatchToEventThread(Task task) override;

private:
  void EnsureInitialized();
  void ProcessEventTasks();
  void UploadFrame();

private:
  static constexpr uint32_t BYTES_PER_PIXEL = 4u;

  LWE::WebContainer* mWebContainer;

  std::atomic<uint32_t> mWidth;
  std::atomic<uint32_t> mHeight;
  uint32_t              mRenderWidth;
  uint32_t              mRenderHeight;
  std::vector<uint8_t>  mRenderBuffer;

  std::mutex           mFrameMutex;
  std::vector<uint8_t> mReadyBuffer;
  uint32_t             mReadyWidth;
  uint32_t             mReadyHeight;
  bool                 mFrameUploadPending;

  Dali::NativeImagePtr mNativeImage;
  uint32_t             mImageWidth;
  uint32_t             mImageHeight;

  std::mutex        mTaskMutex;
  std::deque<Task>  mTasks;
  std::atomic_bool  mAcceptTasks;

  std::shared_ptr<Dali::EventThreadCallback> mEventThreadCallback;
  FrameRenderedCallback                     mFrameRenderedCallback;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_EXTENSION_WEB_ENGINE_LWE_BACKEND_WIN_H
