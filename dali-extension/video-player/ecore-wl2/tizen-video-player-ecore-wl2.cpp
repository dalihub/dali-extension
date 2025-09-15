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
#include <tizen-video-player.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/devel-api/common/stage-devel.h>
#include <dali/devel-api/common/stage.h>
#include <dali/devel-api/threading/mutex.h>
#include <dali/integration-api/debug.h>
#include <system_info.h>

#ifdef OVER_TIZEN_VERSION_9
#include <wayland-egl-tizen.h>
#endif

// INTERNAL INCLUDES

// The plugin factories
extern "C" DALI_EXPORT_API Dali::VideoPlayerPlugin* CreateVideoPlayerPlugin(Dali::Actor actor, Dali::VideoSyncMode syncMode)
{
  return new Dali::Plugin::TizenVideoPlayer(actor, syncMode);
}

extern "C" DALI_EXPORT_API void DestroyVideoPlayerPlugin(Dali::VideoPlayerPlugin* plugin)
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
const char* TIZEN_GLIB_CONTEXT_ENV = "TIZEN_GLIB_CONTEXT";

const int TIMER_INTERVAL(20);

static void MediaPacketVideoDecodedCb(media_packet_h packet, void* user_data)
{
  TizenVideoPlayer* player = static_cast<TizenVideoPlayer*>(user_data);

  if(player == NULL)
  {
    DALI_LOG_ERROR("Decoded callback got Null pointer as user_data.\n");
    return;
  }

  player->PushPacket(packet);
}

static GMainContext* GetTizenGlibContext()
{
  static GMainContext* context = nullptr;

  if(!context)
  {
    const char* env = Dali::EnvironmentVariable::GetEnvironmentVariable(TIZEN_GLIB_CONTEXT_ENV);
    if(env)
    {
      context = (GMainContext*)strtoul(env, nullptr, 10);
    }
  }

  return context;
}

static void EmitPlaybackFinishedSignal(void* user_data)
{
  TizenVideoPlayer* player = static_cast<TizenVideoPlayer*>(user_data);

  if(player == NULL)
  {
    DALI_LOG_ERROR("Decoded callback got Null pointer as user_data.\n");
    return;
  }

  if(!player->mFinishedSignal.Empty())
  {
    DALI_LOG_ERROR("EmitPlaybackFinishedSignal.\n");

    // This function is invoked on the main thread from MMFW.
    // So the FinishedSignal has to be posted to UIThread when UIThread is enabled.
    // If not, it causes an assertion in the timer.
    // The timer has to run on the same thread as the adaptor's
    GMainContext* context = GetTizenGlibContext();
    if(context)
    {
      GSource* source = g_idle_source_new();
      g_source_set_callback(
        source,
        [](gpointer userData) -> gboolean {
          auto* player = static_cast<TizenVideoPlayer*>(userData);
          player->mFinishedSignal.Emit();
          player->Stop();
          return G_SOURCE_REMOVE;
        },
        player,
        nullptr);
      g_source_attach(source, context);
      g_source_unref(source);
    }
    else
    {
      player->mFinishedSignal.Emit();
      player->Stop();
    }
  }
}

// ToDo: VD player_set_play_position() doesn't work when callback pointer is NULL.
// We should check whether this callback is needed in platform.
static void PlayerSeekCompletedCb(void* data)
{
}

int LogPlayerError(int error)
{
  int ret = 0;
  if(error != PLAYER_ERROR_NONE)
  {
    ret = error;
    switch(error)
    {
      case PLAYER_ERROR_OUT_OF_MEMORY:
      {
        DALI_LOG_ERROR("Player error: Out of memory\n");
      }
      break;
      case PLAYER_ERROR_INVALID_PARAMETER:
      {
        DALI_LOG_ERROR("Player error: Invalid parameter\n");
      }
      break;
      case PLAYER_ERROR_NO_SUCH_FILE:
      {
        DALI_LOG_ERROR("Player error: No such file\n");
      }
      break;
      case PLAYER_ERROR_INVALID_OPERATION:
      {
        DALI_LOG_ERROR("Player error: Invalid operation\n");
      }
      break;
      case PLAYER_ERROR_FILE_NO_SPACE_ON_DEVICE:
      {
        DALI_LOG_ERROR("Player error: No space on device\n");
      }
      break;
      case PLAYER_ERROR_FEATURE_NOT_SUPPORTED_ON_DEVICE:
      {
        DALI_LOG_ERROR("Player error: Not supported feature on device\n");
      }
      break;
      case PLAYER_ERROR_SEEK_FAILED:
      {
        DALI_LOG_ERROR("Player error: Seek failed\n");
      }
      break;
      case PLAYER_ERROR_INVALID_STATE:
      {
        DALI_LOG_ERROR("Player error: Invalid state\n");
      }
      break;
      case PLAYER_ERROR_NOT_SUPPORTED_FILE:
      {
        DALI_LOG_ERROR("Player error: Not supported file\n");
      }
      break;
      case PLAYER_ERROR_INVALID_URI:
      {
        DALI_LOG_ERROR("Player error: Invalid uri\n");
      }
      break;
      case PLAYER_ERROR_SOUND_POLICY:
      {
        DALI_LOG_ERROR("Player error: Sound policy\n");
      }
      break;
      case PLAYER_ERROR_CONNECTION_FAILED:
      {
        DALI_LOG_ERROR("Player error: Connection failed\n");
      }
      break;
      case PLAYER_ERROR_VIDEO_CAPTURE_FAILED:
      {
        DALI_LOG_ERROR("Player error: Video capture failed\n");
      }
      break;
      case PLAYER_ERROR_DRM_EXPIRED:
      {
        DALI_LOG_ERROR("Player error: DRM expired\n");
      }
      break;
      case PLAYER_ERROR_DRM_NO_LICENSE:
      {
        DALI_LOG_ERROR("Player error: No license\n");
      }
      break;
      case PLAYER_ERROR_DRM_FUTURE_USE:
      {
        DALI_LOG_ERROR("Player error: License for future use\n");
      }
      break;
      case PLAYER_ERROR_DRM_NOT_PERMITTED:
      {
        DALI_LOG_ERROR("Player error: Format not permitted\n");
      }
      break;
      case PLAYER_ERROR_RESOURCE_LIMIT:
      {
        DALI_LOG_ERROR("Player error: Resource limit\n");
      }
      break;
      case PLAYER_ERROR_PERMISSION_DENIED:
      {
        DALI_LOG_ERROR("Player error: Permission denied\n");
      }
      break;
      case PLAYER_ERROR_SERVICE_DISCONNECTED:
      {
        DALI_LOG_ERROR("Player error: Service disconnected\n");
      }
      break;
      case PLAYER_ERROR_BUFFER_SPACE:
      {
        DALI_LOG_ERROR("Player error: Buffer space\n");
      }
      break;
      case PLAYER_ERROR_NOT_SUPPORTED_VIDEO_CODEC:
      {
        DALI_LOG_ERROR("Player error: The target should not support the codec type\n");
      }
      break;
      default:
      {
        DALI_LOG_ERROR("Player error: Unknown error code ( %d ) \n", error);
      }
      break;
    }
  }
  return ret;
}

