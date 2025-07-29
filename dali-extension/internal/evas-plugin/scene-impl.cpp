/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
// CLASS HEADER
#include <dali-extension/internal/evas-plugin/scene-impl.h>

// EXTERNAL INCLUDES
#include <dali-toolkit/devel-api/accessibility-manager/accessibility-manager.h>
#include <dali-toolkit/devel-api/focus-manager/keyinput-focus-manager.h>
#include <dali/devel-api/adaptor-framework/clipboard.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/native-render-surface-factory.h>
#include <dali/integration-api/adaptor-framework/native-render-surface.h>
#include <dali/integration-api/adaptor-framework/trigger-event-factory.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/actors/layer.h>
#include <tbm_surface_internal.h>
#include <tbm_surface_queue.h>

// INTERNAL INCLUDES
#include <dali-extension/internal/evas-plugin/evas-event-handler.h>
#include <dali-extension/internal/evas-plugin/evas-plugin-impl.h>
#include <dali-extension/internal/evas-plugin/evas-wrapper.h>

namespace Dali
{

namespace Extension
{

namespace Internal
{

IntrusivePtr<Scene> Scene::New(Evas_Object* parentEvasObject, uint16_t width, uint16_t height, bool isTranslucent)
{
  IntrusivePtr<Scene> scene = new Scene(parentEvasObject, width, height, isTranslucent);
  return scene;
}

Scene::Scene(Evas_Object* parentEvasObject, uint16_t width, uint16_t height, bool isTranslucent)
: mAdaptor(nullptr),
  mEvasWrapper(new EvasWrapper(parentEvasObject, width, height, isTranslucent)),
  mEvasEventHandler(),
  mRenderNotification(),
  mIsFocus(false),
  mIsTranslucent(isTranslucent),
  mConsumeSurface(nullptr)
{
  DALI_ASSERT_ALWAYS(parentEvasObject && "No parent object for the scene");

  // Create surface
  Any surface;
  mSurface = std::unique_ptr<RenderSurfaceInterface>(CreateNativeSurface(SurfaceSize(width, height), surface, isTranslucent));
}

void Scene::Initialize(EvasPlugin* evasPlugin, bool isDefaultScene)
{
  mAdaptor = evasPlugin->GetAdaptor();

  DALI_ASSERT_ALWAYS(mAdaptor && "Scene can not be created when the Adaptor is null");

  if(isDefaultScene)
  {
    Initialize();
    return;
  }

  if(evasPlugin->GetState() != EvasPlugin::RUNNING)
  {
    evasPlugin->PreInitSignal().Connect(this, &Scene::OnPreInitEvasPlugin);

    return;
  }

  Dali::Integration::SceneHolder sceneHolderHandler = Dali::Integration::SceneHolder(this);
  mAdaptor->AddWindow(sceneHolderHandler);

  Initialize();
}

void Scene::Initialize()
{
  // Connect callback to be notified when the surface is rendered
  TriggerEventFactory triggerEventFactory;

  mRenderNotification = std::move(triggerEventFactory.CreateTriggerEvent(MakeCallback(this, &Scene::OnPostRender), TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER));
  DALI_LOG_DEBUG_INFO("mRenderNotification Trigger Id(%u)\n", mRenderNotification->GetId());

  NativeRenderSurface* surface = GetNativeRenderSurface();

  if(!surface)
  {
    return;
  }

  surface->SetRenderNotification(mRenderNotification.get());

  if(!mEvasEventHandler)
  {
    mEvasEventHandler = std::unique_ptr<EvasEventHandler>(new EvasEventHandler(*this));
  }
}

void Scene::OnPreInitEvasPlugin()
{
  Dali::Integration::SceneHolder sceneHolderHandler = Dali::Integration::SceneHolder(this);
  mAdaptor->AddWindow(sceneHolderHandler);

  Initialize();
}

Scene::~Scene()
{
  NativeRenderSurface* surface = GetNativeRenderSurface();

  if(surface)
  {
    // To prevent notification triggering in NativeRenderSurface::PostRender while deleting SceneHolder
    surface->SetRenderNotification(nullptr);
  }
}

uint32_t Scene::GetLayerCount() const
{
  return mScene.GetLayerCount();
}

Layer Scene::GetLayer(uint32_t depth) const
{
  return mScene.GetLayer(depth);
}

Scene::SceneSize Scene::GetSize() const
{
  Size size = mScene.GetSize();

  return Scene::SceneSize(static_cast<uint16_t>(size.width), static_cast<uint16_t>(size.height));
}

Dali::Any Scene::GetNativeHandle() const
{
  return mEvasWrapper->GetNativeWindow();
}

NativeRenderSurface* Scene::GetNativeRenderSurface() const
{
  return dynamic_cast<NativeRenderSurface*>(mSurface.get());
}

Evas_Object* Scene::GetAccessEvasObject()
{
  return mEvasWrapper->GetAccessibilityTarget();
}

Evas_Object* Scene::GetDaliEvasObject()
{
  return mEvasWrapper->GetFocusTarget();
}

void Scene::ResizeSurface(uint16_t width, uint16_t height)
{
  if(!mSurface || !mAdaptor || width <= 1 || height <= 1)
  {
    return;
  }

  int intWidth  = static_cast<int>(width);
  int intHeight = static_cast<int>(height);

  PositionSize currentSize = mSurface->GetPositionSize();
  if(currentSize.width == intWidth && currentSize.height == intHeight)
  {
    return;
  }

  mSurface->MoveResize(PositionSize(0, 0, intWidth, intHeight));

  SurfaceResized();

  Adaptor::SurfaceSize newSize(width, height);

  // When surface size is updated, inform adaptor of resizing and emit ResizedSignal
  mAdaptor->SurfaceResizePrepare(mSurface.get(), newSize);

  mResizedSignal.Emit(Extension::Scene(this), width, height);

  mAdaptor->SurfaceResizeComplete(mSurface.get(), newSize);
}

void Scene::OnPostRender()
{
  // Bind offscreen surface to the evas object
  NativeRenderSurface* surface = GetNativeRenderSurface();

  if(!surface)
  {
    return;
  }

  tbm_surface_queue_h tbmQueue = AnyCast<tbm_surface_queue_h>(surface->GetNativeRenderable());

  if(tbm_surface_queue_can_acquire(tbmQueue, 1))
  {
    tbm_surface_h oldSurface = mConsumeSurface;

    if(tbm_surface_queue_acquire(tbmQueue, &mConsumeSurface) != TBM_SURFACE_QUEUE_ERROR_NONE)
    {
      DALI_LOG_ERROR("Failed to acquire a tbm_surface\n");
      return;
    }

    if(mConsumeSurface)
    {
      tbm_surface_internal_ref(mConsumeSurface);

      tbm_surface_internal_unref(oldSurface);
      if(tbm_surface_internal_is_valid(oldSurface))
      {
        tbm_surface_queue_release(tbmQueue, oldSurface);
      }
    }
  }

  mEvasWrapper->BindTBMSurface(mConsumeSurface);

  mEvasWrapper->RequestRender();

  surface->ReleaseLock();
}

EvasWrapper* Scene::GetEvasWrapper() const
{
  return mEvasWrapper.get();
}

void Scene::OnEvasObjectTouchEvent(Dali::Integration::Point& touchPoint, unsigned long timeStamp)
{
  FeedTouchPoint(touchPoint, timeStamp);
}

void Scene::OnEvasObjectWheelEvent(Dali::Integration::WheelEvent& wheelEvent)
{
  FeedWheelEvent(wheelEvent);
}

void Scene::OnEvasObjectKeyEvent(Dali::Integration::KeyEvent& keyEvent)
{
  FeedKeyEvent(keyEvent);
}

void Scene::OnEvasObjectMove(const Rect<int>& geometry)
{
}

void Scene::OnEvasObjectResize(const Rect<int>& geometry)
{
  ResizeSurface(static_cast<uint16_t>(geometry.width), static_cast<uint16_t>(geometry.height));
}

void Scene::OnEvasObjectVisiblityChanged(bool visible)
{
  if(mVisible == visible)
  {
    return;
  }
  DALI_LOG_RELEASE_INFO("Scene::OnEvasObjectVisiblityChanged( %s )", visible ? "T" : "F");

  SetVisibility(visible);
}

void Scene::OnEvasObjectFocusIn()
{
  if(!mIsFocus)
  {
    mFocusChangedSignal.Emit(Extension::Scene(this), true);

    mIsFocus = true;
  }
}

void Scene::OnEvasObjectFocusOut()
{
  if(mIsFocus)
  {
    mIsFocus = false;

    Toolkit::KeyInputFocusManager focusManager   = Toolkit::KeyInputFocusManager::Get();
    Toolkit::Control              currentFocused = focusManager.GetCurrentFocusControl();
    if(currentFocused)
    {
      focusManager.RemoveFocus(currentFocused);
    }

    Clipboard::Get().HideClipboard();

    mFocusChangedSignal.Emit(Extension::Scene(this), false);
  }
}

bool Scene::OnElmAccessibilityActionEvent(AccessActionInfo& accessActionInfo)
{
  return false;
}

void Scene::OnEcoreWl2VisibilityChange(bool visible)
{
  DALI_LOG_RELEASE_INFO("Scene::OnEcoreWl2VisibilityChange( %s )", visible ? "T" : "F");

  SetVisibility(visible);
}

void Scene::SetVisibility(bool visible)
{
  if(mVisible == visible)
  {
    return;
  }

  mVisible = visible;

  if(!mAdaptor)
  {
    return;
  }

  if(mVisible)
  {
    mAdaptor->OnWindowShown();
  }
  else
  {
    mAdaptor->OnWindowHidden();

    mSurface->ReleaseLock();
  }

  mVisibilityChangedSignal.Emit(Extension::Scene(this), mVisible);
}

} // namespace Internal

} // namespace Extension

} // namespace Dali
