#ifndef DALI_TIZEN_RIVE_ANIMATION_RENDERER_PLUGIN_H
#define DALI_TIZEN_RIVE_ANIMATION_RENDERER_PLUGIN_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <tbm_surface.h>
#include <tbm_surface_queue.h>

// RIVE - THORVG INCLUDES
#include <thorvg.h>
#include <rive/animation/linear_animation_instance.hpp>
#include <rive/artboard.hpp>
//

// INTERNAL INCLUDES
#include <dali-extension/vector-animation-renderer/tizen-vector-animation-event-handler.h>

namespace Dali
{

namespace Plugin
{

/**
 * @brief Implementation of the Tizen vector animation renderer class which has Tizen platform dependency.
 */
class TizenRiveAnimationRenderer : public Dali::VectorAnimationRendererPlugin, public TizenVectorAnimationEventHandler
{
public:

  /**
   * @brief Constructor.
   */
  TizenRiveAnimationRenderer();

  /**
   * @brief Destructor.
   */
  virtual ~TizenRiveAnimationRenderer();

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::Initialize()
   */
  bool Load( const std::string& url ) override;

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
  bool Render( uint32_t frameNumber) override;

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

  /**
   * @brief Load rive resource file for artboard.
   */
  void LoadRiveFile(const std::string& filename);

private:

  std::string                            mUrl;                   ///< The content file path
  mutable Dali::Mutex                    mMutex;                 ///< Mutex
  Dali::Renderer                         mRenderer;              ///< Renderer
  Dali::Texture                          mTexture;               ///< Texture
  Dali::Texture                          mRenderedTexture;       ///< Rendered Texture
  Dali::Texture                          mPreviousTexture;       ///< Previous rendered texture
  NativeImageSourceQueuePtr              mTargetSurface;         ///< The target surface
  UploadCompletedSignalType              mUploadCompletedSignal; ///< Upload completed signal
  tbm_surface_queue_h                    mTbmQueue;              ///< Tbm surface queue handle
  std::unique_ptr<tvg::SwCanvas>         mSwCanvas;              ///< ThorVG SW canvas handle
  rive::File                             *mFile;                 ///< Rive file handle
  rive::Artboard                         *mArtboard;             ///< Rive artboard handle
  rive::LinearAnimation                  *mAnimation;            ///< Rive animation handle
  rive::LinearAnimationInstance          *mAnimationInstance;    ///< Rive animation instance
  uint32_t                               mStartFrameNumber;      ///< The start frame number
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

#endif // DALI_TIZEN_RIVE_ANIMATION_RENDERER_PLUGIN_H