const char* const VIDEO_PLAYER_SIZE_NAME("videoPlayerSize");

#ifdef OVER_TIZEN_VERSION_9
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
  DALI_LOG_RELEASE_INFO("eglWindowBufferPreCommit: wrapper: %p\n", bufferCommitData);
  Ecore_Wl2_VideoShell_Surface_wrapper_update_geometry(bufferCommitData->ecoreVideoShellSurfaceWrapper, bufferCommitData->x, bufferCommitData->y, bufferCommitData->width, bufferCommitData->height);
  Ecore_Wl2_VideoShell_Surface_wrapper_commit(bufferCommitData->ecoreVideoShellSurfaceWrapper);
}

struct VideoShellSyncConstraint
{
public:
  VideoShellSyncConstraint(Ecore_Wl2_VideoShell_Surface_Wrapper* ecoreVidoeShellSurfaceWrapper, wl_egl_window* eglWindowBuffer, int screenWidth, int screenHeight)
  {
    mEglWindowBuffer               = eglWindowBuffer;
    mEcoreVidoeShellSurfaceWrapper = ecoreVidoeShellSurfaceWrapper;

    mHalfScreenWidth  = static_cast<float>(screenWidth) / 2;
    mHalfScreenHeight = static_cast<float>(screenHeight) / 2;
    DALI_LOG_RELEASE_INFO("create videoShell constraint: mEcoreVidoeShellSurfaceWrapper %p, mEglWindowBuffer: %p\n", mEcoreVidoeShellSurfaceWrapper, mEglWindowBuffer);
  }

  void operator()(Vector3& current, const PropertyInputContainer& inputs)
  {
    const Vector3& size          = inputs[0]->GetVector3();
    const Vector3& worldScale    = inputs[1]->GetVector3();
    const Vector3& worldPosition = inputs[2]->GetVector3();

    Vector3 actorSize = size * worldScale;
    Vector2 screenPosition(mHalfScreenWidth + worldPosition.x, mHalfScreenHeight + worldPosition.y);

    //1. Update VideoView area
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

    DALI_LOG_DEBUG_INFO("Setup the eglWindow PreCommit Callback with wrapper : %p, (%d,%d)(%d x %d)\n", bufferCommitData->ecoreVideoShellSurfaceWrapper, bufferCommitData->x, bufferCommitData->y, bufferCommitData->width, bufferCommitData->height);

    // callback option 0 : Once, 1 : continuos
    wl_egl_window_tizen_set_pre_commit_callback(mEglWindowBuffer, eglWindowBufferPreCommit, bufferCommitData, ONCE);
  }

private:
  wl_egl_window*                        mEglWindowBuffer;
  Ecore_Wl2_VideoShell_Surface_Wrapper* mEcoreVidoeShellSurfaceWrapper;
  float                                 mHalfScreenWidth;
  float                                 mHalfScreenHeight;
};
#endif

/**
 * @brief Whether set play positoin accurately or not.
 *  If true, we set play position to the nearest frame position. but this might be considerably slow, accurately.
 *  If false, we set play position to the nearest key frame position. this might be faster but less accurate.
 * see player_set_play_position()
 */
constexpr bool ACCURATE_PLAY_POSITION_SET = true;

} // unnamed namespace

TizenVideoPlayer::TizenVideoPlayer(Dali::Actor actor, Dali::VideoSyncMode syncMode)
: mUrl(),
  mPlayer(NULL),
  mPlayerState(PLAYER_STATE_NONE),
  mPacket(NULL),
  mNativeImageSourcePtr(NULL),
  mTimer(),
  mBackgroundColor(Dali::Vector4(1.0f, 1.0f, 1.0f, 0.0f)),
  mTargetType(NATIVE_IMAGE),
  mPacketMutex(),
  mPacketList(),
  mStreamInfo(NULL),
  mStreamType(SOUND_STREAM_TYPE_MEDIA),
  mCodecType(PLAYER_CODEC_TYPE_HW),
  mEcoreWlWindow(nullptr),
  mEcoreSubVideoWindow(nullptr),
  mSyncActor(actor),
  mVideoSizePropertyIndex(Property::INVALID_INDEX),
  mSyncMode(syncMode),
  mIsMovedHandle(false),
  mIsSceneConnected(false),
