#ifndef DALI_EXTENSION_INTERNAL_RIVE_ANIMATION_VIEW_H
#define DALI_EXTENSION_INTERNAL_RIVE_ANIMATION_VIEW_H

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
#include <dali/devel-api/actors/actor-devel.h>
#include <dali/public-api/adaptor-framework/window.h>
#include <dali/public-api/object/property-notification.h>
#include <dali-toolkit/public-api/controls/control-impl.h>

// INTERNAL INCLUDES
#include <dali-extension/devel-api/rive-animation-view/rive-animation-view.h>
#include <dali-extension/internal/rive-animation-view/rive-animation-task.h>
#include <dali-extension/internal/rive-animation-view/rive-animation-manager.h>
#include <dali-extension/internal/rive-animation-view/rive-animation-task.h>

namespace Dali
{
namespace Extension
{
namespace Internal
{

class RiveAnimationView : public Toolkit::Internal::Control, public RiveAnimationManager::LifecycleObserver
{
protected:
  /**
   * Construct a new RiveAnimationView.
   */
  RiveAnimationView();

  /**
   * A reference counted object may only be deleted by calling Unreference()
   */
  virtual ~RiveAnimationView();

public:
  /**
   * @brief Create a new RiveAnimationView.
   *
   * @return A smart-pointer to the newly allocated RiveAnimationView.
   */
  static Dali::Extension::RiveAnimationView New();

  /**
   * @copydoc Dali::Extension::RiveAnimationView::PlayAnimation
   */
  void PlayAnimation();

  /**
   * @copydoc Dali::Extension::RiveAnimationView::StopAnimation
   */
  void StopAnimation();

  /**
   * @copydoc Dali::Extension::RiveAnimationView::PauseAnimation
   */
  void PauseAnimation();

  /**
   * @copydoc Dali::Extension::RiveAnimationView::EnableAnimation
   */
  void EnableAnimation(const std::string& animationName, bool enable);

  /**
   * @copydoc Dali::Extension::RiveAnimationView::SetShapeFillColor
   */
  void SetShapeFillColor(const std::string& fillName, Vector4 color);

  /**
   * @copydoc Dali::Extension::RiveAnimationView::SetShapeStrokeColor
   */
  void SetShapeStrokeColor(const std::string& strokeName, Vector4 color);

  /**
   * @copydoc Dali::Extension::RiveAnimationView::SetNodeOpacity
   */
  void SetNodeOpacity(const std::string& nodeName, float opacity);

  /**
   * @copydoc Dali::Extension::RiveAnimationView::SetNodeScale
   */
  void SetNodeScale(const std::string& nodeName, Vector2 scale);

  /**
   * @copydoc Dali::Extension::RiveAnimationView::SetNodeRotation
   */
  void SetNodeRotation(const std::string& nodeName, Degree degree);

  /**
   * @copydoc Dali::Extension::RiveAnimationView::SetNodePosition
   */
  void SetNodePosition(const std::string& nodeName, Vector2 position);

  /**
   * @copydoc Dali::Extension::RiveAnimationView::AnimationFinishedSignal
   */
  Dali::Extension::RiveAnimationView::AnimationSignalType& AnimationFinishedSignal();

  // Properties
  /**
   * Called when a property of an object of this type is set.
   * @param[in] object The object whose property is set.
   * @param[in] index The property index.
   * @param[in] value The new property value.
   */
  static void SetProperty(BaseObject* object, Property::Index index, const Property::Value& value);

  /**
   * Called to retrieve a property of an object of this type.
   * @param[in] object The object whose property is to be retrieved.
   * @param[in] index The property index.
   * @return The current value of the property.
   */
  static Property::Value GetProperty(BaseObject* object, Property::Index propertyIndex);

private: // From CustomActorImpl
  /**
   * @copydoc CustomActorImpl::OnSceneConnection()
   */
  void OnSceneConnection(int depth) override;

