#ifndef DALI_TIZEN_VECTOR_ANIMATION_RENDERER_PLUGIN_H
#define DALI_TIZEN_VECTOR_ANIMATION_RENDERER_PLUGIN_H

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/math/uint-16-pair.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/devel-api/threading/mutex.h>
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>
#include <dali/devel-api/adaptor-framework/vector-animation-renderer-plugin.h>
#include <memory>
#include <rlottie.h>
#include <tbm_surface.h>
#include <tbm_surface_queue.h>

namespace Dali
{

namespace Plugin
{

/**
 * @brief Implementation of the Tizen vector animation renderer class which has Tizen platform dependency.
 */
class TizenVectorAnimationRenderer : public Dali::VectorAnimationRendererPlugin
{
public:

  /**
   * @brief Constructor.
   */
  TizenVectorAnimationRenderer();

  /**
   * @brief Destructor.
   */
  virtual ~TizenVectorAnimationRenderer();

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::SetUrl()
   */
  void SetUrl( const std::string& url ) override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::SetRenderer()
   */
  void SetRenderer( Renderer renderer ) override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::SetSize()
   */
  void SetSize( uint32_t width, uint32_t height ) override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::StartRender()
   */
  bool StartRender() override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::StopRender()
   */
  void StopRender() override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::Render()
   */
  void Render( uint32_t frameNumber ) override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::GetTotalFrameNumber()
   */
  uint32_t GetTotalFrameNumber() override;

private:

  /**
   * @brief Set shader for NativeImageSourceQueue with custom sampler type and prefix.
   */
  void SetShader();

private:

  using SurfacePair = std::pair< tbm_surface_h, rlottie::Surface >;

  std::string                          mUrl;               ///< The content file path
  std::vector< SurfacePair >           mBuffers;           ///< EGL Image vector
  Dali::Mutex                          mMutex;             ///< Mutex
  Dali::Renderer                       mRenderer;          ///< Renderer
  NativeImageSourceQueuePtr            mTargetSurface;     ///< The target surface
  std::unique_ptr< rlottie::Animation > mVectorRenderer;    ///< The vector animation renderer
  tbm_surface_queue_h                  mTbmQueue;          ///< Tbm surface queue handle
  uint32_t                             mTotalFrameNumber;  ///< The total frame number
  uint32_t                             mWidth;             ///< The width of the surface
  uint32_t                             mHeight;            ///< The height of the surface
};

} // namespace Plugin

} // namespace Dali;

#endif // DALI_TIZEN_VECTOR_ANIMATION_RENDERER_PLUGIN_H
