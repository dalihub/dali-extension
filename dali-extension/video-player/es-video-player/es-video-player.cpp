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
#include <es-video-player.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/integration-api/debug.h>
#include <tbm_surface.h>

#ifdef OVER_TIZEN_VERSION_9
#include <wayland-egl-tizen.h>
#endif

namespace Dali
{
namespace Plugin
{

namespace
{
/**
 * @brief Converts esplusplayer error code to string.
 *
 * @param[in] error The esplusplayer error code
 * @return String description of the error
 */
const char* GetEsPlayerErrorString(int error)
{
  switch(error)
  {
    case ESPLUSPLAYER_ERROR_TYPE_NONE:
      return "No error";
    case ESPLUSPLAYER_ERROR_TYPE_OUT_OF_MEMORY:
      return "Out of memory";
    case ESPLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER:
      return "Invalid parameter";
    case ESPLUSPLAYER_ERROR_TYPE_INVALID_OPERATION:
      return "Invalid operation";
    case ESPLUSPLAYER_ERROR_TYPE_INVALID_STATE:
      return "Invalid state";
    case ESPLUSPLAYER_ERROR_TYPE_NOT_SUPPORTED_AUDIO_CODEC:
      return "Not supported audio codec (video can be played)";
    case ESPLUSPLAYER_ERROR_TYPE_NOT_SUPPORTED_VIDEO_CODEC:
      return "Not supported video codec (audio can be played)";
    case ESPLUSPLAYER_ERROR_TYPE_NOT_SUPPORTED_FILE:
      return "File format not supported";
    case ESPLUSPLAYER_ERROR_TYPE_CONNECTION_FAILED:
      return "Streaming connection failed";
    case ESPLUSPLAYER_ERROR_TYPE_DRM_EXPIRED:
      return "DRM license expired";
    case ESPLUSPLAYER_ERROR_TYPE_DRM_NO_LICENSE:
      return "No DRM license";
    case ESPLUSPLAYER_ERROR_TYPE_DRM_FUTURE_USE:
      return "DRM license for future use";
    case ESPLUSPLAYER_ERROR_TYPE_NOT_PERMITTED:
      return "Format not permitted";
    case ESPLUSPLAYER_ERROR_TYPE_DRM_DECRYPTION_FAILED:
      return "DRM decryption failed";
    case ESPLUSPLAYER_ERROR_TYPE_NOT_SUPPORTED_FORMAT:
      return "Format not supported";
    case ESPLUSPLAYER_ERROR_TYPE_UNKNOWN:
      return "Unknown error";
    default:
      return "Undefined error";
  }
}

static void MediaPacketVideoDecodedCb(esplusplayer_decoded_video_packet* packet, void* userData)
{
  EsVideoPlayer* player = static_cast<EsVideoPlayer*>(userData);

  if(player == NULL || packet == NULL)
  {
    DALI_LOG_ERROR("Decoded callback got Null pointer as user_data: [%p], packet: [%p]\n", userData, packet);
    return;
  }
  player->PushPacket(packet);
  player->TriggerUiUpdate();
}

static esplusplayer_handle ExtractEsPlayerHandle(const Dali::VideoPlayerPlugin::VideoSourceDescriptor& source)
{
  if(source.nativeSession.IsType<esplusplayer_handle>())
  {
    return AnyCast<esplusplayer_handle>(source.nativeSession);
  }
  if(source.nativeSession.IsType<void*>())
  {
    return static_cast<esplusplayer_handle>(AnyCast<void*>(source.nativeSession));
  }
  return nullptr;
}

const char* const VIDEO_PLAYER_SIZE_NAME("videoPlayerSize");

#ifdef OVER_TIZEN_VERSION_9
#ifdef USE_TCORE_BACKEND
struct BufferCommitData
{
  tizen_core_wl_video_shell_surface_wrapper_h tcoreVideoShellSurfaceWrapper;
  int32_t                                     x;
  int32_t                                     y;
  int32_t                                     width;
  int32_t                                     height;
};

static void eglWindowBufferPreCommit(void* data)
{
  struct BufferCommitData* bufferCommitData = (struct BufferCommitData*)data;
  if(!bufferCommitData)
  {
    DALI_LOG_ERROR("user data is nullptrs\n");
    return;
  }
  tizen_core_wl_video_shell_surface_wrapper_update_geometry(bufferCommitData->tcoreVideoShellSurfaceWrapper, bufferCommitData->x, bufferCommitData->y, bufferCommitData->width, bufferCommitData->height);
  tizen_core_wl_video_shell_surface_wrapper_commit(bufferCommitData->tcoreVideoShellSurfaceWrapper);
}

// Keeps the underlay video shell surface's geometry synced to UI frame commits, independent
// of whether the decoder is producing new frames (see VideoView SetSyncMode()). Ported from
// TizenVideoPlayer (MMPlayer); unlike MMPlayer, esplusplayer_capi has no display type that
// actually binds the player's display to this shell surface's handle, so this constraint
// currently keeps the surface's own geometry updated but does not yet affect what's on screen.
struct VideoShellSyncConstraint
{
public:
  VideoShellSyncConstraint(tizen_core_wl_video_shell_surface_wrapper_h tcoreVideoShellSurfaceWrapper, Dali::Window window, int screenWidth, int screenHeight)
  {
    mWindow                        = window;
    mTcoreVideoShellSurfaceWrapper = tcoreVideoShellSurfaceWrapper;

    mHalfScreenWidth  = static_cast<float>(screenWidth) / 2;
    mHalfScreenHeight = static_cast<float>(screenHeight) / 2;
    DALI_LOG_RELEASE_INFO("create videoShell constraint: mTcoreVideoShellSurfaceWrapper %p\n", mTcoreVideoShellSurfaceWrapper);
  }