#ifdef OVER_TIZEN_VERSION_9
  mEcoreVideoShellSurface(nullptr),
#endif
  mVideoShellSizePropertyIndex(Property::INVALID_INDEX)
{
}

TizenVideoPlayer::~TizenVideoPlayer()
{
  if(mSyncMode == Dali::VideoSyncMode::ENABLED)
  {
    DestroyVideoShellConstraint();
  }

  if(mEcoreSubVideoWindow)
  {
    ecore_wl2_subsurface_del(mEcoreSubVideoWindow);
    mEcoreSubVideoWindow = nullptr;
  }

#ifdef OVER_TIZEN_VERSION_9
  if(mEcoreVideoShellSurface)
  {
    Ecore_Wl2_VideoShell_Surface_del(mEcoreVideoShellSurface);
    mEcoreVideoShellSurface = nullptr;
  }
#endif

  DestroyPlayer();
}

void TizenVideoPlayer::GetPlayerState(player_state_e* state) const
{
  if(mPlayer != NULL && player_get_state(mPlayer, state) != PLAYER_ERROR_NONE)
  {
    DALI_LOG_ERROR("player_get_state error: Invalid parameter\n");
    *state = PLAYER_STATE_NONE;
  }
}

void TizenVideoPlayer::SetUrl(const std::string& url)
{
  int ret = 0;
  if(mUrl != url)
  {
    int error = PLAYER_ERROR_NONE;

    mUrl = url;

    GetPlayerState(&mPlayerState);

    if(mPlayerState != PLAYER_STATE_NONE && mPlayerState != PLAYER_STATE_IDLE)
    {
      if(mNativeImageSourcePtr)
      {
        error = player_unset_media_packet_video_frame_decoded_cb(mPlayer);
        ret   = LogPlayerError(error);
        if(ret)
        {
          DALI_LOG_ERROR("SetUrl, player_unset_media_packet_video_frame_decoded_cb() is failed\n");
        }
      }
      Stop();

      error = player_unprepare(mPlayer);
      ret   = LogPlayerError(error);
      if(ret)
      {
        DALI_LOG_ERROR("SetUrl, player_unprepare() is failed\n");
      }

      if(mNativeImageSourcePtr)
      {
        error = player_set_media_packet_video_frame_decoded_cb(mPlayer, MediaPacketVideoDecodedCb, this);
        ret   = LogPlayerError(error);
        if(ret)
        {
          DALI_LOG_ERROR("SetUrl, player_set_media_packet_video_frame_decoded_cb() is failed\n");
        }
      }
      else
      {
        int                width, height;
        Ecore_Wl2_Display* wl2_display = ecore_wl2_connected_display_get(NULL);
        ecore_wl2_display_screen_size_get(wl2_display, &width, &height);

        {
          error = player_set_ecore_wl_display(mPlayer, PLAYER_DISPLAY_TYPE_OVERLAY, mEcoreWlWindow, 0, 0, width, height);
          ret   = LogPlayerError(error);
          if(ret)
          {
            DALI_LOG_ERROR("SetUrl, player_set_ecore_wl_display() is failed\n");
          }
        }
      }
      GetPlayerState(&mPlayerState);
      ret = LogPlayerError(error);
      if(ret)
      {
        DALI_LOG_ERROR("SetUrl, GetPlayerState() is failed\n");
      }
    }

    if(mPlayerState == PLAYER_STATE_IDLE)
    {
      error = player_set_uri(mPlayer, mUrl.c_str());
      ret   = LogPlayerError(error);
      if(ret)
      {
        DALI_LOG_ERROR("SetUrl, player_set_uri() is failed\n");
      }

      error = player_prepare(mPlayer);
      ret   = LogPlayerError(error);
      if(ret)
      {
        DALI_LOG_ERROR("SetUrl, player_prepare() is failed\n");
      }
    }
  }
}

std::string TizenVideoPlayer::GetUrl()
{
  return mUrl;
}

void TizenVideoPlayer::SetRenderingTarget(Any target)
{
  DestroyPlayer();

  mNativeImageSourcePtr = NULL;

  if(target.GetType() == typeid(Dali::NativeImageSourcePtr))
  {
    if(mSyncMode == Dali::VideoSyncMode::ENABLED)
    {
      DestroyVideoShellConstraint();
    }
    mTargetType = TizenVideoPlayer::NATIVE_IMAGE;

    Dali::NativeImageSourcePtr nativeImageSourcePtr = AnyCast<Dali::NativeImageSourcePtr>(target);

    DALI_LOG_RELEASE_INFO("target is not underlay mode\n");
    InitializeTextureStreamMode(nativeImageSourcePtr);
  }
  else if(target.GetType() == typeid(Ecore_Wl2_Window*))
  {
    mTargetType = TizenVideoPlayer::WINDOW_SURFACE;

    if(mSyncMode == Dali::VideoSyncMode::ENABLED)
    {
      InitializeVideoShell(Dali::AnyCast<Ecore_Wl2_Window*>(target));
    }

    DALI_LOG_RELEASE_INFO("target is underlay mode\n");
    InitializeUnderlayMode(Dali::AnyCast<Ecore_Wl2_Window*>(target));
  }
  else
  {
    DALI_LOG_ERROR("SetRenderingTarget, Video rendering target is unknown\n");
  }
}

void TizenVideoPlayer::SetLooping(bool looping)
{
  GetPlayerState(&mPlayerState);

  if(mPlayerState != PLAYER_STATE_NONE)
  {
    int error = player_set_looping(mPlayer, looping);
    int ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("SetLooping, player_set_looping() is failed\n");
    }
  }
}

