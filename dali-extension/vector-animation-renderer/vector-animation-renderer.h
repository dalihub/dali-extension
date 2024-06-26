#ifndef DALI_EXTENSION_VECTOR_ANIMATION_RENDERER_H
#define DALI_EXTENSION_VECTOR_ANIMATION_RENDERER_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/devel-api/adaptor-framework/vector-animation-renderer-plugin.h>
#include <dali/devel-api/threading/mutex.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <rlottie.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali-extension/vector-animation-renderer/vector-animation-event-handler.h>

namespace Dali
{
namespace Plugin
{
/**
 * @brief Implementation of the Tizen vector animation renderer class which has Tizen platform dependency.
 */
class VectorAnimationRenderer : public Dali::VectorAnimationRendererPlugin, public VectorAnimationEventHandler
{
public:
  /**
   * @brief Constructor.
   */
  VectorAnimationRenderer();

  /**
   * @brief Destructor.
   */
  virtual ~VectorAnimationRenderer();

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::Finalize()
   */
  void Finalize() override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::Load()
   */
  bool Load(const std::string& url) override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::Load()
   */
  bool Load(const Dali::Vector<uint8_t>& data) override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::SetRenderer()
   */
  void SetRenderer(Renderer renderer) override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::SetSize()
   */
  void SetSize(uint32_t width, uint32_t height) override;

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
  void GetDefaultSize(uint32_t& width, uint32_t& height) const override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::GetLayerInfo()
   */
  void GetLayerInfo(Property::Map& map) const override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::GetMarkerInfo()
   */
  bool GetMarkerInfo(const std::string& marker, uint32_t& startFrame, uint32_t& endFrame) const override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::GetMarkerInfo()
   */
  void GetMarkerInfo(Property::Map& map) const override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::InvalidateBuffer()
   */
  void InvalidateBuffer() override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::AddPropertyValueCallback()
   */
  void AddPropertyValueCallback(const std::string& keyPath, VectorProperty property, CallbackBase* callback, int32_t id) override;

  void KeepRasterizedBuffer();

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::UploadCompletedSignal()
   */
  UploadCompletedSignalType& UploadCompletedSignal() override;

protected: // Implementation of VectorAnimationEventHandler
  /**
   * @copydoc Dali::Plugin::VectorAnimationEventHandler::NotifyEvent()
   */
  void NotifyEvent() override;

protected:
  class RenderingData
  {
  public:
    Dali::Texture mTexture;   ///< Texture
    uint32_t      mWidth{0};  ///< The width of the surface
    uint32_t      mHeight{0}; ///< The height of the surface
  };

  /**
   * @brief Reset buffer list.
   */
  virtual void ResetBuffers() = 0;

  /**
   * @brief Reset properties
   */
  virtual void OnFinalize() = 0;

  /**
   * @brief Notify event thread comes
   */
  virtual void OnNotify() = 0;

  /**
   * @brief Prepare target
   */
  virtual void PrepareTarget(std::shared_ptr<RenderingData> renderingData) = 0;

  /**
   * @brief Apply the changes of Size
   */
  virtual void OnSetSize(std::shared_ptr<RenderingData> renderingData) = 0;

  /**
   * @brief Retrieve whether the target is prepared or not.
   */
  virtual bool IsTargetPrepared() = 0;

  /**
   * @brief Retrieve whether the rendering is ready or not.
   */
  virtual bool IsRenderReady() = 0;

  /**
   * @brief Retrieve target texture.
   */
  virtual Dali::Texture GetTargetTexture() = 0;

  /**
   * @brief Clear Previous RenderingData
   */
  void ClearPreviousRenderingData();

  /**
   * @brief Create RenderingData
   */
  virtual std::shared_ptr<RenderingData> CreateRenderingData() = 0;

protected:
  std::string                                        mUrl;               ///< The content file path
  std::vector<std::unique_ptr<CallbackBase>>         mPropertyCallbacks; ///< Property callback list
  std::vector<std::pair<std::vector<uint8_t>, bool>> mDecodedBuffers;

  std::shared_ptr<RenderingData>              mPreparedRenderingData;
  std::shared_ptr<RenderingData>              mCurrentRenderingData;
  std::vector<std::shared_ptr<RenderingData>> mPreviousRenderingData;

  mutable Dali::Mutex mMutex;              ///< Mutex. We can lock mRenderingDataMutex under this scope.
  mutable Dali::Mutex mRenderingDataMutex; ///< Mutex. We cannot lock any mutex under this scope.

  Dali::Renderer                      mRenderer;                   ///< Renderer
  std::unique_ptr<rlottie::Animation> mVectorRenderer;             ///< The vector animation renderer
  UploadCompletedSignalType           mUploadCompletedSignal;      ///< Upload completed signal
  uint32_t                            mTotalFrameNumber;           ///< The total frame number
  uint32_t                            mDefaultWidth;               ///< The width of the surface
  uint32_t                            mDefaultHeight;              ///< The height of the surface
  float                               mFrameRate;                  ///< The frame rate of the content
  bool                                mLoadFailed : 1;             ///< Whether the file is loaded
  bool                                mResourceReady : 1;          ///< Whether the resource is ready
  bool                                mResourceReadyTriggered : 1; ///< Whether the resource ready is triggered
  bool                                mEnableFixedCache : 1;
  bool                                mFinalized : 1;
};

} // namespace Plugin

} // namespace Dali

#endif // DALI_EXTENSION_VECTOR_ANIMATION_RENDERER_H