  /**
   * @copydoc CustomActorImpl::OnSceneDisconnection()
   */
  void OnSceneDisconnection() override;

private: // From Control
  /**
   * @copydoc Toolkit::Control::OnInitialize
   */
  void OnInitialize() override;

  /**
   * @copydoc Toolkit::Control::GetNaturalSize
   */
  Vector3 GetNaturalSize() override;

  /**
   * @copydoc Toolkit::Control::OnRelayout()
   */
  void OnRelayout(const Vector2& size, RelayoutContainer& container) override;

private: // From RiveAnimationManager::LifecycleObserver:
  /**
   * @copydoc RiveAnimationManager::LifecycleObserver::RiveAnimationManagerDestroyed()
   */
  void RiveAnimationManagerDestroyed() override;

private:
  /**
   * @brief Set an image url.
   *
   * @param[in] url The url of the image resource to display
   */
  void SetUrl(const std::string& url);

  /**
   * @brief Called when the texture upload is completed.
   */
  void OnUploadCompleted();

  /**
   * @brief Event callback from rasterize thread. This is called after the animation is finished.
   */
  void OnAnimationFinished();

  /**
   * @brief Send animation data to the rasterize thread.
   */
  void SendAnimationData();

  /**
   * @brief Clear animations data.
   */
  void ClearAnimationsData();

  /**
   * @brief Set the vector image size.
   */
  void SetVectorImageSize();

  /**
   * @brief Trigger rasterization of the vector content.
   */
  void TriggerVectorRasterization();

  /**
   * @brief Callback when the world scale factor changes.
   */
  void OnScaleNotification(PropertyNotification& source);

  /**
   * @brief Callback when the size changes.
   */
  void OnSizeNotification(PropertyNotification& source);

  /**
   * @brief Callback when the visibility of the actor is changed.
   */
  void OnControlVisibilityChanged(Actor actor, bool visible, DevelActor::VisibilityChange::Type type);

  /**
   * @brief Callback when the visibility of the window is changed.
   */
  void OnWindowVisibilityChanged(Window window, bool visible);

  /**
   * @brief Callback when the event is processed.
   */
  void OnProcessEvents();

private:
  // Undefined
  RiveAnimationView(const RiveAnimationView&);
  RiveAnimationView& operator=(const RiveAnimationView&);

private:
  std::string                                             mUrl{};
  RiveAnimationTask::AnimationData                        mAnimationData{};
  RiveAnimationTaskPtr                                    mRiveAnimationTask{};
  PropertyNotification                                    mScaleNotification;
  PropertyNotification                                    mSizeNotification;
  Dali::Renderer                                          mRenderer;
  Dali::Extension::RiveAnimationView::AnimationSignalType mFinishedSignal{};
  Vector2                                                 mSize{};
  Vector2                                                 mScale{};
  Dali::Extension::RiveAnimationView::PlayState           mPlayState{Dali::Extension::RiveAnimationView::PlayState::STOPPED};
  CallbackBase*                                           mEventCallback{nullptr}; // Not owned
  bool                                                    mLoadFailed{false};
  bool                                                    mRendererAdded{false};
  bool                                                    mCoreShutdown{false};
};

} // namespace Internal

// Helpers for public-api forwarding methods
inline Extension::Internal::RiveAnimationView& GetImplementation(Extension::RiveAnimationView& obj)
{
  DALI_ASSERT_ALWAYS(obj);
  Dali::RefObject& handle = obj.GetImplementation();
  return static_cast<Extension::Internal::RiveAnimationView&>(handle);
}

inline const Extension::Internal::RiveAnimationView& GetImplementation(const Extension::RiveAnimationView& obj)
{
  DALI_ASSERT_ALWAYS(obj);
  const Dali::RefObject& handle = obj.GetImplementation();
  return static_cast<const Extension::Internal::RiveAnimationView&>(handle);
}

} // namespace Extension

} // namespace Dali

#endif // DALI_EXTENSION_INTERNAL_RIVE_ANIMATION_VIEW_H