bool TizenVideoPlayer::IsLooping()
{
  GetPlayerState(&mPlayerState);

  bool looping = false;
  if(mPlayerState != PLAYER_STATE_NONE)
  {
    int error = player_is_looping(mPlayer, &looping);
    int ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("IsLooping, player_is_looping() is failed\n");
    }
  }

  return looping;
}

void TizenVideoPlayer::Play()
{
  GetPlayerState(&mPlayerState);

  if(mPlayerState == PLAYER_STATE_READY || mPlayerState == PLAYER_STATE_PAUSED)
  {
    if(mNativeImageSourcePtr && mTimer)
    {
      mTimer.Start();
    }

    int error = player_start(mPlayer);
    int ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("Play, player_start() is failed\n");
    }
  }
}

void TizenVideoPlayer::Pause()
{
  GetPlayerState(&mPlayerState);
  int ret = 0;
  if(mPlayerState == PLAYER_STATE_PLAYING)
  {
    int error = player_pause(mPlayer);
    ret       = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("Pause, player_pause() is failed\n");
    }

    if(mNativeImageSourcePtr && mTimer)
    {
      mTimer.Stop();
      DestroyPackets();
    }
  }
}

void TizenVideoPlayer::Stop()
{
  GetPlayerState(&mPlayerState);

  if(mPlayerState == PLAYER_STATE_PLAYING || mPlayerState == PLAYER_STATE_PAUSED)
  {
    int error = player_stop(mPlayer);
    int ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("Stop, player_stop() is failed\n");
    }

    if(mNativeImageSourcePtr && mTimer)
    {
      mTimer.Stop();
      DestroyPackets();
    }
  }
}

void TizenVideoPlayer::SetMute(bool muted)
{
  GetPlayerState(&mPlayerState);

  if(mPlayerState == PLAYER_STATE_IDLE ||
     mPlayerState == PLAYER_STATE_READY ||
     mPlayerState == PLAYER_STATE_PLAYING ||
     mPlayerState == PLAYER_STATE_PAUSED)
  {
    int error = player_set_mute(mPlayer, muted);
    int ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("SetMute, player_set_mute() is failed\n");
    }
  }
}

bool TizenVideoPlayer::IsMuted()
{
  GetPlayerState(&mPlayerState);
  bool muted = false;

  if(mPlayerState == PLAYER_STATE_IDLE ||
     mPlayerState == PLAYER_STATE_READY ||
     mPlayerState == PLAYER_STATE_PLAYING ||
     mPlayerState == PLAYER_STATE_PAUSED)
  {
    int error = player_is_muted(mPlayer, &muted);
    int ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("IsMuted, player_is_muted() is failed\n");
    }
  }

  return muted;
}

void TizenVideoPlayer::SetVolume(float left, float right)
{
  GetPlayerState(&mPlayerState);

  int error = player_set_volume(mPlayer, left, right);
  int ret   = LogPlayerError(error);
  if(ret)
  {
    DALI_LOG_ERROR("SetVolume, player_set_volume() is failed\n");
  }
}

void TizenVideoPlayer::GetVolume(float& left, float& right)
{
  GetPlayerState(&mPlayerState);

  int error = player_get_volume(mPlayer, &left, &right);
  int ret   = LogPlayerError(error);
  if(ret)
  {
    DALI_LOG_ERROR("GetVolume, player_get_volume() is failed\n");
  }
}

void TizenVideoPlayer::SetPlayPosition(int millisecond)
{
  int error;
  int ret = 0;

  GetPlayerState(&mPlayerState);

  if(mPlayerState == PLAYER_STATE_IDLE)
  {
    error = player_prepare(mPlayer);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("SetPlayPosition, player_prepare() is failed\n");
    }

    GetPlayerState(&mPlayerState); // Check the status again.
  }

  if(mPlayerState == PLAYER_STATE_READY ||
     mPlayerState == PLAYER_STATE_PLAYING ||
     mPlayerState == PLAYER_STATE_PAUSED)
  {
    error = player_set_play_position(mPlayer, millisecond, ACCURATE_PLAY_POSITION_SET, PlayerSeekCompletedCb, NULL);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("SetPlayPosition, player_set_play_position() is failed\n");
    }
  }
}

int TizenVideoPlayer::GetPlayPosition()
{
  int error;
  int millisecond = 0;

  GetPlayerState(&mPlayerState);

  if(mPlayerState == PLAYER_STATE_IDLE ||
     mPlayerState == PLAYER_STATE_READY ||
     mPlayerState == PLAYER_STATE_PLAYING ||
     mPlayerState == PLAYER_STATE_PAUSED)
  {
    error   = player_get_play_position(mPlayer, &millisecond);
    int ret = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("GetPlayPosition, player_get_play_position() is failed\n");
    }
  }

  return millisecond;
}

void TizenVideoPlayer::SetDisplayRotation(Dali::VideoPlayerPlugin::DisplayRotation rotation)
{
  if(mNativeImageSourcePtr)
  {
    DALI_LOG_ERROR("SetDisplayRotation is only for window rendering target.\n");
    return;
  }

  int error;
  if(mPlayerState != PLAYER_STATE_NONE)
  {
    error   = player_set_display_rotation(mPlayer, static_cast<player_display_rotation_e>(rotation));
    int ret = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("SetDisplayRotation, player_set_display_rotation() is failed\n");
    }
  }
}

Dali::VideoPlayerPlugin::DisplayRotation TizenVideoPlayer::GetDisplayRotation()
{
  if(mNativeImageSourcePtr)
  {
    DALI_LOG_ERROR("GetDisplayRotation is only for window rendering target.\n");
    return Dali::VideoPlayerPlugin::ROTATION_NONE;
  }

  int                       error;
  player_display_rotation_e rotation = PLAYER_DISPLAY_ROTATION_NONE;
  if(mPlayerState != PLAYER_STATE_NONE)
  {
    error   = player_get_display_rotation(mPlayer, &rotation);
    int ret = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("GetDisplayRotation, player_get_display_rotation() is failed\n");
    }
  }
  return static_cast<Dali::VideoPlayerPlugin::DisplayRotation>(rotation);
}

