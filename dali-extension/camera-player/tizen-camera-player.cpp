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
#include <tizen-camera-player.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/stage.h>
#include <dali/devel-api/threading/mutex.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES

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

} // unnamed namespace

TizenCameraPlayer::TizenCameraPlayer()
: mCameraPlayer(NULL),
  mCameraPlayerState(CAMERA_STATE_NONE),
  mTbmSurface(NULL),
  mPacket(NULL),
  mNativeImageSourcePtr(NULL),
  mTimer(),
  mBackgroundColor(Dali::Vector4(1.0f, 1.0f, 1.0f, 0.0f)),
  mPacketMutex(),
  mPacketVector(),
  mEcoreWlWindow(nullptr)
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
  mNativeImageSourcePtr = NULL;

  InitializeUnderlayMode(Dali::AnyCast<Ecore_Wl2_Window*>(target.GetNativeHandle()));
}

void TizenCameraPlayer::SetNativeImageRenderingTarget(Dali::NativeImageSourcePtr target)
{
  mNativeImageSourcePtr = NULL;

  InitializeTextureStreamMode(target);
}

void TizenCameraPlayer::StopPreview()
{
  GetPlayerState(&mCameraPlayerState);

  if(mCameraPlayerState == CAMERA_STATE_PREVIEW)
  {
    int error = camera_stop_preview(mCameraPlayer);
    CameraPlayerError(error, __FUNCTION__, __LINE__);

    if(mNativeImageSourcePtr && mTimer)
    {
      mTimer.Stop();
      DestroyPackets();
    }
  }
}

void TizenCameraPlayer::Destroy()
{
  if(mNativeImageSourcePtr && mTimer)
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

void TizenCameraPlayer::InitializeTextureStreamMode(Dali::NativeImageSourcePtr nativeImageSourcePtr)
{
  int error;

  mNativeImageSourcePtr = nativeImageSourcePtr;

  GetPlayerState(&mCameraPlayerState);

  bool isPlay = false;
  if(mCameraPlayerState == CAMERA_STATE_PREVIEW)
  {
    isPlay = true;
    StopPreview();
  }

  GetPlayerState(&mCameraPlayerState);

  if(mCameraPlayerState == CAMERA_STATE_CREATED && mNativeImageSourcePtr)
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

void TizenCameraPlayer::InitializeUnderlayMode(Ecore_Wl2_Window* ecoreWlWindow)
{
  int error;

  mEcoreWlWindow = ecoreWlWindow;

  GetPlayerState(&mCameraPlayerState);

  bool isPlay = false;
  if(mCameraPlayerState == CAMERA_STATE_PREVIEW)
  {
    isPlay = true;
    StopPreview();
  }

  GetPlayerState(&mCameraPlayerState);

  if(mCameraPlayerState == CAMERA_STATE_CREATED)
  {
    ecore_wl2_window_alpha_set(mEcoreWlWindow, false);

    error = camera_set_display_mode(mCameraPlayer, CAMERA_DISPLAY_MODE_CUSTOM_ROI);
    CameraPlayerError(error, __FUNCTION__, __LINE__);

    error = camera_attr_set_display_roi_area(mCameraPlayer, 0, 0, 1, 1);
    CameraPlayerError(error, __FUNCTION__, __LINE__);

    error = camera_set_ecore_wl_display(mCameraPlayer, GET_DISPLAY(mEcoreWlWindow));
    CameraPlayerError(error, __FUNCTION__, __LINE__);

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
  mNativeImageSourcePtr->SetSource(source);
  Dali::Stage::GetCurrent().KeepRendering(0.0f);

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

void TizenCameraPlayer::SetDisplayArea(DisplayArea area)
{
  GetPlayerState(&mCameraPlayerState);

  if(mNativeImageSourcePtr)
  {
    DALI_LOG_ERROR("SetDisplayArea is only for window surface target.\n");
    return;
  }

  if(mCameraPlayerState == CAMERA_STATE_CREATED ||
     mCameraPlayerState == CAMERA_STATE_PREVIEW)
  {
    int                width, height;
    Ecore_Wl2_Display* wl2_display = ecore_wl2_connected_display_get(NULL);
    ecore_wl2_display_screen_size_get(wl2_display, &width, &height);

    // camera x, y postion
    camera_rotation_e rotation = CAMERA_ROTATION_NONE;

    /* Get the default display rotation value */
    int error = camera_get_display_rotation(mCameraPlayer, &rotation);
    CameraPlayerError(error, __FUNCTION__, __LINE__);

    switch(rotation)
    {
      case CAMERA_ROTATION_270:
      {
        int temp = area.y;
        area.y   = width - area.x - area.width;
        area.x   = temp;

        int tempWidth = area.width;
        area.width    = area.height;
        area.height   = tempWidth;
        break;
      }
      case CAMERA_ROTATION_NONE:
      {
        // same position
        break;
      }
      case CAMERA_ROTATION_90:
      {
        int temp = area.x;
        area.x   = height - area.y - area.height;
        area.y   = temp;

        int tempWidth = area.width;
        area.width    = area.height;
        area.height   = tempWidth;
        break;
      }
      case CAMERA_ROTATION_180:
      {
        area.x = width - area.x - area.width;
        area.y = height - area.y - area.height;
        break;
      }
      default:
      {
        DALI_LOG_ERROR("Unkown camera rotation : %d \n", rotation);
        break;
      }
    }

    area.x = (area.x < 0) ? 0 : area.x;
    area.y = (area.y < 0) ? 0 : area.y;

    error = camera_attr_set_display_roi_area(mCameraPlayer, area.x, area.y, area.width, area.height);
    CameraPlayerError(error, __FUNCTION__, __LINE__);
  }
}

} // namespace Plugin
} // namespace Dali