  void operator()(Vector3& current, const PropertyInputContainer& inputs)
  {
    // Fetched fresh every time instead of captured once at Constraint-creation time: the
    // window's EGL surface is created lazily on first render, so it is still null when this
    // constraint is set up during SceneConnection(). By the time operator() runs, rendering
    // is already underway and the EGL window is guaranteed to exist.
    wl_egl_window* eglWindowBuffer = Dali::AnyCast<wl_egl_window*>(DevelWindow::GetNativeBuffer(mWindow));
    if(!eglWindowBuffer)
    {
      DALI_LOG_ERROR("VideoShellSyncConstraint, EGL window buffer is not ready yet\n");
      return;
    }

    const Vector3& size          = inputs[0]->GetVector3();
    const Vector3& worldScale    = inputs[1]->GetVector3();
    const Vector3& worldPosition = inputs[2]->GetVector3();

    Vector3 actorSize = size * worldScale;
    Vector2 screenPosition(mHalfScreenWidth + worldPosition.x, mHalfScreenHeight + worldPosition.y);

    DisplayArea area;
    area.x      = screenPosition.x - actorSize.x / 2;
    area.y      = screenPosition.y - actorSize.y / 2;
    area.width  = actorSize.x;
    area.height = actorSize.y;

    struct BufferCommitData* bufferCommitData = (struct BufferCommitData*)calloc(1, sizeof(struct BufferCommitData));
    if(!bufferCommitData)
    {
      DALI_LOG_ERROR("Fail to calloc for BufferCommitData!!");
      return;
    }

    bufferCommitData->tcoreVideoShellSurfaceWrapper = mTcoreVideoShellSurfaceWrapper;
    bufferCommitData->x                             = area.x;
    bufferCommitData->y                             = area.x;
    bufferCommitData->width                         = area.width;
    bufferCommitData->height                        = area.height;

    // callback option 0 : Once, 1 : continuos
    wl_egl_window_tizen_set_pre_commit_callback(eglWindowBuffer, eglWindowBufferPreCommit, bufferCommitData, ONCE);
  }

private:
  Dali::Window                                mWindow;
  tizen_core_wl_video_shell_surface_wrapper_h mTcoreVideoShellSurfaceWrapper;
  float                                       mHalfScreenWidth;
  float                                       mHalfScreenHeight;
};
#else
struct BufferCommitData
{
  Ecore_Wl2_VideoShell_Surface_Wrapper* ecoreVideoShellSurfaceWrapper;
  int32_t                               x;
  int32_t                               y;
  int32_t                               width;
  int32_t                               height;
};

static void eglWindowBufferPreCommit(void* data)
{
  struct BufferCommitData* bufferCommitData = (struct BufferCommitData*)data;
  if(!bufferCommitData)
  {
    DALI_LOG_ERROR("user data is nullptrs\n");
    return;
  }
  Ecore_Wl2_VideoShell_Surface_wrapper_update_geometry(bufferCommitData->ecoreVideoShellSurfaceWrapper, bufferCommitData->x, bufferCommitData->y, bufferCommitData->width, bufferCommitData->height);
  Ecore_Wl2_VideoShell_Surface_wrapper_commit(bufferCommitData->ecoreVideoShellSurfaceWrapper);
}

// Keeps the underlay video shell surface's geometry synced to UI frame commits, independent
// of whether the decoder is producing new frames (see VideoView SetSyncMode()). Ported from
// TizenVideoPlayer (MMPlayer); unlike MMPlayer, esplusplayer_capi has no display type that
// actually binds the player's display to this shell surface's handle, so this constraint
// currently keeps the surface's own geometry updated but does not yet affect what's on screen.
struct VideoShellSyncConstraint
{
public:
  VideoShellSyncConstraint(Ecore_Wl2_VideoShell_Surface_Wrapper* ecoreVidoeShellSurfaceWrapper, Dali::Window window, int screenWidth, int screenHeight)
  {
    mWindow                        = window;
    mEcoreVidoeShellSurfaceWrapper = ecoreVidoeShellSurfaceWrapper;

    mHalfScreenWidth  = static_cast<float>(screenWidth) / 2;
    mHalfScreenHeight = static_cast<float>(screenHeight) / 2;
    DALI_LOG_RELEASE_INFO("create videoShell constraint: mEcoreVidoeShellSurfaceWrapper %p\n", mEcoreVidoeShellSurfaceWrapper);
  }

