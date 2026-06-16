#ifndef DALI_TIZEN_CAMERA_PLAYER_PLUGIN_H
#define DALI_TIZEN_CAMERA_PLAYER_PLUGIN_H

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

// EXTERNAL INCLUDES
#include <camera.h>
#include <dali/devel-api/adaptor-framework/camera-player-plugin.h>
#include <dali/devel-api/threading/mutex.h>
#include <dali/public-api/adaptor-framework/native-image.h>
#include <dali/public-api/adaptor-framework/timer.h>
#include <deque>
#include <string>

#ifndef HAVE_WAYLAND
#define HAVE_WAYLAND
#endif
#ifdef USE_TCORE_BACKEND
#include <tizen_core_wl.h>
#else
#include <Ecore_Wl2.h>
#endif
#include <camera_internal.h>

namespace Dali
{
namespace Plugin
{

void CameraPlayerError(int error, const char* function, int line);

/**
 * @brief Implementation of the Tizen camera player class which has Tizen
 * platform dependency.
 */
class TizenCameraPlayer : public Dali::CameraPlayerPlugin, public Dali::ConnectionTracker
{
public:
  /**
   * @brief Constructor.
   */
  TizenCameraPlayer();

  /**
   * @brief Destructor.
   */
  virtual ~TizenCameraPlayer();

  /**
   * @copydoc Dali::CameraPlayerPlugin::SetWindowRenderingTarget()
   */
  void SetWindowRenderingTarget(Dali::Window target) override;

  /**
   * @copydoc Dali::CameraPlayerPlugin::SetNativeImageRenderingTarget()
   */
  void SetNativeImageRenderingTarget(Dali::NativeImagePtr target) override;

  /**
   * @copydoc Dali::CameraPlayerPlugin::SetDisplayArea()
   */
  void SetDisplayArea(DisplayArea area) override;

  /**
   * @copydoc Dali::CameraPlayerPlugin::SetCameraPlayer()
   */
  void SetCameraPlayer(Any handle) override;

  /**
   * @brief Push media packet with camera frame image
   */
  void PushPacket(media_packet_h packet);

private:
  /**
   * @brief Updates camera frame image by timer if rendering targe is native
   * image source
   */
  bool Update();

  /**
   * @brief Gets current player state
   */
  void GetPlayerState(camera_state_e* state) const;

  /**
   * @brief Destroy all packests, which this plugin stores
   */
  void DestroyPackets();

  /**
   * @brief Initializes player for camera rendering using native image
   */
  void
  InitializeTextureStreamMode(Dali::NativeImagePtr nativeImagePtr);

  /**
   * @brief Initializes player for camera rendering using wayland window surface
   */
#ifdef USE_TCORE_BACKEND
  void InitializeUnderlayMode(tizen_core_wl_window_h tcoreWlWindow);
#else
  void InitializeUnderlayMode(Ecore_Wl2_Window* ecoreWlWindow);
#endif

  /**
   * @brief Stop camera preview
   */
  void StopPreview();

  /**
   * @brief Destroy camera player
   */
  void Destroy();

private:
  camera_h       mCameraPlayer;      ///< Camera handle
  camera_state_e mCameraPlayerState; ///< State of Camera Player

  tbm_surface_h  mTbmSurface; ///< tbm surface handle
  media_packet_h mPacket;     ///< Media packet handle with tbm surface of current camera frame image

  Dali::NativeImagePtr mNativeImagePtr;  ///< native image for camera rendering
  Dali::Timer          mTimer;           ///< Timer for texture streaming rendering
  Dali::Vector4        mBackgroundColor; ///< Current background color, which
                                         ///< texturestream mode needs.

  Dali::Mutex mPacketMutex;

  std::deque<media_packet_h> mPacketVector; ///< Container for media packet handle from Tizen player callback

#ifdef USE_TCORE_BACKEND
  tizen_core_wl_window_h mTcoreWlWindow; ///< tizen-core native window handle
#else
  Ecore_Wl2_Window* mEcoreWlWindow; ///< ecore native window handle
#endif
};

} // namespace Plugin
} // namespace Dali

#endif
