#ifndef DALI_EXTENSION_INTERNAL_SCENE_IMPL_H
#define DALI_EXTENSION_INTERNAL_SCENE_IMPL_H

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <memory>
#include <Ecore_IMF_Evas.h>

#include <dali/integration-api/adaptor-framework/scene-holder-impl.h>
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/signals/connection-tracker.h>

// INTERNAL INCLUDES
#include <dali-extension/internal/evas-plugin/evas-event-interface.h>
#include <dali-extension/devel-api/evas-plugin/scene.h>

namespace Dali
{
class Adaptor;
class Layer;
class NativeRenderSurface;
class TriggerEventInterface;

namespace Extension
{

namespace Internal
{

class EvasEventHandler;
class EvasWrapper;

/**
 * Implementation of the Scene class.
 */
class Scene : public Dali::Internal::Adaptor::SceneHolder,
              public ConnectionTracker,
              public Extension::Internal::EvasEventInterface
{
public:
  typedef Dali::Extension::Scene::SceneSize SceneSize;

  typedef Dali::Extension::Scene::VisibilityChangedSignalType VisibilityChangedSignalType;

  typedef Dali::Extension::Scene::FocusChangedSignalType FocusChangedSignalType;

  typedef Dali::Extension::Scene::ResizedSignalType ResizedSignalType;

  /**
   * @brief Create a new scene
   *
   * @param[in] parentEvasObject Parent Evas object of the new scene
   * @param[in] width The initial width of the scene
   * @param[in] height The initial height of the scene
   * @param[in] isTranslucent Whether the Evas object is translucent or not
   */
  static IntrusivePtr<Scene> New( Evas_Object* parentEvasObject, uint16_t width, uint16_t height, bool isTranslucent );

public:

  /**
   * @copydoc Dali::Extension::Scene::GetLayerCount
   */
  uint32_t GetLayerCount() const;

  /**
   * @copydoc Dali::Extension::Scene::GetLayer
   */
  Layer GetLayer( uint32_t depth ) const;

  /**
   * @copydoc Dali::Extension::Scene::GetSize
   */
  SceneSize GetSize() const;

  /**
   * @brief Gets the native handle.
   * @note When users call this function, it wraps the actual type used by the underlying system.
   * @return The native handle or an empty handle
   */
  Dali::Any GetNativeHandle() const override;

  /**
   * @copydoc Dali::Extension::Scene::GetAccessEvasObject
   */
  Evas_Object* GetAccessEvasObject();

  /**
   * @copydoc Dali::Extension::Scene::GetDaliEvasObject
   */
  Evas_Object* GetDaliEvasObject();

  /**
   * @copydoc Dali::Extension::Scene::VisibilityChangedSignal
   */
  VisibilityChangedSignalType& VisibilityChangedSignal()
  {
    return mVisibilityChangedSignal;
  }

  /**
   * @copydoc Dali::Extension::Scene::ResizedSignal
   */
  ResizedSignalType& ResizedSignal()
  {
    return mResizedSignal;
  }

  /**
   * @copydoc Dali::Extension::Scene::FocusChangedSignal
   */
  FocusChangedSignalType& FocusChangedSignal()
  {
    return mFocusChangedSignal;
  }

  /*
   * @brief Initialize the Scene
   * @param[in] evasPlugin The EvasPlugin instance to be used to intialize the new scene
   * @param[in] isDefaultScene Whether the Scene is a default one or not
   */
  void Initialize( EvasPlugin* evasPlugin, bool isDefaultScene );

private:

  /**
   * @brief Resize the surface
   * @param[in] width The width value
   * @param[in] height The height value
   */
  void ResizeSurface( uint16_t width, uint16_t height );

  /**
   * This function is called after drawing by dali.
   */
  void OnPostRender();

  /**
   * @brief Set visibility of the Scene
   * @param[in] visible The visibility
   */
  void SetVisibility( bool visible );

  /**
   * @brief Get the native render surface
   * @return The render surface
   */
  NativeRenderSurface* GetNativeRenderSurface() const;

private:

  /**
   * @copydoc Dali::Extension::Internal::EvasEventInterface::GetEvasWrapper
   */
  EvasWrapper* GetEvasWrapper() const override;

  /**
   * @copydoc Dali::Extension::Internal::EvasEventInterface::OnEvasObjectTouchEvent
   */
  void OnEvasObjectTouchEvent( Dali::Integration::Point& touchPoint, unsigned long timeStamp ) override;

  /**
   * @copydoc Dali::Extension::Internal::EvasEventInterface::OnEvasObjectWheelEvent
   */
  void OnEvasObjectWheelEvent( Dali::Integration::WheelEvent& wheelEvent ) override;

  /**
   * @copydoc Dali::Extension::Internal::EvasEventInterface::OnEvasObjectKeyEvent
   */
  void OnEvasObjectKeyEvent( Dali::Integration::KeyEvent& keyEvent ) override;

  /**
   * @copydoc Dali::Extension::Internal::EvasEventInterface::OnEvasObjectMove
   */
  void OnEvasObjectMove( const Rect<int>& geometry ) override;

  /**
   * @copydoc Dali::Extension::Internal::EvasEventInterface::OnEvasObjectResize
   */
  void OnEvasObjectResize( const Rect<int>& geometry ) override;

  /**
   * @copydoc Dali::Extension::Internal::EvasEventInterface::OnEvasObjectVisiblityChanged
   */
  void OnEvasObjectVisiblityChanged( bool visible ) override;

  /**
   * @copydoc Dali::Extension::Internal::EvasEventInterface::OnEvasObjectFocusIn
   */
  void OnEvasObjectFocusIn() override;

  /**
   * @copydoc Dali::Extension::Internal::EvasEventInterface::OnEvasObjectFocusOut
   */
  void OnEvasObjectFocusOut() override;

  /**
   * @copydoc Dali::Extension::Internal::EvasEventInterface::OnEvasRenderPost
   */
  void OnEvasPostRender() override;

  /**
   * @copydoc Dali::Extension::Internal::EvasEventInterface::OnElmAccessibilityActionEvent
   */
  bool OnElmAccessibilityActionEvent( AccessActionInfo& actionInfo ) override;

  /**
   * @copydoc Dali::Extension::Internal::EvasEventInterface::OnEcoreWl2VisibilityChange
   */
  void OnEcoreWl2VisibilityChange( bool visible ) override;

private:

  /**
   * Private constructor
   *
   * @param[in] parentEvasObject Parent Evas object of the new scene
   * @param[in] width The initial width of the scene
   * @param[in] height The initial height of the scene
   * @param[in] isTranslucent Whether the Evas object is translucent or not
   */
  Scene( Evas_Object* parentEvasObject, uint16_t width, uint16_t height, bool isTranslucent );

  /**
   * Destructor
   */
  virtual ~Scene();

  // Undefined
  Scene( const Scene& );
  Scene& operator=( Scene& );

  /**
   * This is for initialization of this Scene in case it is created before adaptor is running.
   */
  void OnPreInitEvasPlugin();

  /*
   * @brief Initialize the Scene (for internal use)
   */
  void Initialize();

private:

  Adaptor*                                  mAdaptor;

  std::unique_ptr< EvasWrapper >            mEvasWrapper;
  std::unique_ptr< EvasEventHandler >       mEvasEventHandler;
  std::unique_ptr< TriggerEventInterface >  mRenderNotification;

  ResizedSignalType                         mResizedSignal;
  VisibilityChangedSignalType               mVisibilityChangedSignal;
  FocusChangedSignalType                    mFocusChangedSignal;

  bool                                      mIsFocus;
  bool                                      mIsTranslucent;
};

inline Scene& GetImplementation( Extension::Scene& scene )
{
  DALI_ASSERT_ALWAYS( scene && "Extension::Scene handle is empty" );

  BaseObject& handle = scene.GetBaseObject();

  return static_cast<Scene&>( handle );
}

inline const Scene& GetImplementation( const Extension::Scene& scene )
{
  DALI_ASSERT_ALWAYS( scene && "Extension::Scene handle is empty" );

  const BaseObject& handle = scene.GetBaseObject();

  return static_cast<const Scene&>( handle );
}

} // namespace Internal

} // namespace Extension

} // namespace Dali

#endif // DALI_EXTENSION_INTERNAL_SCENE_IMPL_H