  void operator()(Vector3& current, const PropertyInputContainer& inputs)
  {
    // Fetched fresh every time instead of captured once at Constraint-creation time: the
    // window's EGL surface is created lazily on first render, so it is still null when this
    // constraint is set up during SceneConnection(). By the time operator() runs, rendering
    // is already underway and the EGL window is guaranteed to exist.
    wl_egl_window* eglWindowBuffer = Dali::AnyCast<wl_egl_window*>(DevelWindow::GetNativeBuffer(mWindow));
    if(!eglWindowBuffer)
    {
      DALI_LOG_ERROR("VideoShellSyncConstraint, EGL window buffer is not ready yet\n");
      return;
    }

    const Vector3& size          = inputs[0]->GetVector3();
    const Vector3& worldScale    = inputs[1]->GetVector3();
    const Vector3& worldPosition = inputs[2]->GetVector3();

    Vector3 actorSize = size * worldScale;
    Vector2 screenPosition(mHalfScreenWidth + worldPosition.x, mHalfScreenHeight + worldPosition.y);

    DisplayArea area;
    area.x      = screenPosition.x - actorSize.x / 2;
    area.y      = screenPosition.y - actorSize.y / 2;
    area.width  = actorSize.x;
    area.height = actorSize.y;

    struct BufferCommitData* bufferCommitData = (struct BufferCommitData*)calloc(1, sizeof(struct BufferCommitData));
    if(!bufferCommitData)
    {
      DALI_LOG_ERROR("Fail to calloc for BufferCommitData!!");
      return;
    }

    bufferCommitData->ecoreVideoShellSurfaceWrapper = mEcoreVidoeShellSurfaceWrapper;
    bufferCommitData->x                             = area.x;
    bufferCommitData->y                             = area.x;
    bufferCommitData->width                         = area.width;
    bufferCommitData->height                        = area.height;

    // callback option 0 : Once, 1 : continuos
    wl_egl_window_tizen_set_pre_commit_callback(eglWindowBuffer, eglWindowBufferPreCommit, bufferCommitData, ONCE);
  }

private:
  Dali::Window                          mWindow;
  Ecore_Wl2_VideoShell_Surface_Wrapper* mEcoreVidoeShellSurfaceWrapper;
  float                                 mHalfScreenWidth;
  float                                 mHalfScreenHeight;
};
#endif
#endif

} // namespace

EsVideoPlayer::EsVideoPlayer(Dali::VideoPlayerPlugin::VideoSourceDescriptor source, Dali::VideoSyncMode syncMode, Dali::Actor syncActor)
: VideoPlayerBase(syncMode, syncActor),
  mEsPlayer(ExtractEsPlayerHandle(source))
#ifdef USE_TCORE_BACKEND
  , mTcoreWlWindow(nullptr)
#endif
{
  if(mEsPlayer == nullptr)
  {
    DALI_LOG_ERROR("EsVideoPlayer source initialized with invalid handle type!\n");
  }
  else
  {
    DALI_LOG_RELEASE_INFO("EsVideoPlayer source initialized: %p\n", mEsPlayer);
  }
}

EsVideoPlayer::~EsVideoPlayer()
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer Destructor\n");

  if(mSyncMode == Dali::VideoSyncMode::ENABLED)
  {
    DestroyVideoShellConstraint();
  }

#ifdef OVER_TIZEN_VERSION_9
#ifdef USE_TCORE_BACKEND
  if(mTcoreVideoShellSurface)
  {
    tizen_core_wl_video_shell_surface_destroy(mTcoreVideoShellSurface);
    mTcoreVideoShellSurface = nullptr;
  }
#else
  if(mEcoreVideoShellSurface)
  {
    Ecore_Wl2_VideoShell_Surface_del(mEcoreVideoShellSurface);
    mEcoreVideoShellSurface = nullptr;
  }
#endif
#endif