Dali::VideoPlayerPlugin::VideoPlayerSignalType& TizenVideoPlayer::FinishedSignal()
{
  return mFinishedSignal;
}

void TizenVideoPlayer::InitializeTextureStreamMode(Dali::NativeImageSourcePtr nativeImageSourcePtr)
{
  int error;
  int ret = 0;

  mNativeImageSourcePtr = nativeImageSourcePtr;

  if(mPlayerState == PLAYER_STATE_NONE)
  {
    error = player_create(&mPlayer);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("InitializeTextureStreamMode, player_create() is failed\n");
    }
  }

  GetPlayerState(&mPlayerState);

  if(mPlayerState == PLAYER_STATE_IDLE)
  {
    error = player_set_completed_cb(mPlayer, EmitPlaybackFinishedSignal, this);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("InitializeTextureStreamMode, player_set_completed_cb() is failed\n");
    }

    error = player_set_media_packet_video_frame_decoded_cb(mPlayer, MediaPacketVideoDecodedCb, this);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("InitializeTextureStreamMode, player_set_media_packet_video_frame_decoded_cb() is failed\n");
    }

    error = sound_manager_create_stream_information(mStreamType, NULL, NULL, &mStreamInfo);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("InitializeTextureStreamMode, sound_manager_create_stream_information() is failed\n");
    }

    error = player_set_sound_stream_info(mPlayer, mStreamInfo);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("InitializeTextureStreamMode, player_set_sound_stream_info() is failed\n");
    }

    error = player_set_display_mode(mPlayer, PLAYER_DISPLAY_MODE_FULL_SCREEN);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("InitializeTextureStreamMode, player_set_display_mode() is failed\n");
    }

    error = player_set_display(mPlayer, PLAYER_DISPLAY_TYPE_NONE, NULL);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("InitializeTextureStreamMode, player_set_display() is failed\n");
    }

    error = player_set_video_codec_type(mPlayer, mCodecType);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("InitializeTextureStreamMode, player_set_video_codec_type() is failed\n");
    }
    error = player_set_display_visible(mPlayer, true);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("InitializeTextureStreamMode, player_set_display_visible() is failed\n");
    }

    mTimer = Dali::Timer::New(TIMER_INTERVAL);
    mTimer.TickSignal().Connect(this, &TizenVideoPlayer::Update);
  }
}

void TizenVideoPlayer::InitializeVideoShell(Ecore_Wl2_Window* ecoreWlWindow)
{
#ifdef OVER_TIZEN_VERSION_9
  if(mEcoreWlWindow != ecoreWlWindow)
  {
    mEcoreWlWindow = ecoreWlWindow;
    // check previous video subsurface and destroy
    if(mEcoreVideoShellSurface)
    {
      Ecore_Wl2_VideoShell_Surface_del(mEcoreVideoShellSurface);
    }

    // Crate ecore_wl2 sursurface
    mEcoreVideoShellSurface = Ecore_Wl2_VideoShell_Surface_new(mEcoreWlWindow);
    if(!mEcoreVideoShellSurface)
    {
      DALI_LOG_ERROR("InitializeVideoShell, ecore_wl2_subsurface_new() is failed\n");
      return;
    }

    Ecore_Wl2_VideoShell_Surface_place_below(mEcoreVideoShellSurface, nullptr);
    Ecore_Wl2_VideoShell_Surface_map(mEcoreVideoShellSurface);

    const char* videoShellHandle = Ecore_Wl2_VideoShell_Surface_handle_get(mEcoreVideoShellSurface);

    DALI_LOG_RELEASE_INFO("VideoShell(%p) handle: %s\n", mEcoreVideoShellSurface, videoShellHandle);
  }
#else
  DALI_LOG_ERROR("InitializeVideoShell, ecore_wl2_subsurface_new() NOT SUPPORT THIS TIZEN VERSION!\n");
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TizenVideoPlayer::InitializeUnderlayMode(Ecore_Wl2_Window* ecoreWlWindow)
{
  int error;
  int ret = 0;

  mEcoreWlWindow = ecoreWlWindow;

  if(mPlayerState == PLAYER_STATE_NONE)
  {
    error = player_create(&mPlayer);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("InitializeUnderlayMode, player_create() is failed\n");
    }
  }

  GetPlayerState(&mPlayerState);
  if(mPlayerState == PLAYER_STATE_IDLE)
  {
    error = player_set_completed_cb(mPlayer, EmitPlaybackFinishedSignal, this);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("InitializeUnderlayMode, player_set_completed_cb() is failed\n");
    }

    error = sound_manager_create_stream_information(mStreamType, NULL, NULL, &mStreamInfo);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("InitializeUnderlayMode, sound_manager_create_stream_information() is failed\n");
    }

    error = player_set_sound_stream_info(mPlayer, mStreamInfo);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("InitializeUnderlayMode, player_set_sound_stream_info() is failed\n");
    }

    error = player_set_video_codec_type(mPlayer, mCodecType);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("InitializeUnderlayMode, player_set_video_codec_type() is failed\n");
    }

    int                width, height;
    Ecore_Wl2_Display* wl2_display = ecore_wl2_connected_display_get(NULL);
    ecore_wl2_display_screen_size_get(wl2_display, &width, &height);
    ecore_wl2_window_alpha_set(mEcoreWlWindow, false);

    error = player_set_display_mode(mPlayer, PLAYER_DISPLAY_MODE_DST_ROI);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("InitializeUnderlayMode, player_set_display_mode() is failed\n");
    }

    error = player_set_display_roi_area(mPlayer, 0, 0, 1, 1);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("InitializeUnderlayMode, player_set_display_roi_area() is failed\n");
    }

    error = player_set_ecore_wl_display(mPlayer, PLAYER_DISPLAY_TYPE_OVERLAY, mEcoreWlWindow, 0, 0, width, height);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("InitializeUnderlayMode, player_set_ecore_wl_display() is failed\n");
    }

    error = player_set_display_visible(mPlayer, true);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("InitializeUnderlayMode, player_set_display_visible() is failed\n");
    }
  }
}

