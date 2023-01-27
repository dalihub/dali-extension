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

// CLASS HEADER
#include "rive-animation-view-impl.h"

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/devel-api/common/stage.h>
#include <dali/devel-api/rendering/renderer-devel.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/type-registry-helper.h>
#include <dali/public-api/object/type-registry.h>
#include <dali-toolkit/devel-api/controls/control-devel.h>

// INTERNAL INCLUDES
#include <dali-extension/devel-api/rive-animation-view/rive-animation-view.h>

namespace Dali
{
namespace Extension
{
namespace Internal
{
namespace
{

Geometry CreateQuadGeometry()
{
  const float halfWidth  = 0.5f;
  const float halfHeight = 0.5f;
  struct QuadVertex
  {
    Vector2 position;
  };
  QuadVertex quadVertexData[4] =
    {
      {Vector2(-halfWidth, -halfHeight)},
      {Vector2(-halfWidth, halfHeight)},
      {Vector2(halfWidth, -halfHeight)},
      {Vector2(halfWidth, halfHeight)}};

  Property::Map quadVertexFormat;
  quadVertexFormat["aPosition"] = Property::VECTOR2;
  VertexBuffer quadVertices     = VertexBuffer::New(quadVertexFormat);
  quadVertices.SetData(quadVertexData, 4);

  // Create the geometry object
  Geometry geometry = Geometry::New();
  geometry.AddVertexBuffer(quadVertices);
  geometry.SetType(Geometry::TRIANGLE_STRIP);

  return geometry;
}

BaseHandle Create()
{
  return Extension::RiveAnimationView::New();
}

// Setup properties, signals and actions using the type-registry.
DALI_TYPE_REGISTRATION_BEGIN(Extension::RiveAnimationView, Toolkit::Control, Create);
DALI_PROPERTY_REGISTRATION(Extension, RiveAnimationView, "url", STRING, URL)
DALI_PROPERTY_REGISTRATION(Extension, RiveAnimationView, "playState", INTEGER, PLAY_STATE)
DALI_TYPE_REGISTRATION_END()

const char* VERTEX_SHADER = DALI_COMPOSE_SHADER(
  attribute mediump vec2 aPosition;\n
  uniform highp   mat4 uMvpMatrix;\n
  uniform highp   vec3 uSize;\n
  varying mediump vec2 vTexCoord;\n
  \n
  void main()\n
  {\n
    gl_Position = uMvpMatrix * vec4(aPosition * uSize.xy, 0.0, 1.0);\n
    vTexCoord = aPosition + vec2(0.5);\n
  }\n
);

const char* FRAGMENT_SHADER = DALI_COMPOSE_SHADER(
  varying mediump vec2 vTexCoord;\n
  uniform sampler2D sTexture;\n
  uniform lowp vec4 uColor;\n
  \n
  void main()\n
  {\n
      gl_FragColor = texture2D( sTexture, vTexCoord ) * uColor;\n
  }\n
);

#if defined(DEBUG_ENABLED)
Debug::Filter* gRiveAnimationLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_RIVE_ANIMATION");
#endif

} // unnamed namespace

RiveAnimationView::RiveAnimationView()
: Control(ControlBehaviour(CONTROL_BEHAVIOUR_DEFAULT)),
  mRiveAnimationTask(new RiveAnimationTask())
{
}

RiveAnimationView::~RiveAnimationView()
{
  if(!mCoreShutdown)
  {
    auto& riveAnimationManager = RiveAnimationManager::GetInstance();
    riveAnimationManager.RemoveObserver(*this);

    if(mEventCallback)
    {
      riveAnimationManager.UnregisterEventCallback(mEventCallback);
    }

    // Finalize animation task and disconnect the signal in the main thread
    mRiveAnimationTask->UploadCompletedSignal().Disconnect(this, &RiveAnimationView::OnUploadCompleted);
    mRiveAnimationTask->Finalize();
  }
}

Extension::RiveAnimationView RiveAnimationView::New()
{
  RiveAnimationView* impl = new RiveAnimationView();

  Dali::Extension::RiveAnimationView handle = Dali::Extension::RiveAnimationView(*impl);

  // Second-phase init of the implementation
  // This can only be done after the CustomActor connection has been made...
  impl->Initialize();

  return handle;
}

void RiveAnimationView::OnSceneConnection(int depth)
{
  Control::OnSceneConnection(depth);

  if(mLoadFailed)
  {
    //TODO: do something
  }
  else
  {
    mRiveAnimationTask->SetRenderer(mRenderer);
    Actor actor = Self();

    // Add property notification for scaling & size
    mScaleNotification = actor.AddPropertyNotification(Actor::Property::WORLD_SCALE, StepCondition(0.1f, 1.0f));
    mScaleNotification.NotifySignal().Connect(this, &RiveAnimationView::OnScaleNotification);

    mSizeNotification = actor.AddPropertyNotification(Actor::Property::SIZE, StepCondition(3.0f));
    mSizeNotification.NotifySignal().Connect(this, &RiveAnimationView::OnSizeNotification);

    DevelActor::VisibilityChangedSignal(actor).Connect(this, &RiveAnimationView::OnControlVisibilityChanged);

    Window window = DevelWindow::Get(actor);
    if(window)
    {
      DevelWindow::VisibilityChangedSignal(window).Connect(this, &RiveAnimationView::OnWindowVisibilityChanged);
    }
  }

  DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationView::OnSceneConnection [%p]\n", this);

}

void RiveAnimationView::OnSceneDisconnection()
{
  Control::OnSceneDisconnection();

  StopAnimation();
  SendAnimationData();

  Actor actor = Self();

  if(mRenderer)
  {
    actor.RemoveRenderer(mRenderer);
    mRendererAdded = false;
  }

  // Remove property notification
  actor.RemovePropertyNotification(mScaleNotification);
  actor.RemovePropertyNotification(mSizeNotification);

  DevelActor::VisibilityChangedSignal(actor).Disconnect(this, &RiveAnimationView::OnControlVisibilityChanged);

  Window window = DevelWindow::Get(actor);
  if(window)
  {
    DevelWindow::VisibilityChangedSignal(window).Disconnect(this, &RiveAnimationView::OnWindowVisibilityChanged);
  }

  // Reset the visual size to zero so that when adding the actor back to stage the rasterization is forced
  mSize  = Vector2::ZERO;
  mScale = Vector2::ONE;

  DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationView::OnSceneDisconnection [%p]\n", this);
}

void RiveAnimationView::OnInitialize()
{
  // Accessibility
  Self().SetProperty(Toolkit::DevelControl::Property::ACCESSIBILITY_ROLE, Dali::Accessibility::Role::IMAGE);
  Self().SetProperty(Toolkit::DevelControl::Property::ACCESSIBILITY_HIGHLIGHTABLE, true);
}

Vector3 RiveAnimationView::GetNaturalSize()
{
  Vector3 naturalSize;

  if(mSize != Vector2::ZERO)
  {
    naturalSize = mSize;
  }
  else
  {
    uint32_t width, height;
    mRiveAnimationTask->GetDefaultSize(width, height);
    naturalSize.x = width;
    naturalSize.y = height;
  }

  DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationView::GetNaturalSize: w = %f, h = %f [%p]\n", naturalSize.width, naturalSize.height, this);

  return naturalSize;
}

void RiveAnimationView::OnRelayout(const Vector2& size, RelayoutContainer& container)
{
  Control::OnRelayout(size, container);

  if(Self()[Actor::Property::CONNECTED_TO_SCENE] && size != mSize)
  {
    DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationView::OnRelayout: width = %f, height = %f [%p]\n", size.width, size.height, this);

    mSize = size;

    SetVectorImageSize();

    if(mPlayState == Dali::Extension::RiveAnimationView::PlayState::PLAYING && mAnimationData.playState != Dali::Extension::RiveAnimationView::PlayState::PLAYING)
    {
      mAnimationData.playState = Dali::Extension::RiveAnimationView::PlayState::PLAYING;
      mAnimationData.resendFlag |= RiveAnimationTask::RESEND_PLAY_STATE;
    }

    SendAnimationData();
  }
}

///////////////////////////////////////////////////////////
//
// Properties
//

void RiveAnimationView::SetProperty(BaseObject* object, Property::Index index, const Property::Value& value)
{
  Dali::Extension::RiveAnimationView riveAnimationView = Dali::Extension::RiveAnimationView::DownCast(Dali::BaseHandle(object));

  if(riveAnimationView)
  {
    RiveAnimationView& impl = GetImplementation(riveAnimationView);
    switch(index)
    {
      case Dali::Extension::RiveAnimationView::Property::URL:
      {
        std::string url;
        if(value.Get(url))
        {
          impl.SetUrl(url);
        }
        break;
      }
    }
  }
}

Property::Value RiveAnimationView::GetProperty(BaseObject* object, Property::Index propertyIndex)
{
  Property::Value value;

  Dali::Extension::RiveAnimationView riveAnimationView = Dali::Extension::RiveAnimationView::DownCast(Dali::BaseHandle(object));

  if(riveAnimationView)
  {
    RiveAnimationView& impl = GetImplementation(riveAnimationView);
    switch(propertyIndex)
    {
      case Dali::Extension::RiveAnimationView::Property::URL:
      {
        value = impl.mUrl;
        break;
      }
    }
  }

  return value;
}

void RiveAnimationView::PlayAnimation()
{
  Vector3 size = Self().GetProperty<Vector3>(Dali::Actor::Property::SIZE);

  if(Self()[Actor::Property::CONNECTED_TO_SCENE] && size != Vector3::ZERO)
  {
    if(mAnimationData.playState != Dali::Extension::RiveAnimationView::PlayState::PLAYING)
    {
      mAnimationData.playState = Dali::Extension::RiveAnimationView::PlayState::PLAYING;
      mAnimationData.resendFlag |= RiveAnimationTask::RESEND_PLAY_STATE;
    }
  }
  mPlayState = Dali::Extension::RiveAnimationView::PlayState::PLAYING;

  TriggerVectorRasterization();
}

void RiveAnimationView::StopAnimation()
{
  if(mAnimationData.playState != Dali::Extension::RiveAnimationView::PlayState::STOPPED)
  {
    mAnimationData.playState = Dali::Extension::RiveAnimationView::PlayState::STOPPED;
    mAnimationData.resendFlag |= RiveAnimationTask::RESEND_PLAY_STATE;
  }
  mPlayState = Dali::Extension::RiveAnimationView::PlayState::STOPPED;

  TriggerVectorRasterization();
}

void RiveAnimationView::PauseAnimation()
{
  if(mAnimationData.playState == Dali::Extension::RiveAnimationView::PlayState::PLAYING)
  {
    mAnimationData.playState = Dali::Extension::RiveAnimationView::PlayState::PAUSED;
    mAnimationData.resendFlag |= RiveAnimationTask::RESEND_PLAY_STATE;
  }
  mPlayState = Dali::Extension::RiveAnimationView::PlayState::PAUSED;

  TriggerVectorRasterization();
}

void RiveAnimationView::EnableAnimation(const std::string& animationName, bool enable)
{
  mAnimationData.animations.push_back(std::pair<std::string, bool>(animationName, enable));
  mAnimationData.resendFlag |= RiveAnimationTask::RESEND_ENABLE_ANIMATION;

  TriggerVectorRasterization();
}

void RiveAnimationView::SetAnimationElapsedTime(const std::string& animationName, float elapsed)
{
  mAnimationData.elapsedTimes.push_back(std::pair<std::string, float>(animationName, elapsed));
  mAnimationData.resendFlag |= RiveAnimationTask::RESEND_ANIMATION_ELAPSED_TIME;

  TriggerVectorRasterization();
}

void RiveAnimationView::SetShapeFillColor(const std::string& fillName, Vector4 color)
{
  mAnimationData.fillColors.push_back(std::pair<std::string, Vector4>(fillName, color));
  mAnimationData.resendFlag |= RiveAnimationTask::RESEND_FILL_COLOR;

  TriggerVectorRasterization();
}

void RiveAnimationView::SetShapeStrokeColor(const std::string& strokeName, Vector4 color)
{
  mAnimationData.strokeColors.push_back(std::pair<std::string, Vector4>(strokeName, color));
  mAnimationData.resendFlag |= RiveAnimationTask::RESEND_STROKE_COLOR;

  TriggerVectorRasterization();
}

void RiveAnimationView::SetNodeOpacity(const std::string& nodeName, float opacity)
{
  mAnimationData.opacities.push_back(std::pair<std::string, float>(nodeName, opacity));
  mAnimationData.resendFlag |= RiveAnimationTask::RESEND_OPACITY;

  TriggerVectorRasterization();
}

void RiveAnimationView::SetNodeScale(const std::string& nodeName, Vector2 scale)
{
  mAnimationData.scales.push_back(std::pair<std::string, Vector2>(nodeName, scale));
  mAnimationData.resendFlag |= RiveAnimationTask::RESEND_SCALE;

  TriggerVectorRasterization();
}

void RiveAnimationView::SetNodeRotation(const std::string& nodeName, Degree degree)
{
  mAnimationData.rotations.push_back(std::pair<std::string, Degree>(nodeName, degree));
  mAnimationData.resendFlag |= RiveAnimationTask::RESEND_ROTATION;

  TriggerVectorRasterization();
}

void RiveAnimationView::SetNodePosition(const std::string& nodeName, Vector2 position)
{
  mAnimationData.positions.push_back(std::pair<std::string, Vector2>(nodeName, position));
  mAnimationData.resendFlag |= RiveAnimationTask::RESEND_POSITION;

  TriggerVectorRasterization();
}

void RiveAnimationView::PointerMove(float x, float y)
{
  mRiveAnimationTask->PointerMove(x, y);
}

void RiveAnimationView::PointerDown(float x, float y)
{
  mRiveAnimationTask->PointerDown(x, y);
}

void RiveAnimationView::PointerUp(float x, float y)
{
  mRiveAnimationTask->PointerUp(x, y);
}

bool RiveAnimationView::SetNumberState(const std::string& stateMachineName, const std::string& inputName, float value)
{
  return mRiveAnimationTask->SetNumberState(stateMachineName, inputName, value);
}

bool RiveAnimationView::SetBooleanState(const std::string& stateMachineName, const std::string& inputName, bool value)
{
  return mRiveAnimationTask->SetBooleanState(stateMachineName, inputName, value);
}

bool RiveAnimationView::FireState(const std::string& stateMachineName, const std::string& inputName)
{
  return mRiveAnimationTask->FireState(stateMachineName, inputName);
}

Dali::Extension::RiveAnimationView::AnimationSignalType& RiveAnimationView::AnimationFinishedSignal()
{
  return mFinishedSignal;
}

void RiveAnimationView::RiveAnimationManagerDestroyed()
{
  mCoreShutdown = true;
}

void RiveAnimationView::SetUrl(const std::string& url)
{
  mUrl = url;

  if(!mRiveAnimationTask->Load(mUrl))
  {
    mLoadFailed = true;
  }

  mRiveAnimationTask->UploadCompletedSignal().Connect(this, &RiveAnimationView::OnUploadCompleted);
  mRiveAnimationTask->SetAnimationFinishedCallback(new EventThreadCallback(MakeCallback(this, &RiveAnimationView::OnAnimationFinished)));

  auto& vectorAnimationManager = RiveAnimationManager::GetInstance();
  vectorAnimationManager.AddObserver(*this);

  Geometry geometry = CreateQuadGeometry();
  Shader   shader   = Shader::New(VERTEX_SHADER, FRAGMENT_SHADER);
  mRenderer  = Renderer::New(geometry, shader);

  TextureSet textureSet = TextureSet::New();
  mRenderer.SetTextures(textureSet);

  mRenderer.SetProperty(Renderer::Property::BLEND_PRE_MULTIPLIED_ALPHA, true);

  mRiveAnimationTask->SetRenderer(mRenderer);

  TriggerVectorRasterization();
}

void RiveAnimationView::OnUploadCompleted()
{
  if(!mRendererAdded)
  {
    Self().AddRenderer(mRenderer);
    mRendererAdded = true;

    //TODO: do something - resource ready

    DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationView::OnUploadCompleted: Renderer is added [%p]\n", this);
  }
}

void RiveAnimationView::OnAnimationFinished()
{
  DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationView::OnAnimationFinished: action state = %d [%p]\n", mPlayState, this);

  if(mPlayState != Dali::Extension::RiveAnimationView::PlayState::STOPPED)
  {
    mPlayState = Dali::Extension::RiveAnimationView::PlayState::STOPPED;

    mAnimationData.playState = Dali::Extension::RiveAnimationView::PlayState::STOPPED;

    Dali::Extension::RiveAnimationView handle(GetOwner());
    mFinishedSignal.Emit(handle);
  }

  if(mRenderer)
  {
    mRenderer.SetProperty(DevelRenderer::Property::RENDERING_BEHAVIOR, DevelRenderer::Rendering::IF_REQUIRED);
  }
}

void RiveAnimationView::SendAnimationData()
{
  if(mAnimationData.resendFlag)
  {
    mRiveAnimationTask->SetAnimationData(mAnimationData);
    ClearAnimationsData();

    if(mRenderer)
    {
      if(mAnimationData.playState == Dali::Extension::RiveAnimationView::PlayState::PLAYING)
      {
        mRenderer.SetProperty(DevelRenderer::Property::RENDERING_BEHAVIOR, DevelRenderer::Rendering::CONTINUOUSLY);
      }
      else
      {
        mRenderer.SetProperty(DevelRenderer::Property::RENDERING_BEHAVIOR, DevelRenderer::Rendering::IF_REQUIRED);
      }
    }

    mAnimationData.resendFlag = 0;
  }
}

void RiveAnimationView::ClearAnimationsData()
{
    mAnimationData.animations.clear();
    mAnimationData.elapsedTimes.clear();
    mAnimationData.fillColors.clear();
    mAnimationData.strokeColors.clear();
    mAnimationData.opacities.clear();
    mAnimationData.scales.clear();
    mAnimationData.rotations.clear();
    mAnimationData.positions.clear();
}

void RiveAnimationView::SetVectorImageSize()
{
  uint32_t width  = static_cast<uint32_t>(mSize.width * mScale.width);
  uint32_t height = static_cast<uint32_t>(mSize.height * mScale.height);

  mAnimationData.width  = width;
  mAnimationData.height = height;
  mAnimationData.resendFlag |= RiveAnimationTask::RESEND_SIZE;
}

void RiveAnimationView::TriggerVectorRasterization()
{
  if(!mEventCallback && !mCoreShutdown)
  {
    mEventCallback             = MakeCallback(this, &RiveAnimationView::OnProcessEvents);
    auto& riveAnimationManager = RiveAnimationManager::GetInstance();
    riveAnimationManager.RegisterEventCallback(mEventCallback);
    Stage::GetCurrent().KeepRendering(0.0f); // Trigger event processing
  }
}

void RiveAnimationView::OnScaleNotification(PropertyNotification& source)
{
  Vector3 scale = Self().GetProperty<Vector3>(Actor::Property::WORLD_SCALE);

  if(scale.width >= 1.0f || scale.height >= 1.0f)
  {
    mScale.width  = scale.width;
    mScale.height = scale.height;

    DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationView::OnScaleNotification: scale = %f, %f [%p]\n", mScale.width, mScale.height, this);

    SetVectorImageSize();
    SendAnimationData();

    Stage::GetCurrent().KeepRendering(0.0f); // Trigger event processing
  }
}

void RiveAnimationView::OnSizeNotification(PropertyNotification& source)
{
  Vector3 size       = Self().GetCurrentProperty<Vector3>(Actor::Property::SIZE);
  mSize.width  = size.width;
  mSize.height = size.height;

  DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationView::OnSizeNotification: size = %f, %f [%p]\n", mSize.width, mSize.height, this);

  SetVectorImageSize();
  SendAnimationData();

  Stage::GetCurrent().KeepRendering(0.0f); // Trigger event processing
}

void RiveAnimationView::OnControlVisibilityChanged(Actor actor, bool visible, DevelActor::VisibilityChange::Type type)
{
  if(!visible)
  {
    StopAnimation();
    TriggerVectorRasterization();

    DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationView::OnControlVisibilityChanged: invisibile. Pause animation [%p]\n", this);
  }
}

void RiveAnimationView::OnWindowVisibilityChanged(Window window, bool visible)
{
  if(!visible)
  {
    StopAnimation();
    TriggerVectorRasterization();

    DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationView::OnWindowVisibilityChanged: invisibile. Pause animation [%p]\n", this);
  }
}

void RiveAnimationView::OnProcessEvents()
{
  SendAnimationData();

  mEventCallback = nullptr; // The callback will be deleted in the RiveAnimationManager
}

} // namespace Internal
} // namespace Extension
} // namespace Dali