  DestroyPlayer();
}

// Template Method Pattern: Primitive operations

bool EsVideoPlayer::IsPlayerReady()
{
  if(!mEsPlayer) return false;

  esplusplayer_state playerState = esplusplayer_get_state(mEsPlayer);
  return (playerState != ESPLUSPLAYER_STATE_NONE);
}

void EsVideoPlayer::DoInitializePlayer()
{
  // Player handle is already provided externally, nothing to initialize
  DALI_LOG_RELEASE_INFO("EsVideoPlayer::DoInitializePlayer - Player handle already provided\n");
}

void EsVideoPlayer::DoPlay()
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer DoPlay\n");
  if(!mEsPlayer)
  {
    return;
  }

  esplusplayer_state playerState = esplusplayer_get_state(mEsPlayer);
  if(playerState == ESPLUSPLAYER_STATE_READY || playerState == ESPLUSPLAYER_STATE_PAUSED)
  {
    int error = esplusplayer_start(mEsPlayer);
    LogPlayerError(error);
  }
}

void EsVideoPlayer::DoPause()
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer DoPause\n");
  if(!mEsPlayer)
  {
    return;
  }

  esplusplayer_state playerState = esplusplayer_get_state(mEsPlayer);
  if(playerState == ESPLUSPLAYER_STATE_PLAYING)
  {
    int error = esplusplayer_pause(mEsPlayer);
    LogPlayerError(error);
  }
}

void EsVideoPlayer::DoStop()
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer DoStop\n");
  if(!mEsPlayer)
  {
    return;
  }

  esplusplayer_state playerState = esplusplayer_get_state(mEsPlayer);
  if(playerState == ESPLUSPLAYER_STATE_PLAYING || playerState == ESPLUSPLAYER_STATE_PAUSED)
  {
    int error = esplusplayer_stop(mEsPlayer);
    LogPlayerError(error);
  }
}

void EsVideoPlayer::DoSetMute(bool mute)
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer DoSetMute\n");
  if(!mEsPlayer)
  {
    return;
  }

  int error = esplusplayer_set_audio_mute(mEsPlayer, mute);
  LogPlayerError(error);
}

void EsVideoPlayer::DoSetVolume(float left, float right)
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer DoSetVolume\n");
  if(!mEsPlayer)
  {
    return;
  }

  // esplusplayer uses volume in range 0-100
  int volume = static_cast<int>(left); // Use left channel for simplicity
  int error  = esplusplayer_set_volume(mEsPlayer, volume);
  LogPlayerError(error);
}

void EsVideoPlayer::DoSetLooping(bool looping)
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer DoSetLooping - Not supported\n");
}

void EsVideoPlayer::DoSetUrl(const std::string& url)
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer DoSetUrl - Not supported\n");
}

int EsVideoPlayer::DoGetPlayPosition()
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer DoGetPlayPosition\n");
  if(!mEsPlayer)
  {
    return 0;
  }

  uint64_t           currentTime = 0;
  esplusplayer_state playerState = esplusplayer_get_state(mEsPlayer);
  if(playerState != ESPLUSPLAYER_STATE_NONE && playerState != ESPLUSPLAYER_STATE_IDLE)
  {
    int error = esplusplayer_get_playing_time(mEsPlayer, &currentTime);
    LogPlayerError(error);
  }

  return static_cast<int>(currentTime);
}

void EsVideoPlayer::DoSetPlayPosition(int millisecond)
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer DoSetPlayPosition\n");
  if(!mEsPlayer)
  {
    return;
  }

  esplusplayer_state playerState = esplusplayer_get_state(mEsPlayer);
  if(playerState == ESPLUSPLAYER_STATE_READY ||
     playerState == ESPLUSPLAYER_STATE_PLAYING ||
     playerState == ESPLUSPLAYER_STATE_PAUSED)
  {
    int error = esplusplayer_seek(mEsPlayer, millisecond);
    LogPlayerError(error);
  }
}

