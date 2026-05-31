/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <tizen-camera-player.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/threading/mutex.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>

// The plugin factories
extern "C" DALI_EXPORT_API Dali::CameraPlayerPlugin* CreateCameraPlayerPlugin()
{
  return new Dali::Plugin::TizenCameraPlayer();
}

extern "C" DALI_EXPORT_API void DestroyCameraPlayerPlugin(Dali::CameraPlayerPlugin* plugin)
{
  if(plugin != NULL)
  {
    delete plugin;
  }
}
namespace Dali
{
namespace Plugin
{
namespace
{

const int TIMER_INTERVAL(20);

static void MediaPacketCameraPreviewCb(media_packet_h packet, void* user_data)
{
  TizenCameraPlayer* player = static_cast<TizenCameraPlayer*>(user_data);

  if(player == NULL)
  {
    DALI_LOG_ERROR("Preview callback got Null pointer as user_data.\n");
    return;
  }

  player->PushPacket(packet);
}

} // unnamed namespace

void CameraPlayerError(int error, const char* function, int line)
{
  if(error != CAMERA_ERROR_NONE)
  {
    switch(error)
    {
      case CAMERA_ERROR_INVALID_PARAMETER:
      {
        DALI_LOG_ERROR("Invalid parameter at %s (line %d)\n", function, line);
        return;
      }
      case CAMERA_ERROR_INVALID_STATE:
      {
        DALI_LOG_ERROR("Invalid state at %s (line %d)\n", function, line);
        return;
      }
      case CAMERA_ERROR_OUT_OF_MEMORY:
      {
        DALI_LOG_ERROR("Out of memory at %s (line %d)\n", function, line);
        return;
      }
      case CAMERA_ERROR_DEVICE:
      {
        DALI_LOG_ERROR("Device error at %s (line %d)\n", function, line);
        return;
      }
      case CAMERA_ERROR_INVALID_OPERATION:
      {
        DALI_LOG_ERROR("Internal error at %s (line %d)\n", function, line);
        return;
      }
      case CAMERA_ERROR_SECURITY_RESTRICTED:
      {
        DALI_LOG_ERROR("Restricted by security system policy at %s (line %d)\n", function, line);
        return;
      }
      case CAMERA_ERROR_DEVICE_BUSY:
      {
        DALI_LOG_ERROR("The device is using another application or working on some operation at %s (line %d)\n", function, line);
        return;
      }
      case CAMERA_ERROR_DEVICE_NOT_FOUND:
      {
        DALI_LOG_ERROR("No camera device at %s (line %d)\n", function, line);
        return;
      }
      case CAMERA_ERROR_ESD:
      {
        DALI_LOG_ERROR("ESD situation at %s (line %d)\n", function, line);
        return;
      }
      case CAMERA_ERROR_PERMISSION_DENIED:
      {
        DALI_LOG_ERROR("The access to the resources can not be granted at %s (line %d)\n", function, line);
        return;
      }
      case CAMERA_ERROR_NOT_SUPPORTED:
      {
        DALI_LOG_ERROR("The feature is not supported at %s (line %d)\n", function, line);
        return;
      }
      case CAMERA_ERROR_RESOURCE_CONFLICT:
      {
        DALI_LOG_ERROR("Blocked by resource conflict at %s (line %d)\n", function, line);
        return;
      }
      case CAMERA_ERROR_SERVICE_DISCONNECTED:
      {
        DALI_LOG_ERROR("Socket connection lost at %s (line %d)\n", function, line);
        return;
      }
      default:
      {
        DALI_LOG_ERROR("Unknown error code ( %d ) at %s (line %d)\n", error, function, line);
        return;
      }
    }
  }
}

TizenCameraPlayer::TizenCameraPlayer()
: mCameraPlayer(NULL),
  mCameraPlayerState(CAMERA_STATE_NONE),
  mTbmSurface(NULL),
  mPacket(NULL),
  mNativeImagePtr(NULL),
  mTimer(),
  mBackgroundColor(Dali::Vector4(1.0f, 1.0f, 1.0f, 0.0f)),
  mPacketMutex(),
  mPacketVector(),
#ifdef USE_TCORE_BACKEND
  mTcoreWlWindow(nullptr)
#else
  mEcoreWlWindow(nullptr)
#endif
{
}

TizenCameraPlayer::~TizenCameraPlayer()
{
  Destroy();
}

void TizenCameraPlayer::GetPlayerState(camera_state_e* state) const
{
  if(mCameraPlayer != NULL &&
     camera_get_state(mCameraPlayer, state) != CAMERA_ERROR_NONE)
  {
    DALI_LOG_ERROR("camera_get_state error: Invalid parameter\n");
    *state = CAMERA_STATE_NONE;
  }
}

void TizenCameraPlayer::SetWindowRenderingTarget(Dali::Window target)
{
  mNativeImagePtr = NULL;

#ifdef USE_TCORE_BACKEND
  InitializeUnderlayMode(Dali::AnyCast<tizen_core_wl_window_h>(target.GetNativeHandle()));
#else
  InitializeUnderlayMode(Dali::AnyCast<Ecore_Wl2_Window*>(target.GetNativeHandle()));
#endif
}

void TizenCameraPlayer::SetNativeImageRenderingTarget(Dali::NativeImagePtr target)
{
  mNativeImagePtr = NULL;

  InitializeTextureStreamMode(target);
}

void TizenCameraPlayer::StopPreview()
{
  GetPlayerState(&mCameraPlayerState);

  if(mCameraPlayerState == CAMERA_STATE_PREVIEW)
  {
    int error = camera_stop_preview(mCameraPlayer);
    CameraPlayerError(error, __FUNCTION__, __LINE__);

    if(mNativeImagePtr && mTimer)
    {
      mTimer.Stop();
      DestroyPackets();
    }
  }
}

void TizenCameraPlayer::Destroy()
{
  if(mNativeImagePtr && mTimer)
  {
    mTimer.Stop();
    DestroyPackets();

    GetPlayerState(&mCameraPlayerState);
    if(mCameraPlayerState != CAMERA_STATE_NONE)
    {
      int error = camera_unset_media_packet_preview_cb(mCameraPlayer);
      CameraPlayerError(error, __FUNCTION__, __LINE__);
    }
  }
}

void TizenCameraPlayer::SetCameraPlayer(Any handle)
{
  mCameraPlayer = static_cast<camera_h>(AnyCast<void*>(handle));
  GetPlayerState(&mCameraPlayerState);
}

void TizenCameraPlayer::InitializeTextureStreamMode(Dali::NativeImagePtr nativeImagePtr)
{
  int error;

  mNativeImagePtr = nativeImagePtr;

  GetPlayerState(&mCameraPlayerState);

  bool isPlay = false;
  if(mCameraPlayerState == CAMERA_STATE_PREVIEW)
  {
    isPlay = true;
    StopPreview();
  }

  GetPlayerState(&mCameraPlayerState);

  if(mCameraPlayerState == CAMERA_STATE_CREATED && mNativeImagePtr)
  {
    error = camera_set_media_packet_preview_cb(mCameraPlayer, MediaPacketCameraPreviewCb, this);
    CameraPlayerError(error, __FUNCTION__, __LINE__);

    error = camera_set_display(mCameraPlayer, CAMERA_DISPLAY_TYPE_NONE, NULL);
    CameraPlayerError(error, __FUNCTION__, __LINE__);

    mTimer = Dali::Timer::New(TIMER_INTERVAL);
    mTimer.TickSignal().Connect(this, &TizenCameraPlayer::Update);
    mTimer.Start();

    if(isPlay)
    {
      error = camera_start_preview(mCameraPlayer);
      CameraPlayerError(error, __FUNCTION__, __LINE__);
    }
  }
}

bool TizenCameraPlayer::Update()
{
  int error;

  if(mPacket != NULL)
  {
    error = media_packet_destroy(mPacket);
    if(error != MEDIA_PACKET_ERROR_NONE)
    {
      DALI_LOG_ERROR("Media packet destroy error: %d\n", error);
    }
    mPacket = NULL;
  }

  {
    Dali::Mutex::ScopedLock lock(mPacketMutex);

    if(!mPacketVector.empty())
    {
      mPacket = mPacketVector.front();
      mPacketVector.pop_front();
    }
  }

  if(mPacket == NULL)
  {
    return true;
  }

  error = media_packet_get_tbm_surface(mPacket, &mTbmSurface);
  if(error != MEDIA_PACKET_ERROR_NONE)
  {
    media_packet_destroy(mPacket);
    mPacket = NULL;
    DALI_LOG_ERROR("error: %d\n", error);
    return true;
  }

  Any source(mTbmSurface);
  mNativeImagePtr->SetSource(source);
  Dali::Adaptor::Get().RequestProcessEventsAndUpdate();

  return true;
}

void TizenCameraPlayer::DestroyPackets()
{
  int error;
  if(mPacket != NULL)
  {
    error = media_packet_destroy(mPacket);
    DALI_LOG_ERROR("Media packet destroy error: %d\n", error);
    mPacket = NULL;
  }

  {
    Dali::Mutex::ScopedLock lock(mPacketMutex);
    while(!mPacketVector.empty())
    {
      mPacket = mPacketVector.front();
      error   = media_packet_destroy(mPacket);
      DALI_LOG_ERROR("Media packet destroy error: %d\n", error);
      mPacketVector.pop_front();
      mPacket = NULL;
    }
    mPacketVector.clear();
  }
}

void TizenCameraPlayer::PushPacket(media_packet_h packet)
{
  Dali::Mutex::ScopedLock lock(mPacketMutex);

  mPacketVector.push_back(packet);
}

} // namespace Plugin
} // namespace Dali