bool TizenVideoPlayer::Update()
{
  Dali::Mutex::ScopedLock lock(mPacketMutex);

  int error;

  if(mPacket != NULL)
  {
    error = media_packet_destroy(mPacket);
    if(error != MEDIA_PACKET_ERROR_NONE)
    {
      DALI_LOG_ERROR("Update, media_packet_destroy() is failed\n");
    }
    mPacket = NULL;
  }

  if(!mPacketList.empty())
  {
    mPacket = static_cast<media_packet_h>(mPacketList.front());
    mPacketList.pop_front();
  }

  if(mPacket == NULL)
  {
    return true;
  }

  tbm_surface_h tbmSurface = NULL;
  error                    = media_packet_get_tbm_surface(mPacket, &tbmSurface);
  if(error != MEDIA_PACKET_ERROR_NONE)
  {
    media_packet_destroy(mPacket);
    mPacket = NULL;
    DALI_LOG_ERROR(" error: %d\n", error);
    return true;
  }

  Any source(tbmSurface);
  mNativeImageSourcePtr->SetSource(source);
  Dali::Stage::GetCurrent().KeepRendering(0.0f);

  return true;
}

void TizenVideoPlayer::DestroyPackets()
{
  int error;
  if(mPacket != NULL)
  {
    error = media_packet_destroy(mPacket);
    DALI_LOG_ERROR("Media packet destroy error: %d\n", error);
    mPacket = NULL;
  }

  std::list<media_packet_h>::iterator iter = mPacketList.begin();
  for(; iter != mPacketList.end(); ++iter)
  {
    mPacket = *iter;
    error   = media_packet_destroy(mPacket);
    DALI_LOG_ERROR("Media packet destroy error: %d\n", error);
    mPacket = NULL;
  }
  mPacketList.clear();
}

void TizenVideoPlayer::PushPacket(media_packet_h packet)
{
  Dali::Mutex::ScopedLock lock(mPacketMutex);
  mPacketList.push_back(packet);
}

void TizenVideoPlayer::SetDisplayArea(DisplayArea area)
{
  int ret = 0;
  DALI_LOG_RELEASE_INFO("Set Display Area (%f , %f) (%f x %f)\n", area.x, area.y, area.width, area.height);
  GetPlayerState(&mPlayerState);

  if(mNativeImageSourcePtr)
  {
    DALI_LOG_ERROR("SetDisplayArea is only for window surface target.\n");
    return;
  }

  if(mPlayerState == PLAYER_STATE_IDLE ||
     mPlayerState == PLAYER_STATE_READY ||
     mPlayerState == PLAYER_STATE_PLAYING ||
     mPlayerState == PLAYER_STATE_PAUSED)
  {
    area.x = (area.x < 0) ? 0 : area.x;
    area.y = (area.y < 0) ? 0 : area.y;

    int error = player_set_display_roi_area(mPlayer, area.x, area.y, area.width, area.height);
    ret       = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("SetDisplayArea, player_set_display_roi_area() is failed\n");
    }
  }
}

void TizenVideoPlayer::Forward(int millisecond)
{
  int error;
  int ret = 0;

  GetPlayerState(&mPlayerState);

  if(mPlayerState == PLAYER_STATE_READY ||
     mPlayerState == PLAYER_STATE_PLAYING ||
     mPlayerState == PLAYER_STATE_PAUSED)
  {
    int currentPosition = 0;
    int nextPosition    = 0;

    error = player_get_play_position(mPlayer, &currentPosition);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("Forward, player_get_play_position() is failed\n");
    }

    nextPosition = currentPosition + millisecond;

    error = player_set_play_position(mPlayer, nextPosition, ACCURATE_PLAY_POSITION_SET, PlayerSeekCompletedCb, NULL);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("Forward, player_set_play_position() is failed\n");
    }
  }
}

void TizenVideoPlayer::Backward(int millisecond)
{
  int error;
  int ret = 0;

  GetPlayerState(&mPlayerState);

  if(mPlayerState == PLAYER_STATE_READY ||
     mPlayerState == PLAYER_STATE_PLAYING ||
     mPlayerState == PLAYER_STATE_PAUSED)
  {
    int currentPosition = 0;
    int nextPosition    = 0;

    error = player_get_play_position(mPlayer, &currentPosition);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("Backward, player_get_play_position() is failed\n");
    }

    nextPosition = currentPosition - millisecond;
    nextPosition = (nextPosition < 0) ? 0 : nextPosition;

    error = player_set_play_position(mPlayer, nextPosition, ACCURATE_PLAY_POSITION_SET, PlayerSeekCompletedCb, NULL);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("Backward, player_set_play_position() is failed\n");
    }
  }
}

bool TizenVideoPlayer::IsVideoTextureSupported()
{
  bool featureFlag = true;
  int  error       = SYSTEM_INFO_ERROR_NONE;

  error = system_info_get_platform_bool("tizen.org/feature/multimedia.raw_video", &featureFlag);

  if(error != SYSTEM_INFO_ERROR_NONE)
  {
    DALI_LOG_ERROR("Plugin can't check platform feature\n");
    return false;
  }

  return featureFlag;
}