void EsVideoPlayer::DoSetDisplayRotation(Dali::VideoPlayerPlugin::DisplayRotation rotation)
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer DoSetDisplayRotation\n");
  if(!mEsPlayer || mNativeImagePtr)
  {
    DALI_LOG_ERROR("EsVideoPlayer::DoSetDisplayRotation - Not supported in native image mode\n");
    return;
  }

  int error;
  switch(rotation)
  {
    case Dali::VideoPlayerPlugin::DisplayRotation::ROTATION_NONE:
      error = esplusplayer_set_display_rotation(mEsPlayer, ESPLUSPLAYER_DISPLAY_ROTATION_TYPE_NONE);
      break;
    case Dali::VideoPlayerPlugin::DisplayRotation::ROTATION_90:
      error = esplusplayer_set_display_rotation(mEsPlayer, ESPLUSPLAYER_DISPLAY_ROTATION_TYPE_90);
      break;
    case Dali::VideoPlayerPlugin::DisplayRotation::ROTATION_180:
      error = esplusplayer_set_display_rotation(mEsPlayer, ESPLUSPLAYER_DISPLAY_ROTATION_TYPE_180);
      break;
    case Dali::VideoPlayerPlugin::DisplayRotation::ROTATION_270:
      error = esplusplayer_set_display_rotation(mEsPlayer, ESPLUSPLAYER_DISPLAY_ROTATION_TYPE_270);
      break;
    default:
      error = esplusplayer_set_display_rotation(mEsPlayer, ESPLUSPLAYER_DISPLAY_ROTATION_TYPE_NONE);
      break;
  }
  LogPlayerError(error);
}

Dali::VideoPlayerPlugin::DisplayRotation EsVideoPlayer::DoGetDisplayRotation()
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer::DoGetDisplayRotation - Not supported\n");
  return Dali::VideoPlayerPlugin::DisplayRotation::ROTATION_NONE;
}

void EsVideoPlayer::DoSetDisplayArea(DisplayArea area)
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer DoSetDisplayArea\n");
  if(!mEsPlayer || mNativeImagePtr)
  {
    DALI_LOG_ERROR("EsVideoPlayer::DoSetDisplayArea - Not supported in native image mode\n");
    return;
  }

  int error = esplusplayer_set_display_roi(mEsPlayer, area.x, area.y, area.width, area.height);
  LogPlayerError(error);
}

void EsVideoPlayer::DoInitializeTextureStreamMode(Dali::NativeImagePtr nativeImagePtr)
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer::DoInitializeTextureStreamMode\n");
  esplusplayer_state playerState = ESPLUSPLAYER_STATE_NONE;
  if(mEsPlayer)
  {
    playerState = esplusplayer_get_state(mEsPlayer);
  }

  if(playerState == ESPLUSPLAYER_STATE_IDLE)
  {
    int error = esplusplayer_set_video_frame_buffer_type(mEsPlayer, ESPLUSPLAYER_DECODED_VIDEO_FRAME_BUFFER_TYPE_COPY);
    LogPlayerError(error);
    error = esplusplayer_set_media_packet_video_decoded_cb(mEsPlayer, (esplusplayer_media_packet_video_decoded_cb)MediaPacketVideoDecodedCb, this);
    LogPlayerError(error);
  }

  InitializeUiUpdateCallback();
}

void EsVideoPlayer::SetRenderingTarget(Any target)
{
  if(target.Empty())
  {
    DALI_LOG_ERROR("EsVideoPlayer::SetRenderingTarget: Empty target\n");
    return;
  }

  mNativeImagePtr = NULL;

  if(target.IsType<Dali::NativeImagePtr>())
  {
    // NativeImage (texture stream) path
    mNativeImagePtr = AnyCast<Dali::NativeImagePtr>(target);
    DoInitializeTextureStreamMode(mNativeImagePtr);
  }
#ifdef USE_TCORE_BACKEND
  else if(target.IsType<tizen_core_wl_window_h>())
  {
    // Underlay (window surface) path - tcore only
    if(mSyncMode == Dali::VideoSyncMode::ENABLED)
    {
      InitializeVideoShell(AnyCast<tizen_core_wl_window_h>(target));
    }
    InitializeUnderlayMode(AnyCast<tizen_core_wl_window_h>(target));
  }
#else
  else if(target.IsType<Ecore_Wl2_Window*>())
  {
    // Underlay is not supported for ESPlayer on the ecore backend
    if(mSyncMode == Dali::VideoSyncMode::ENABLED)
    {
      InitializeVideoShell(AnyCast<Ecore_Wl2_Window*>(target));
    }
    InitializeUnderlayMode(AnyCast<Ecore_Wl2_Window*>(target));
  }
#endif
  else
  {
    DALI_LOG_ERROR("EsVideoPlayer::SetRenderingTarget: unknown rendering target type\n");
  }
}

