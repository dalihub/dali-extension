#ifndef DALI_EXTENSION_INTERNAL_RIVE_ANIMATION_RENDERER_H
#define DALI_EXTENSION_INTERNAL_RIVE_ANIMATION_RENDERER_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/threading/mutex.h>
#include <dali/integration-api/debug.h> ///< note : Debug::DebugPriority::DEBUG can be removed due to <rive/rive_types.hpp>.
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/rendering/renderer.h>
#include <tbm_surface.h>
#include <tbm_surface_queue.h>
#include <memory>

// RIVE - INCLUDES
#include <rive/animation/linear_animation_instance.hpp>
#include <rive/artboard.hpp>
#include <rive_tizen.hpp>

// INTERNAL INCLUDES
#include <dali-extension/internal/rive-animation-view/animation-renderer/rive-animation-renderer-event-handler.h>

namespace Dali
{
namespace Extension
{
namespace Internal
{
class RiveAnimationRenderer;
using RiveAnimationRendererPtr = IntrusivePtr<RiveAnimationRenderer>;

/**
 * @brief Implementation of the Rive animation renderer class which has Tizen platform dependency.
 */
class RiveAnimationRenderer : public RefObject, public RiveAnimationRendererEventHandler
{
public:
  /**
   * @brief UploadCompleted signal type.
   */
  using UploadCompletedSignalType = Signal<void()>;

  /**
   * @brief Constructor.
   */
  RiveAnimationRenderer();

  /**
   * @brief Destructor.
   */
  virtual ~RiveAnimationRenderer();

  /**
   * @brief Loads the animation file.
   *
   * @param[in] url The url of the vector animation file
   * @return True if loading success, false otherwise.
   */
  bool Load(const std::string& url);

  /**
   * @brief Finalizes the renderer. It will be called in the main thread.
   */
  void Finalize();

  /**
   * @brief Sets the renderer used to display the result image.
   *
   * @param[in] renderer The renderer used to display the result image
   */
  void SetRenderer(Renderer renderer);

  /**
   * @brief Sets the target image size.
   *
   * @param[in] width The target image width
   * @param[in] height The target image height
   */
  void SetSize(uint32_t width, uint32_t height);

  /**
   * @brief Renders the content to the target buffer synchronously.
   *
   * @param[in] elapsed The elapsed time
   * @return True if the rendering success, false otherwise.
   */
  bool Render(double elapsed);

  /**
   * @brief Gets the total number of frames of the file.
   *
   * @return The total number of frames
   */
  uint32_t GetTotalFrameNumber() const;

  /**
   * @brief Gets the frame rate of the file.
   *
   * @return The frame rate of the file
   */
  float GetFrameRate() const;

  /**
   * @brief Gets the default size of the file.
   *
   * @param[out] width The default width of the file
   * @param[out] height The default height of the file
   */
  void GetDefaultSize(uint32_t& width, uint32_t& height) const;

  /**
   * @brief Enables the animation state of given rive animation.
   *
   * @param[in] animationName The animation name
   * @param[in] enable The state of animation
   */
  void EnableAnimation(const std::string& animationName, bool enable);

  /**
   * @brief Sets the animation elapsed time.
   *
   * @param[in] animationName The animation name
   * @param[in] time The animation elapsed time (in second)
   */
  void SetAnimationElapsedTime(const std::string& animationName, float elapsed);

  /**
   * @brief Sets the shape fill color of given fill name.
   *
   * @param[in] fillName The fill name
   * @param[in] color The rgba color
   */
  void SetShapeFillColor(const std::string& fillName, Vector4 color);

  /**
   * @brief Sets the shape stroke color of given stroke name.
   *
   * @param[in] strokeName The stroke name
   * @param[in] color The rgba color
   */
  void SetShapeStrokeColor(const std::string& strokeName, Vector4 color);

  /**
   * @brief Sets the opacity of given node.
   *
   * @param[in] nodeName The node name
   * @param[in] opacity The opacity of given node
   */
  void SetNodeOpacity(const std::string& nodeName, float opacity);

  /**
   * @brief Sets the scale of given node.
   *
   * @param[in] nodeName The node name
   * @param[in] scale The scale of given node
   */
  void SetNodeScale(const std::string& nodeName, Vector2 scale);

  /**
   * @brief Sets the rotation of given node.
   *
   * @param[in] nodeName The node name
   * @param[in] degree The degree of given node
   */
  void SetNodeRotation(const std::string& nodeName, Degree degree);

  /**
   * @brief Sets the position of given node.
   *
   * @param[in] nodeName The node name
   * @param[in] position The position of given node
   */
  void SetNodePosition(const std::string& nodeName, Vector2 position);

