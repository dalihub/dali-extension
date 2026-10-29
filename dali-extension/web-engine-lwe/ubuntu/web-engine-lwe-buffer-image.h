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

#pragma once

#include <dali/public-api/images/native-image-interface.h>

#include <cstdint>
#include <mutex>
#include <vector>

namespace DALI_NAMESPACE
{
namespace Plugin
{
/** CPU frames uploaded on DALi's render thread without platform native surfaces. */
class WebEngineLweBufferImage : public Dali::NativeImageInterface
{
public:
  WebEngineLweBufferImage(uint32_t width, uint32_t height);
  void setFrame(std::vector<uint8_t>&& pixels);

  bool CreateResource() override;
  void DestroyResource() override;
  uint32_t TargetTexture() override;
  PrepareTextureResult PrepareTexture() override;
  uint32_t GetWidth() const override;
  uint32_t GetHeight() const override;
  bool RequiresBlending() const override;
  int GetTextureTarget() const override;
  bool ApplyNativeFragmentShader(Dali::String&, int) override;
  const char* GetCustomSamplerTypename() const override;
  Dali::Any GetNativeImageHandle() const override;
  bool SourceChanged() const override;
  Dali::Rect<uint32_t> GetUpdatedArea() override;
  void PostRender() override;

private:
  const uint32_t mWidth;
  const uint32_t mHeight;
  mutable std::mutex mMutex;
  std::vector<uint8_t> mPixels;
  bool mChanged{true};
  uint32_t mTexture{0u};
};
} // namespace Plugin
} // namespace DALI_NAMESPACE