#ifdef USE_TCORE_BACKEND
void EsVideoPlayer::InitializeUnderlayMode(tizen_core_wl_window_h wlWindow)
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer::InitializeUnderlayMode (tcore)\n");
  if(!mEsPlayer || !wlWindow)
  {
    DALI_LOG_ERROR("EsVideoPlayer::InitializeUnderlayMode: null handle/window (player=%p, window=%p)\n", mEsPlayer, wlWindow);
    return;
  }
  mTcoreWlWindow = wlWindow;

  // hole-punch: make the window an opaque overlay so the video shows through the
  // transparent hole punched by the UI underlay visual.
  tizen_core_wl_window_set_alpha(mTcoreWlWindow, false);

  // esplusplayer_set_display_roi() (called from DoSetDisplayArea()) requires the display
  // mode to be DST_ROI first, per the esplusplayer_capi documentation.
  int error = esplusplayer_set_display_mode(mEsPlayer, ESPLUSPLAYER_DISPLAY_MODE_DST_ROI);
  if(error != ESPLUSPLAYER_ERROR_TYPE_NONE)
  {
    DALI_LOG_ERROR("EsVideoPlayer::InitializeUnderlayMode: esplusplayer_set_display_mode() failed: %d\n", error);
  }

  // esplusplayer_set_display() with ESPLUSPLAYER_DISPLAY_TYPE_OVERLAY hangs inside the
  // esplusplayer daemon on the tcore backend (ecore_pipe_write() assertion). Use
  // esplusplayer_set_display_window() instead, which binds the window and initial ROI
  // in one call without going through that path. DoSetDisplayArea() refines the ROI
  // afterwards via esplusplayer_set_display_roi().
  //
  // Bind with the window's actual geometry (matches the validated esplusplayer tcore
  // test tool's usage), not a placeholder rect.
  int x = 0, y = 0, w = 1, h = 1;
  tizen_core_wl_window_get_geometry(mTcoreWlWindow, &x, &y, &w, &h);

  error = esplusplayer_set_display_window(mEsPlayer,
                                          static_cast<void*>(mTcoreWlWindow),
                                          x, y, w, h);
  if(error != ESPLUSPLAYER_ERROR_TYPE_NONE)
  {
    DALI_LOG_ERROR("EsVideoPlayer::InitializeUnderlayMode: esplusplayer_set_display_window() failed: %d\n", error);
  }
}
#else
void EsVideoPlayer::InitializeUnderlayMode(Ecore_Wl2_Window* wlWindow)
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer::InitializeUnderlayMode (ecore)\n");
  if(!mEsPlayer || !wlWindow)
  {
    DALI_LOG_ERROR("EsVideoPlayer::InitializeUnderlayMode: null handle/window (player=%p, window=%p)\n", mEsPlayer, wlWindow);
    return;
  }
  mEcoreWlWindow = wlWindow;

  int                 width = 0, height = 0;
  Ecore_Wl2_Display*  wl2Display = ecore_wl2_connected_display_get(NULL);
  ecore_wl2_display_screen_size_get(wl2Display, &width, &height);

  // hole-punch: make the window an opaque overlay so the video shows through the
  // transparent hole punched by the UI underlay visual.
  ecore_wl2_window_alpha_set(mEcoreWlWindow, false);

  // esplusplayer_set_display_roi() (called from DoSetDisplayArea()) requires the display
  // mode to be DST_ROI first, per the esplusplayer_capi documentation.
  int error = esplusplayer_set_display_mode(mEsPlayer, ESPLUSPLAYER_DISPLAY_MODE_DST_ROI);
  if(error != ESPLUSPLAYER_ERROR_TYPE_NONE)
  {
    DALI_LOG_ERROR("EsVideoPlayer::InitializeUnderlayMode: esplusplayer_set_display_mode() failed: %d\n", error);
  }

  error = esplusplayer_set_ecore_display(mEsPlayer,
                                         ESPLUSPLAYER_DISPLAY_TYPE_OVERLAY,
                                         static_cast<void*>(mEcoreWlWindow),
                                         0, 0, width, height);
  if(error != ESPLUSPLAYER_ERROR_TYPE_NONE)
  {
    DALI_LOG_ERROR("EsVideoPlayer::InitializeUnderlayMode: esplusplayer_set_ecore_display() failed: %d\n", error);
  }

  error = esplusplayer_set_display_visible(mEsPlayer, true);
  if(error != ESPLUSPLAYER_ERROR_TYPE_NONE)
  {
    DALI_LOG_ERROR("EsVideoPlayer::InitializeUnderlayMode: esplusplayer_set_display_visible() failed: %d\n", error);
  }
}
#endif

