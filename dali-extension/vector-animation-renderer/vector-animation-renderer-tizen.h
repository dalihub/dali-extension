#ifndef DALI_VECTOR_ANIMATION_RENDERER_TIZEN_H
#define DALI_VECTOR_ANIMATION_RENDERER_TIZEN_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>
#include <tbm_surface.h>
#include <tbm_surface_queue.h>

// INTERNAL INCLUDES
#include <dali-extension/vector-animation-renderer/vector-animation-renderer.h>

namespace Dali
{
namespace Plugin
{
/**
 * @brief Implementation of the Tizen vector animation renderer class which has Tizen platform dependency.
 */
class VectorAnimationRendererTizen : public VectorAnimationRenderer
{
  class RenderingDataImpl;

public:
  /**
   * @brief Constructor.
   */
  VectorAnimationRendererTizen();

  /**
   * @brief Destructor.
   */
  virtual ~VectorAnimationRendererTizen();

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::Render()
   */
  bool Render(uint32_t frameNumber) override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::RenderStopped()
   */
  void RenderStopped() override;

private:
  /**
   * @brief Reset buffer list.
   */
  void ResetBuffers() override;

  /**
   * @copydoc VectorAnimationRenderer::Finalize()
   */
  void OnFinalize() override;

  /**
   * @brief Event callback to process events.
   */
  void OnLottieRendered() override
  {
  }

  /**
   * @copydoc VectorAnimationRenderer::OnNotify()
   */
  void OnNotify() override;

  /**
   * @copydoc VectorAnimationRenderer::PrepareTarget()
   */
  void PrepareTarget(std::shared_ptr<RenderingData> renderingData) override;

  /**
   * @brief Set shader for NativeImageSourceQueue with custom sampler type and prefix.
   */
  void SetShader(std::shared_ptr<RenderingData> renderingData) override;

  /**
   * @copydoc VectorAnimationRenderer::OnSetSize()
   */
  void OnSetSize(std::shared_ptr<RenderingData> renderingData) override;

  /**
   * @copydoc VectorAnimationRenderer::IsTargetPrepared()
   */
  bool IsTargetPrepared() override;

  /**
   * @copydoc VectorAnimationRenderer::IsRenderReady()
   */
  bool IsRenderReady() override;

  /**
   * @copydoc VectorAnimationRenderer::GetTargetTexture()
   */
  Dali::Texture GetTargetTexture() override;

  /**
   * @copydoc VectorAnimationRenderer::CreateRenderingData()
   */
  std::shared_ptr<RenderingData> CreateRenderingData() override;

private:
  using SurfacePair = std::pair<tbm_surface_h, rlottie::Surface>;

  std::vector<SurfacePair> mBuffers; ///< EGL Image vector

  Dali::Texture                      mRenderedTexture;  ///< Rendered Texture
  std::vector<Dali::Texture>         mPreviousTextures; ///< Previous rendered texture
};

} // namespace Plugin

} // namespace Dali

#endif // DALI_TIZEN_VECTOR_ANIMATION_RENDERER_PLUGIN_H
