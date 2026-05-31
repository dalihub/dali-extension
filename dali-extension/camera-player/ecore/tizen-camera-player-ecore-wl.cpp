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

#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Plugin
{

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
    int                width, height;
    Ecore_Wl2_Display* wl2_display = ecore_wl2_connected_display_get(NULL);
    ecore_wl2_display_screen_size_get(wl2_display, &width, &height);

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