#ifdef USE_TCORE_BACKEND
void EsVideoPlayer::InitializeVideoShell(tizen_core_wl_window_h tcoreWlWindow)
{
#ifdef OVER_TIZEN_VERSION_9
  if(mTcoreWlWindow != tcoreWlWindow)
  {
    mTcoreWlWindow = tcoreWlWindow;
    if(mTcoreVideoShellSurface)
    {
      tizen_core_wl_video_shell_surface_destroy(mTcoreVideoShellSurface);
    }

    tizen_core_wl_video_shell_surface_create(mTcoreWlWindow, &mTcoreVideoShellSurface);
    if(!mTcoreVideoShellSurface)
    {
      DALI_LOG_ERROR("InitializeVideoShell, tizen_core_wl_video_shell_surface_create() is failed\n");
      return;
    }

    tizen_core_wl_video_shell_surface_place_below(mTcoreVideoShellSurface, nullptr);
    tizen_core_wl_video_shell_surface_map(mTcoreVideoShellSurface);

    const char* videoShellHandle = nullptr;
    tizen_core_wl_video_shell_surface_get_handle(mTcoreVideoShellSurface, &videoShellHandle);
    DALI_LOG_RELEASE_INFO("VideoShell(%p) handle: %s\n", mTcoreVideoShellSurface, videoShellHandle);
  }
#else
  DALI_LOG_ERROR("InitializeVideoShell, tcore video shell NOT SUPPORT THIS TIZEN VERSION!\n");
#endif
}
#else
void EsVideoPlayer::InitializeVideoShell(Ecore_Wl2_Window* ecoreWlWindow)
{
#ifdef OVER_TIZEN_VERSION_9
  if(mEcoreWlWindow != ecoreWlWindow)
  {
    mEcoreWlWindow = ecoreWlWindow;
    if(mEcoreVideoShellSurface)
    {
      Ecore_Wl2_VideoShell_Surface_del(mEcoreVideoShellSurface);
    }

    mEcoreVideoShellSurface = Ecore_Wl2_VideoShell_Surface_new(mEcoreWlWindow);
    if(!mEcoreVideoShellSurface)
    {
      DALI_LOG_ERROR("InitializeVideoShell, Ecore_Wl2_VideoShell_Surface_new() is failed\n");
      return;
    }

    Ecore_Wl2_VideoShell_Surface_place_below(mEcoreVideoShellSurface, nullptr);
    Ecore_Wl2_VideoShell_Surface_map(mEcoreVideoShellSurface);

    const char* videoShellHandle = Ecore_Wl2_VideoShell_Surface_handle_get(mEcoreVideoShellSurface);
    DALI_LOG_RELEASE_INFO("VideoShell(%p) handle: %s\n", mEcoreVideoShellSurface, videoShellHandle);
  }
#else
  DALI_LOG_ERROR("InitializeVideoShell, ecore video shell NOT SUPPORT THIS TIZEN VERSION!\n");
#endif
}
#endif

void EsVideoPlayer::CreateVideoShellConstraint()
{
#ifdef OVER_TIZEN_VERSION_9
  DALI_LOG_RELEASE_INFO("Create Video Shell Constraint\n");
  if(mVideoShellSizePropertyIndex == Property::INVALID_INDEX)
  {
    if(mVideoShellSizePropertyConstraint)
    {
      mVideoShellSizePropertyConstraint.Remove();
    }

    Actor syncActor = mSyncActor.GetHandle();
    if(syncActor)
    {
      mVideoShellSizePropertyIndex = syncActor.RegisterProperty(VIDEO_PLAYER_SIZE_NAME, Vector3::ZERO);

#ifdef USE_TCORE_BACKEND
      int                     width, height;
      tizen_core_wl_display_h wl2Display = nullptr;
      if(!mTcoreWlWindow || tizen_core_wl_window_get_display(mTcoreWlWindow, &wl2Display) != TIZEN_CORE_WL_ERROR_NONE || !wl2Display)
      {
        DALI_LOG_ERROR("CreateVideoShellConstraint, failed to get display from window\n");
        return;
      }
      tizen_core_wl_screen_h screen = nullptr;
      tizen_core_wl_display_get_preferred_screen(wl2Display, &screen);
      int screenX = 0;
      int screenY = 0;
      tizen_core_wl_screen_get_geometry(screen, &screenX, &screenY, &width, &height);

      Window                                      window                        = Window::Get(syncActor);
      tizen_core_wl_video_shell_surface_wrapper_h tcoreVideoShellSurfaceWrapper = nullptr;
      tizen_core_wl_video_shell_surface_wrapper_create(mTcoreVideoShellSurface, &tcoreVideoShellSurfaceWrapper);

      mVideoShellSizePropertyConstraint = Constraint::New<Vector3>(syncActor,
                                                                   mVideoShellSizePropertyIndex,
                                                                   VideoShellSyncConstraint(tcoreVideoShellSurfaceWrapper, window, width, height));
#else
      int                width, height;
      Ecore_Wl2_Display* wl2Display = ecore_wl2_connected_display_get(NULL);
      ecore_wl2_display_screen_size_get(wl2Display, &width, &height);

      Window                                window                        = Window::Get(syncActor);
      Ecore_Wl2_VideoShell_Surface_Wrapper* ecoreVideoShellSurfaceWrapper = Ecore_Wl2_VideoShell_Surface_wrapper_new(mEcoreVideoShellSurface);

      mVideoShellSizePropertyConstraint = Constraint::New<Vector3>(syncActor,
                                                                   mVideoShellSizePropertyIndex,
                                                                   VideoShellSyncConstraint(ecoreVideoShellSurfaceWrapper, window, width, height));
#endif

      mVideoShellSizePropertyConstraint.AddSource(LocalSource(Actor::Property::SIZE));
      mVideoShellSizePropertyConstraint.AddSource(LocalSource(Actor::Property::WORLD_SCALE));
      mVideoShellSizePropertyConstraint.AddSource(LocalSource(Actor::Property::WORLD_POSITION));
    }
  }
#else
  DALI_LOG_ERROR("Video Shell Constraint NOT SUPPORT THIS TIZEN VERSION!\n");
#endif
}