void TizenVideoPlayer::DestroyPlayer()
{
  mUrl    = "";
  int ret = 0;

  int error;
  // If user take the handle, user must be responsible for its destruction.
  if(mPlayerState != PLAYER_STATE_NONE && !mIsMovedHandle)
  {
    GetPlayerState(&mPlayerState);

    if(mPlayerState != PLAYER_STATE_IDLE)
    {
      if(mNativeImageSourcePtr)
      {
        error = player_unset_media_packet_video_frame_decoded_cb(mPlayer);
        ret   = LogPlayerError(error);
        if(ret)
        {
          DALI_LOG_ERROR("DestroyPlayer, player_unset_media_packet_video_frame_decoded_cb() is failed\n");
        }
      }

      Stop();
      error = player_unprepare(mPlayer);
      ret   = LogPlayerError(error);
      if(ret)
      {
        DALI_LOG_ERROR("DestroyPlayer, player_unprepare() is failed\n");
      }
    }

    error = player_destroy(mPlayer);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("DestroyPlayer, player_destroy() is failed\n");
    }
  }

  if(mStreamInfo != NULL)
  {
    error = sound_manager_destroy_stream_information(mStreamInfo);
    ret   = LogPlayerError(error);
    if(ret)
    {
      DALI_LOG_ERROR("DestroyPlayer, sound_manager_destroy_stream_information() is failed\n");
    }
  }

  mPlayerState = PLAYER_STATE_NONE;
  mPlayer      = NULL;
  mStreamInfo  = NULL;
}

void TizenVideoPlayer::SetCodecType(Dali::VideoPlayerPlugin::CodecType type)
{
  int error;
  int ret = 0;
  switch(type)
  {
    case Dali::VideoPlayerPlugin::CodecType::HW:
    {
      mCodecType = PLAYER_CODEC_TYPE_HW;
      break;
    }
    case Dali::VideoPlayerPlugin::CodecType::SW:
    {
      mCodecType = PLAYER_CODEC_TYPE_SW;
      break;
    }
    default:
    {
      mCodecType = PLAYER_CODEC_TYPE_HW;
      break;
    }
  }

  if(mPlayerState != PLAYER_STATE_NONE)
  {
    GetPlayerState(&mPlayerState);

    if(mPlayerState == PLAYER_STATE_IDLE)
    {
      error = player_set_video_codec_type(mPlayer, mCodecType);
      ret   = LogPlayerError(error);
      if(ret)
      {
        DALI_LOG_ERROR("SetCodecType, player_set_video_codec_type() is failed\n");
      }
    }
  }
}

Dali::VideoPlayerPlugin::CodecType TizenVideoPlayer::GetCodecType() const
{
  Dali::VideoPlayerPlugin::CodecType type = Dali::VideoPlayerPlugin::CodecType::HW;
  int                                ret  = 0;
  if(mPlayerState != PLAYER_STATE_NONE)
  {
    player_codec_type_e codecType = PLAYER_CODEC_TYPE_HW;
    int                 error     = player_get_video_codec_type(mPlayer, &codecType);
    if(error != PLAYER_ERROR_NONE)
    {
      ret = LogPlayerError(error);
      if(ret)
      {
        DALI_LOG_ERROR("GetCodecType, player_get_video_codec_type() is failed\n");
      }
      return type;
    }

    switch(codecType)
    {
      case PLAYER_CODEC_TYPE_HW:
      {
        type = Dali::VideoPlayerPlugin::CodecType::HW;
        break;
      }
      case PLAYER_CODEC_TYPE_SW:
      {
        type = Dali::VideoPlayerPlugin::CodecType::SW;
        break;
      }
      default:
      {
        type = Dali::VideoPlayerPlugin::CodecType::HW;
        break;
      }
    }
  }
  return type;
}

void TizenVideoPlayer::SetDisplayMode(Dali::VideoPlayerPlugin::DisplayMode::Type mode)
{
  int error;
  error   = player_set_display_mode(mPlayer, static_cast<player_display_mode_e>(mode));
  int ret = LogPlayerError(error);
  if(ret)
  {
    DALI_LOG_ERROR("SetDisplayMode, player_set_display_mode() is failed\n");
  }
}

Dali::VideoPlayerPlugin::DisplayMode::Type TizenVideoPlayer::GetDisplayMode() const
{
  player_display_mode_e mode;
  player_get_display_mode(mPlayer, &mode);
  return static_cast<Dali::VideoPlayerPlugin::DisplayMode::Type>(mode);
}

Any TizenVideoPlayer::GetMediaPlayer()
{
  mIsMovedHandle = true;
  return Any((void*)mPlayer);
}

void TizenVideoPlayer::StartSynchronization()
{
  DALI_LOG_RELEASE_INFO("sync VideoPlayer\n");
  //ecore_wl2_subsurface_sync_set(mEcoreSubVideoWindow, EINA_TRUE);
}

void TizenVideoPlayer::FinishSynchronization()
{
  // Finish
  DALI_LOG_RELEASE_INFO("desync VideoPlayer\n");
  //ecore_wl2_subsurface_sync_set(mEcoreSubVideoWindow, EINA_FALSE);
}

