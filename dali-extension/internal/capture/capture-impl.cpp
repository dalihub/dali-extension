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

#include <dali/integration-api/debug.h>
#include <dali/integration-api/adaptors/adaptor.h>

#include <fstream>
#include <string.h>

// INTERNAL INCLUDES
#include <dali-extension/internal/capture/capture-impl.h>

#define ENABLED_CAPTURE_LOGGING

#ifdef ENABLED_CAPTURE_LOGGING
#define DALI_CAPTURE_STATE(format, args...) Dali::Integration::Log::LogMessage(Dali::Integration::Log::DebugInfo, "%s:%d " format "\n", __PRETTY_FUNCTION__, __LINE__, ## args)
#else
#define DALI_CAPTURE_STATE(format, args...)
#endif

namespace
{
unsigned int TIME_OUT_DURATION = 1000;
}

namespace Dali
{

namespace Extension
{

namespace Internal
{

Capture::Capture()
: mProjectionMode(Dali::Camera::PERSPECTIVE_PROJECTION)
, mTbmSurface(NULL)
, mFinishState(Dali::Extension::Capture::FAILED)
{
}

Capture::Capture(Dali::Camera::ProjectionMode mode)
: mProjectionMode(mode)
, mTbmSurface(NULL)
, mFinishState(Dali::Extension::Capture::FAILED)
{
}

CapturePtr Capture::New()
{
  CapturePtr pWorker = new Capture();

  // Second-phase construction
  pWorker->Initialize();

  return pWorker;
}

CapturePtr Capture::New(Dali::Camera::ProjectionMode mode)
{
  CapturePtr pWorker = new Capture(mode);

  // Second-phase construction
  pWorker->Initialize();

  return pWorker;
}

void Capture::Start(Actor source, const Vector2& size, const std::string &path, const Vector4& clearColor)
{
  DALI_ASSERT_ALWAYS(path.size() > 4 && "Path is invalid.");

  // Increase the reference count focely to avoid application mistake.
  Reference();

  mPath = path;

  DALI_CAPTURE_STATE("Start Size[%.2f, %.2f] Path[%s]", size.width, size.height, path.c_str());

  DALI_ASSERT_ALWAYS(source && "Source is NULL.");

  UnsetResources();
  SetupResources(size, clearColor, source);
}

Dali::Extension::Capture::FinishState Capture::GetFinishState()
{
  return mFinishState;
}

Dali::Extension::Capture::CaptureSignalType& Capture::FinishedSignal()
{
  return mFinishedSignal;
}

void Capture::Initialize()
{
}

Capture::~Capture()
{
}

void Capture::CreateSurface(const Vector2& size)
{
  DALI_ASSERT_ALWAYS(!mTbmSurface && "mTbmSurface is already created.");

  mTbmSurface = tbm_surface_create(size.width, size.height, TBM_FORMAT_RGBA8888);
  DALI_CAPTURE_STATE("Create mTbmSurface[%p]", mTbmSurface);
}

void Capture::DeleteSurface()
{
  DALI_ASSERT_ALWAYS(mTbmSurface && "mTbmSurface is empty.");

  DALI_CAPTURE_STATE("Delete mTbmSurface[%p]", mTbmSurface);

  tbm_surface_destroy(mTbmSurface);
  mTbmSurface = NULL;
}

void Capture::ClearSurface(const Vector2& size)
{
  DALI_ASSERT_ALWAYS(mTbmSurface && "mTbmSurface is empty.");

  tbm_surface_info_s surface_info;

  if( tbm_surface_map( mTbmSurface, TBM_SURF_OPTION_WRITE, &surface_info) == TBM_SURFACE_ERROR_NONE )
  {
    //DALI_ASSERT_ALWAYS(surface_info.bpp == 32 && "unsupported tbm format");

    unsigned char* ptr = surface_info.planes[0].ptr;
    memset( ptr, 0, surface_info.size ); // TODO: support color

    if( tbm_surface_unmap( mTbmSurface ) != TBM_SURFACE_ERROR_NONE )
    {
      DALI_CAPTURE_STATE( "Fail to unmap tbm_surface\n" );
    }
  }
  else
  {
     DALI_ASSERT_ALWAYS(0 && "tbm_surface_map failed");
  }

  DALI_CAPTURE_STATE("Clear mTbmSurface[%p]", mTbmSurface);
}

bool Capture::IsSurfaceCreated()
{
  return mTbmSurface != 0;
}

void Capture::CreateNativeImageSource()
{
  Dali::Adaptor& adaptor = Dali::Adaptor::Get();

  DALI_ASSERT_ALWAYS(adaptor.IsAvailable() && "Dali::Adaptor is not available.");

  DALI_ASSERT_ALWAYS(mTbmSurface && "mTbmSurface is empty.");

  DALI_ASSERT_ALWAYS(!mNativeImageSourcePtr && "NativeImageSource is already created.");

  // create the NativeImageSource object with our surface
  mNativeImageSourcePtr = NativeImageSource::New(Dali::Any(mTbmSurface));

  DALI_CAPTURE_STATE("Create NativeImageSource[0x%X]", mNativeImageSourcePtr.Get());
}

void Capture::DeleteNativeImageSource()
{
  DALI_ASSERT_ALWAYS(mNativeImageSourcePtr && "mNativeImageSource is NULL.");

  DALI_CAPTURE_STATE("Delete NativeImageSource[0x%X]", mNativeImageSourcePtr.Get());

  mNativeImageSourcePtr.Reset();
}

bool Capture::IsNativeImageSourceCreated()
{
  return mNativeImageSourcePtr;
}

void Capture::CreateFrameBuffer()
{
  DALI_ASSERT_ALWAYS(mNativeImageSourcePtr && "NativeImageSource is NULL.");

  DALI_ASSERT_ALWAYS(!mFrameBuffer && "FrameBuffer is already created.");

  mNativeTexture = Texture::New( *mNativeImageSourcePtr );

  // Create a FrameBuffer object with no default attachments.
  mFrameBuffer = FrameBuffer::New( mNativeTexture.GetWidth(), mNativeTexture.GetHeight(), FrameBuffer::Attachment::NONE );
  // Add a color attachment to the FrameBuffer object.
  mFrameBuffer.AttachColorTexture( mNativeTexture );

  DALI_CAPTURE_STATE("Create FrameBuffer");
}

void Capture::DeleteFrameBuffer()
{
  DALI_ASSERT_ALWAYS(mFrameBuffer && "FrameBuffer is NULL.");

  DALI_CAPTURE_STATE("Delete FrameBuffer");

  mFrameBuffer.Reset();
  mNativeTexture.Reset();
}

bool Capture::IsFrameBufferCreated()
{
  return mFrameBuffer;
}

void Capture::SetupRenderTask(Actor source, const Vector4& clearColor)
{
  DALI_ASSERT_ALWAYS(source && "Source is empty.");

  mSource = source;

  // Check the original parent about source.
  mParent = mSource.GetParent();

  Stage stage = Stage::GetCurrent();
  Size stageSize = stage.GetSize();

  // Add to stage for rendering the source. If source isn't on the stage then it never be rendered.
  stage.Add(mSource);

  DALI_ASSERT_ALWAYS(!mCameraActor && "CameraActor is already created.");

  mCameraActor = CameraActor::New( stageSize );
  mCameraActor.SetParentOrigin(ParentOrigin::CENTER);
  mCameraActor.SetAnchorPoint(AnchorPoint::CENTER);

  if(mProjectionMode == Camera::ORTHOGRAPHIC_PROJECTION)
  {
	mCameraActor.SetOrthographicProjection(stageSize);
  }
  stage.Add(mCameraActor);

  DALI_ASSERT_ALWAYS(mFrameBuffer && "Framebuffer is NULL.");

  DALI_ASSERT_ALWAYS(!mRenderTask && "RenderTask is already created.");

  RenderTaskList taskList = stage.GetRenderTaskList();
  mRenderTask = taskList.CreateTask();
  mRenderTask.SetRefreshRate(RenderTask::REFRESH_ONCE);
  mRenderTask.SetSourceActor(source);
  mRenderTask.SetCameraActor(mCameraActor);
  mRenderTask.SetScreenToFrameBufferFunction(RenderTask::FULLSCREEN_FRAMEBUFFER_FUNCTION);
  mRenderTask.SetFrameBuffer(mFrameBuffer);
  mRenderTask.SetClearColor( clearColor );
  mRenderTask.SetClearEnabled( true );
  mRenderTask.SetProperty( RenderTask::Property::REQUIRES_SYNC, true );
  mRenderTask.FinishedSignal().Connect(this, &Capture::OnRenderFinished);
  mRenderTask.GetCameraActor().SetInvertYAxis( true );

  mTimer = Timer::New(TIME_OUT_DURATION);
  mTimer.TickSignal().Connect(this, &Capture::OnTimeOut);
  mTimer.Start();

  DALI_CAPTURE_STATE("Setup Camera and RenderTask.");
}

void Capture::UnsetRenderTask()
{
  DALI_ASSERT_ALWAYS(mCameraActor && "CameraActor is NULL.");

  DALI_CAPTURE_STATE("Unset Camera and RenderTask");

  if (mParent)
  {
    // Restore the parent of source.
    mParent.Add(mSource);
    mParent.Reset();
  }
  else
  {
    mSource.Unparent();
  }

  mSource.Reset();

  mTimer.Reset();

  mCameraActor.Unparent();
  mCameraActor.Reset();

  DALI_ASSERT_ALWAYS(mRenderTask && "RenderTask is NULL.");

  RenderTaskList taskList = Stage::GetCurrent().GetRenderTaskList();
  RenderTask firstTask = taskList.GetTask( 0u );

  // Stop rendering via frame-buffers as empty handle is used to clear target
  firstTask.SetFrameBuffer(FrameBuffer());

  taskList.RemoveTask(mRenderTask);
  mRenderTask.Reset();
}

bool Capture::IsRenderTaskSetup()
{
  return mCameraActor && mRenderTask;
}

void Capture::SetupResources(const Vector2& size, const Vector4& clearColor, Actor source)
{
  CreateSurface(size);
  ClearSurface(size);

  CreateNativeImageSource();

  CreateFrameBuffer();

  SetupRenderTask(source, clearColor);

  DALI_CAPTURE_STATE("Setup Resources");
}

void Capture::UnsetResources()
{
  if (IsRenderTaskSetup())
  {
    UnsetRenderTask();
  }

  if (IsFrameBufferCreated())
  {
    DeleteFrameBuffer();
  }

  if (IsNativeImageSourceCreated())
  {
    DeleteNativeImageSource();
  }

  if (IsSurfaceCreated())
  {
    DeleteSurface();
  }

  DALI_CAPTURE_STATE("Unset Resources");
}

void Capture::OnRenderFinished(RenderTask& task)
{
  DALI_CAPTURE_STATE("Render finished");

  mFinishState = Dali::Extension::Capture::SUCCESSED;

  mTimer.Stop();

  if (!Save())
  {
    mFinishState = Dali::Extension::Capture::FAILED;
    DALI_LOG_ERROR("Fail to Capture mTbmSurface[%p] Path[%s]", mTbmSurface, mPath.c_str());
  }

  Dali::Extension::Capture handle(this);
  mFinishedSignal.Emit(handle);

  UnsetResources();

  // Decrease the reference count forcely. It is increased at Start().
  Unreference();
}

bool Capture::OnTimeOut()
{
  DALI_CAPTURE_STATE("Timeout");

  mFinishState = Dali::Extension::Capture::FAILED;

  Dali::Extension::Capture handle(this);
  mFinishedSignal.Emit(handle);

  UnsetResources();

  // Decrease the reference count forcely. It is increased at Start().
  Unreference();

  return false;
}

bool Capture::Save()
{
  DALI_ASSERT_ALWAYS(mNativeImageSourcePtr && "mNativeImageSourcePtr is NULL");

  return mNativeImageSourcePtr->EncodeToFile(mPath);
}

}  // End of namespace Internal

}  // End of namespace Extension

}  // End of namespace Dali
