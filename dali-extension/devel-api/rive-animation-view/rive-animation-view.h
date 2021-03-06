#ifndef DALI_EXTENSION_RIVE_ANIMATION_VIEW_H
#define DALI_EXTENSION_RIVE_ANIMATION_VIEW_H

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
#include <dali-toolkit/public-api/controls/control.h>

namespace Dali
{
namespace Extension
{
namespace Internal DALI_INTERNAL
{
class RiveAnimationView;
}

/**
 * @brief RiveAnimationView is a class for displaying a rive content.
 */
class DALI_IMPORT_API RiveAnimationView : public Toolkit::Control
{
public:
  /**
   * @brief Enumeration for the start and end property ranges for this control.
   */
  enum PropertyRange
  {
    PROPERTY_START_INDEX = Control::CONTROL_PROPERTY_END_INDEX + 1,
    PROPERTY_END_INDEX   = PROPERTY_START_INDEX + 1000,
  };

  /**
   * @brief Enumeration for the instance of properties belonging to the RiveAnimationView class.
   */
  struct Property
  {
    /**
     * @brief Enumeration for the instance of properties belonging to the RiveAnimationView class.
     */
    enum
    {
      /**
       * @brief The URL of the image.
       * @details Name "url", type Property::STRING
       */
      URL = PROPERTY_START_INDEX,

      /**
       * @brief The playing state the control will use.
       * @details Name "playState", Type PlayState::Type (Property::INTEGER)
       * @note This property is read-only.
       */
      PLAY_STATE,
    };
  };

  /**
   * @brief Animation signal type
   */
  using AnimationSignalType = Signal<void(RiveAnimationView)>;

  /**
   * @brief Enumeration for what state the animation is in.
   */
  enum class PlayState : uint8_t
  {
    STOPPED, ///< Animation has stopped
    PLAYING, ///< The animation is playing
    PAUSED   ///< The animation is paused
  };

public:
  /**
   * @brief Creates an uninitialized RiveAnimationView.
   */
  RiveAnimationView();

  /**
   * @brief Create an initialized RiveAnimationView.
   *
   * @return A handle to a newly allocated RiveAnimationView
   *
   * @note RiveAnimationView will not display anything.
   */
  static RiveAnimationView New();

  /**
   * @brief Creates an initialized RiveAnimationView from an URL to an image resource.
   *
   * @param[in] url The url of the image resource to display
   * @return A handle to a newly allocated RiveAnimationView
   */
  static RiveAnimationView New(const std::string& url);

  /**
   * @brief Destructor.
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~RiveAnimationView();

  /**
   * @brief Copy constructor.
   *
   * @param[in] riveAnimationView RiveAnimationView to copy. The copied RiveAnimationView will point at the same implementation
   */
  RiveAnimationView(const RiveAnimationView& riveAnimationView);

  /**
   * @brief Move constructor
   *
   * @param[in] rhs A reference to the moved handle
   */
  RiveAnimationView(RiveAnimationView&& rhs);

  /**
   * @brief Assignment operator.
   *
   * @param[in] riveAnimationView The RiveAnimationView to assign from
   * @return The updated RiveAnimationView
   */
  RiveAnimationView& operator=(const RiveAnimationView& riveAnimationView);

  /**
   * @brief Move assignment
   *
   * @param[in] rhs A reference to the moved handle
   * @return A reference to this
   */
  RiveAnimationView& operator=(RiveAnimationView&& rhs);

  /**
   * @brief Downcasts a handle to RiveAnimationView handle.
   *
   * If handle points to a RiveAnimationView, the downcast produces valid handle.
   * If not, the returned handle is left uninitialized.
   *
   * @param[in] handle Handle to an object
   * @return Handle to a RiveAnimationView or an uninitialized handle
   */
  static RiveAnimationView DownCast(BaseHandle handle);

  /**
   * @brief Play the rive animation.
   */
  void PlayAnimation();

  /**
   * @brief Stop the rive animation.
   */
  void StopAnimation();

  /**
   * @brief Pause the rive animation.
   */
  void PauseAnimation();

  /**
   * @brief Enables the animation state of given animation.
   *
   * @param[in] animationName The animation name
   * @param[in] enable The state of animation
   */
  void EnableAnimation(const std::string& animationName, bool enable);

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
   * @brief Connects to this signal to be notified when animations have finished.
   *
   * @return A signal object to connect with
   */
  AnimationSignalType& AnimationFinishedSignal();

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief Creates a handle using the Extension::Internal implementation.
   *
   * @param[in] implementation The RiveAnimationView implementation
   */
  DALI_INTERNAL RiveAnimationView(Internal::RiveAnimationView& implementation);

  /**
   * @brief Allows the creation of this RiveAnimationView from an Internal::CustomActor pointer.
   *
   * @param[in] internal A pointer to the internal CustomActor
   */
  DALI_INTERNAL RiveAnimationView(Dali::Internal::CustomActor* internal);
  /// @endcond
};

/**
 * @}
 */
} // namespace Extension

} // namespace Dali

#endif // DALI_EXTENSION_RIVE_ANIMATION_VIEW_H
