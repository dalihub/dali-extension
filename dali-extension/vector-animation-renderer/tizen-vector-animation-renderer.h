#ifndef DALI_TIZEN_VECTOR_ANIMATION_RENDERER_PLUGIN_H
#define DALI_TIZEN_VECTOR_ANIMATION_RENDERER_PLUGIN_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/devel-api/threading/mutex.h>
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>
#include <dali/devel-api/adaptor-framework/vector-animation-renderer-plugin.h>
#include <memory>
#include <rlottie.h>
#include <tbm_surface.h>
#include <tbm_surface_queue.h>

// INTERNAL INCLUDES
#include <dali-extension/vector-animation-renderer/tizen-vector-animation-event-handler.h>

namespace Dali
{

namespace Plugin
{

/**
 * @brief Implementation of the Tizen vector animation renderer class which has Tizen platform dependency.
 */
class TizenVectorAnimationRenderer : public Dali::VectorAnimationRendererPlugin, public TizenVectorAnimationEventHandler
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
   * @copydoc Dali::VectorAnimationRendererPlugin::Initialize()
   */
  bool Initialize( const std::string& url ) override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::Finalize()
   */
  void Finalize() override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::SetRenderer()
   */
  void SetRenderer( Renderer renderer ) override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::SetSize()
   */
  void SetSize( uint32_t width, uint32_t height ) override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::Render()
   */
  bool Render( uint32_t frameNumber ) override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::GetTotalFrameNumber()
   */
  uint32_t GetTotalFrameNumber() const override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::GetFrameRate()
   */
  float GetFrameRate() const override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::GetDefaultSize()
   */
  void GetDefaultSize( uint32_t& width, uint32_t& height ) const override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::GetLayerInfo()
   */
  void GetLayerInfo( Property::Map& map ) const override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::GetMarkerInfo()
   */
  bool GetMarkerInfo( const std::string& marker, uint32_t& startFrame, uint32_t& endFrame ) const override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::IgnoreRenderedFrame()
   */
  void IgnoreRenderedFrame() override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::UploadCompletedSignal()
   */
  UploadCompletedSignalType& UploadCompletedSignal() override;

protected: // Implementation of TizenVectorAnimationEventHandler

  /**
   * @copydoc Dali::Plugin::TizenVectorAnimationEventHandler::NotifyEvent()
   */
  void NotifyEvent() override;

private:

  /**
   * @brief Set shader for NativeImageSourceQueue with custom sampler type and prefix.
   */
  void SetShader();

  /**
   * @brief Reset buffer list.
   */
  void ResetBuffers();

private:

  using SurfacePair = std::pair< tbm_surface_h, rlottie::Surface >;

  std::string                            mUrl;                   ///< The content file path
  std::vector< SurfacePair >             mBuffers;               ///< EGL Image vector
  mutable Dali::Mutex                    mMutex;                 ///< Mutex
  Dali::Renderer                         mRenderer;              ///< Renderer
  Dali::Texture                          mTexture;               ///< Texture
  Dali::Texture                          mRenderedTexture;       ///< Rendered Texture
  NativeImageSourceQueuePtr              mTargetSurface;         ///< The target surface
  std::unique_ptr< rlottie::Animation >  mVectorRenderer;        ///< The vector animation renderer
  UploadCompletedSignalType              mUploadCompletedSignal; ///< Upload completed signal
  tbm_surface_queue_h                    mTbmQueue;              ///< Tbm surface queue handle
  uint32_t                               mTotalFrameNumber;      ///< The total frame number
  uint32_t                               mWidth;                 ///< The width of the surface
  uint32_t                               mHeight;                ///< The height of the surface
  uint32_t                               mDefaultWidth;          ///< The width of the surface
  uint32_t                               mDefaultHeight;         ///< The height of the surface
  float                                  mFrameRate;             ///< The frame rate of the content
  bool                                   mResourceReady;         ///< Whether the resource is ready
  bool                                   mShaderChanged;         ///< Whether the shader is changed to support native image
  bool                                   mResourceReadyTriggered;///< Whether the resource ready is triggered
};

} // namespace Plugin

} // namespace Dali;

#endif // DALI_TIZEN_VECTOR_ANIMATION_RENDERER_PLUGIN_H
