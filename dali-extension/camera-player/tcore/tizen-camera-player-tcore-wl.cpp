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

#include <tizen-camera-player.h>
#include <tizen-core-wl/tizen_core_wl_internal.h>
#include <tizen_core_wl.h>

#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Plugin
{

void TizenCameraPlayer::InitializeUnderlayMode(tizen_core_wl_window_h tcoreWlWindow)
{
  int error;

  mTcoreWlWindow = tcoreWlWindow;

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
    tizen_core_wl_window_set_alpha(mTcoreWlWindow, false);

    error = camera_set_display_mode(mCameraPlayer, CAMERA_DISPLAY_MODE_CUSTOM_ROI);
    CameraPlayerError(error, __FUNCTION__, __LINE__);

    error = camera_attr_set_display_roi_area(mCameraPlayer, 0, 0, 1, 1);
    CameraPlayerError(error, __FUNCTION__, __LINE__);

    if(mTcoreWlWindow)
    {
      error = camera_set_tcore_display(mCameraPlayer, static_cast<camera_display_h>(mTcoreWlWindow));
      CameraPlayerError(error, __FUNCTION__, __LINE__);
    }
    else
    {
      DALI_LOG_ERROR("InitializeUnderlayMode, tizen-core window is null\n");
    }

    if(isPlay)
    {
      error = camera_start_preview(mCameraPlayer);
      CameraPlayerError(error, __FUNCTION__, __LINE__);
    }
  }
}

void TizenCameraPlayer::SetDisplayArea(DisplayArea area)
{
  GetPlayerState(&mCameraPlayerState);

  if(mNativeImagePtr)
  {
    DALI_LOG_ERROR("SetDisplayArea is only for window surface target.\n");
    return;
  }

  if(mCameraPlayerState == CAMERA_STATE_CREATED ||
     mCameraPlayerState == CAMERA_STATE_PREVIEW)
  {
    int width = 0;
    int height = 0;

    tizen_core_wl_display_h display = nullptr;
    if(!mTcoreWlWindow || tizen_core_wl_window_get_display(mTcoreWlWindow, &display) != TIZEN_CORE_WL_ERROR_NONE || !display)
    {
      DALI_LOG_ERROR("SetDisplayArea, failed to get display from window\n");
      return;
    }
    tizen_core_wl_screen_h screen = nullptr;
    tizen_core_wl_display_get_preferred_screen(display, &screen);
    int screenX = 0;
    int screenY = 0;
    tizen_core_wl_screen_get_geometry(screen, &screenX, &screenY, &width, &height);

    camera_rotation_e rotation = CAMERA_ROTATION_NONE;

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