  /**
   * @brief Inform current state machine that a pointing device coordinates are changed.
   *
   * @param[in] x, y The position of mouse or touch pointer
   */
  void PointerMove(float x, float y);

  /**
   * @brief Inform current state machine that a pointing device button is released.
   *
   * @param[in] x, y The position of mouse or touch pointer
   */
  void PointerDown(float x, float y);

  /**
   * @brief Inform current state machine that a pointing device button is pressed.
   *
   * @param[in] x, y The position of mouse or touch pointer
   */
  void PointerUp(float x, float y);

  /**
   * @brief Set input state on a number input.
   *
   * @param[in] stateMachineName Name of the stateMachine. Empty for default state machine.
   * @param[in] inputName Name of the input.
   * @param[in] value Number value
   * @return True if success, false otherwise.
   */
  bool SetNumberState(const std::string& stateMachineName, const std::string& inputName, float value);

  /**
   * @brief Set input state on a boolean input.
   *
   * @param[in] stateMachineName Name of the stateMachine. Empty for default state machine.
   * @param[in] inputName Name of the input.
   * @param[in] value Boolean value
   * @return True if success, false otherwise.
   */
  bool SetBooleanState(const std::string& stateMachineName, const std::string& inputName, bool value);

  /**
   * @brief Fire a trigger input.
   *
   * @param[in] stateMachineName Name of the stateMachine. Empty for default state machine.
   * @param[in] inputName Name of the input.
   * @return True if success, false otherwise.
   */
  bool FireState(const std::string& stateMachineName, const std::string& inputName);

  /**
   * @brief Ignores a rendered frame which is not shown yet.
   */
  void IgnoreRenderedFrame();

  /**
   * @brief Connect to this signal to be notified when the texture upload is completed.
   *
   * @return The signal to connect to.
   */
  UploadCompletedSignalType& UploadCompletedSignal();

protected: // Implementation of RiveAnimationRendererEventHandler
  /**
   * @copydoc Dali::Plugin::RiveAnimationRendererEventHandler::NotifyEvent()
   */
  void NotifyEvent() override;

private:
  /**
   * @brief Structure used to manage rive animations.
   */
  struct Animation
  {
  public:
    Animation(rive::LinearAnimationInstance* animationInstance, const std::string& animationName, bool animationEnable)
    : instance(animationInstance),
      name(animationName),
      enable(animationEnable),
      elapsed(-1.0f)
    {
    }
    std::unique_ptr<rive::LinearAnimationInstance> instance;
    const std::string&                             name;
    bool                                           enable;
    float                                          elapsed;
  };

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

  /**
   * @brief Clear Loaded Animations.
   */
  void ClearRiveAnimations();

private:
  std::string               mUrl;                    ///< The content file path
  mutable Dali::Mutex       mMutex;                  ///< Mutex
  Dali::Renderer            mRenderer;               ///< Renderer
  Dali::Texture             mTexture;                ///< Texture
  Dali::Texture             mRenderedTexture;        ///< Rendered Texture
  Dali::Texture             mPreviousTexture;        ///< Previous rendered texture
  NativeImageSourceQueuePtr mTargetSurface;          ///< The target surface
  UploadCompletedSignalType mUploadCompletedSignal;  ///< Upload completed signal
  tbm_surface_queue_h       mTbmQueue;               ///< Tbm surface queue handle
  rive::Artboard*           mArtboard;               ///< Rive artboard handle
  std::vector<Animation>    mAnimations;             ///< Rive animations
  rive::LinearAnimation*    mAnimation;              ///< Rive animation handle
  uint32_t                  mStartFrameNumber;       ///< The start frame number
  uint32_t                  mTotalFrameNumber;       ///< The total frame number
  uint32_t                  mWidth;                  ///< The width of the surface
  uint32_t                  mHeight;                 ///< The height of the surface
  uint32_t                  mDefaultWidth;           ///< The width of the surface
  uint32_t                  mDefaultHeight;          ///< The height of the surface
  float                     mFrameRate;              ///< The frame rate of the content
  bool                      mResourceReady;          ///< Whether the resource is ready
  bool                      mShaderChanged;          ///< Whether the shader is changed to support native image
  bool                      mResourceReadyTriggered; ///< Whether the resource ready is triggered
  RiveTizen*                mRiveTizenAdapter;       ///< Rive Tizen Adapter
};

} // namespace Internal

} // namespace Extension

} // namespace Dali

#endif // DALI_EXTENSION_INTERNAL_RIVE_ANIMATION_RENDERER_H