void EsVideoPlayer::DestroyVideoShellConstraint()
{
#ifdef OVER_TIZEN_VERSION_9
  DALI_LOG_RELEASE_INFO("Destroy VideoShell Constraint: %d\n", mVideoShellSizePropertyIndex);
  if(mVideoShellSizePropertyIndex != Property::INVALID_INDEX)
  {
    mVideoShellSizePropertyConstraint.Remove();
    mVideoShellSizePropertyIndex = Property::INVALID_INDEX;
  }
#else
  DALI_LOG_ERROR("Video Shell Constraint NOT SUPPORT THIS TIZEN VERSION!\n");
#endif
}

void EsVideoPlayer::SceneConnection()
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer Scene is connected\n");

  if(mSyncMode == Dali::VideoSyncMode::ENABLED && mVideoShellSizePropertyIndex == Property::INVALID_INDEX)
  {
    CreateVideoShellConstraint();
    if(mVideoShellSizePropertyConstraint)
    {
      mVideoShellSizePropertyConstraint.ApplyPost();
    }
    else
    {
      DALI_LOG_ERROR("mVideoShellSizePropertyConstraint() creation failed!\n");
    }
  }
}

void EsVideoPlayer::SceneDisconnection()
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer Scene is disconnected\n");

  if(mSyncMode == Dali::VideoSyncMode::ENABLED && mVideoShellSizePropertyIndex != Property::INVALID_INDEX)
  {
    DestroyVideoShellConstraint();
  }
}

Any EsVideoPlayer::DoGetMediaPlayer()
{
  return Any((void*)mEsPlayer);
}

// Hook methods for post-processing
void EsVideoPlayer::OnAfterPlay()
{
  // EsVideoPlayer specific post-play operations if any
}

void EsVideoPlayer::OnAfterPause()
{
  // EsVideoPlayer specific post-pause operations if any
}

void EsVideoPlayer::OnAfterStop()
{
  // EsVideoPlayer specific post-stop operations if any
}

void EsVideoPlayer::DestroyMediaPacket(void* packet)
{
  if(packet != NULL && mEsPlayer)
  {
    int error = esplusplayer_decoded_buffer_destroy(mEsPlayer, static_cast<esplusplayer_decoded_video_packet*>(packet));
    LogPlayerError(error);
  }
}

Any EsVideoPlayer::GetSurfaceFromPacket(void* packet)
{
  if(!packet) return Any();
  esplusplayer_decoded_video_packet* decodedPacket = static_cast<esplusplayer_decoded_video_packet*>(packet);
  return Any((tbm_surface_h)decodedPacket->surface_data);
}

void EsVideoPlayer::LogPlayerError(int error)
{
  if(error != ESPLUSPLAYER_ERROR_TYPE_NONE)
  {
    DALI_LOG_ERROR("Player error: %d (%s)\n", error, GetEsPlayerErrorString(error));
  }
}

void EsVideoPlayer::DestroyPlayer()
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer DestroyPlayer\n");
  if(mEsPlayer)
  {
    esplusplayer_state playerState = esplusplayer_get_state(mEsPlayer);
    if(playerState == ESPLUSPLAYER_STATE_PLAYING || playerState == ESPLUSPLAYER_STATE_PAUSED)
    {
      if(mNativeImagePtr)
      {
        int error = esplusplayer_set_media_packet_video_decoded_cb(mEsPlayer, nullptr, nullptr);
        LogPlayerError(error);
      }

      DoStop();
    }
  }

  ClearPackets();
}

} // namespace Plugin
} // namespace Dali