void TizenVideoPlayer::CreateVideoShellConstraint()
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

      int                width, height;
      Ecore_Wl2_Display* wl2_display = ecore_wl2_connected_display_get(NULL);
      ecore_wl2_display_screen_size_get(wl2_display, &width, &height);

      Window                                window                        = DevelWindow::Get(syncActor);
      wl_egl_window*                        windowBuffer                  = Dali::AnyCast<wl_egl_window*>(DevelWindow::GetNativeBuffer(window));
      Ecore_Wl2_VideoShell_Surface_Wrapper* ecoreVidoeShellSurfaceWrapper = Ecore_Wl2_VideoShell_Surface_wrapper_new(mEcoreVideoShellSurface);

      DALI_LOG_RELEASE_INFO("Get EGL Window Surface: %p\n", windowBuffer);
      const char* videoShellHandle = Ecore_Wl2_VideoShell_Surface_handle_get(mEcoreVideoShellSurface);
      DALI_LOG_RELEASE_INFO("VideoShell(%p) handle: %s\n", mEcoreVideoShellSurface, videoShellHandle);

      mVideoShellSizePropertyConstraint = Constraint::New<Vector3>(syncActor,
                                                                   mVideoShellSizePropertyIndex,
                                                                   VideoShellSyncConstraint(ecoreVidoeShellSurfaceWrapper, windowBuffer, width, height));

      mVideoShellSizePropertyConstraint.AddSource(LocalSource(Actor::Property::SIZE));
      mVideoShellSizePropertyConstraint.AddSource(LocalSource(Actor::Property::WORLD_SCALE));
      mVideoShellSizePropertyConstraint.AddSource(LocalSource(Actor::Property::WORLD_POSITION));
    }
  }
#else
  DALI_LOG_ERROR("Video Shell Constraint NOT SUPPORT THIS TIZEN VERSION!\n");
#endif
}

void TizenVideoPlayer::DestroyVideoShellConstraint()
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

void TizenVideoPlayer::RaiseAbove(Any videoSurface)
{
  // TODO: SR Video shell's designed is completed,
  // it will be re-designed and implemented.
  // Until it is completed, the below code will be commented.

  //  Ecore_Wl2_Subsurface* surface = AnyCast<Ecore_Wl2_Subsurface*>(videoSurface);
  //  ecore_wl2_subsurface_place_surface_above(mEcoreSubVideoWindow, surface);
}

void TizenVideoPlayer::LowerBelow(Any videoSurface)
{
  // TODO: SR Video shell's designed is completed,
  // it will be re-designed and implemented.
  // Until it is completed, the below code will be commented.

  //  Ecore_Wl2_Subsurface* surface = AnyCast<Ecore_Wl2_Subsurface*>(videoSurface);
  //  ecore_wl2_subsurface_place_surface_below(mEcoreSubVideoWindow, surface);
}

void TizenVideoPlayer::RaiseToTop()
{
  // TODO: SR Video shell's designed is completed,
  // it will be re-designed and implemented.
  // Until it is completed, the below code will be commented.

  //  ecore_wl2_subsurface_place_surface_above(mEcoreSubVideoWindow, NULL);
}

void TizenVideoPlayer::LowerToBottom()
{
  // TODO: SR Video shell's designed is completed,
  // it will be re-designed and implemented.
  // Until it is completed, the below code will be commented.

  //  ecore_wl2_subsurface_place_surface_below(mEcoreSubVideoWindow, NULL);
}

Any TizenVideoPlayer::GetVideoPlayerSurface()
{
  return mEcoreSubVideoWindow;
}

void TizenVideoPlayer::SceneConnection()
{
  DALI_LOG_RELEASE_INFO("Scene is connected\n");

  if(mSyncMode == Dali::VideoSyncMode::ENABLED && mVideoShellSizePropertyIndex == Property::INVALID_INDEX)
  {
    CreateVideoShellConstraint();
    if(mVideoShellSizePropertyConstraint)
    {
      DALI_LOG_RELEASE_INFO("mVideoShellSizePropertyConstraint(%d).ApplyPost()\n", mVideoShellSizePropertyIndex);
      mVideoShellSizePropertyConstraint.ApplyPost();
    }
    else
    {
      DALI_LOG_ERROR("mVideoShellSizePropertyConstraint() creation failed!\n");
    }
  }
  mIsSceneConnected = true;
}

void TizenVideoPlayer::SceneDisconnection()
{
  DALI_LOG_RELEASE_INFO("Scene is disconnected\n");

  if(mSyncMode == Dali::VideoSyncMode::ENABLED && mVideoShellSizePropertyIndex != Property::INVALID_INDEX)
  {
    DALI_LOG_RELEASE_INFO("mVideoShellSizePropertyConstraint(%d).Remove()\n", mVideoShellSizePropertyIndex);
    DestroyVideoShellConstraint();
  }
  mIsSceneConnected = false;
}


void TizenVideoPlayer::SetAutoRotationEnabled(bool enable)
{
  if(!mNativeImageSourcePtr)
  {
    DALI_LOG_ERROR("SetAutoRotationEnabled is only for native image rendering target.\n");
    return;
  }

  DALI_LOG_RELEASE_INFO("SetAutoRotationEnabled not yet supported.\n");
}

bool TizenVideoPlayer::IsAutoRotationEnabled() const
{
  if(!mNativeImageSourcePtr)
  {
    DALI_LOG_ERROR("IsAutoRotationEnabled is only for native image rendering target.\n");
    return false;
  }

  DALI_LOG_RELEASE_INFO("IsAutoRotationEnabled not yet supported.\n");
  return false;
}

void TizenVideoPlayer::SetLetterBoxEnabled(bool enable)
{
  if(!mNativeImageSourcePtr)
  {
    DALI_LOG_ERROR("SetLetterBoxEnabled is only for native image rendering target.\n");
    return;
  }

  DALI_LOG_RELEASE_INFO("SetLetterBoxEnabled not yet supported.\n");
}

bool TizenVideoPlayer::IsLetterBoxEnabled() const
{
  if(!mNativeImageSourcePtr)
  {
    DALI_LOG_ERROR("IsLetterBoxEnabled is only for native image rendering target.\n");
    return false;
  }

  DALI_LOG_RELEASE_INFO("IsLetterBoxEnabled not yet supported.\n");
  return false;
}

} // namespace Plugin
} // namespace Dali
