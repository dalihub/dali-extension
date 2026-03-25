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
#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/debug.h>
#include <tbm_surface.h>

namespace Dali
{
namespace Plugin
{

namespace
{
const int MAX_QUEUE_SIZE = 3;

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

} // namespace

EsVideoPlayer::EsVideoPlayer(Dali::VideoPlayerPlugin::PlayerHandle playerHandle, Dali::VideoSyncMode syncMode, Dali::Actor syncActor)
: VideoPlayerBase(syncMode, syncActor),
  mEsPlayer(nullptr)
{
  if(!playerHandle.handle.Empty())
  {
    mEsPlayer = static_cast<esplusplayer_handle>(AnyCast<void*>(playerHandle.handle));
    DALI_LOG_RELEASE_INFO("EsVideoPlayer initialized: %p\n", mEsPlayer);
  }
  else
  {
    DALI_LOG_ERROR("EsVideoPlayer initialized with invalid handle type!\n");
  }
}

EsVideoPlayer::~EsVideoPlayer()
{
  DALI_LOG_RELEASE_INFO("EsVideoPlayer Destructor\n");

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
  int error = esplusplayer_set_volume(mEsPlayer, volume);
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

  uint64_t currentTime = 0;
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
    default :
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
  if (mEsPlayer)
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
  if (error != ESPLUSPLAYER_ERROR_TYPE_NONE)
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
    if(playerState != ESPLUSPLAYER_STATE_NONE)
    {
      if(playerState == ESPLUSPLAYER_STATE_IDLE)
      {
        if(mNativeImagePtr)
        {
          int error = esplusplayer_set_media_packet_video_decoded_cb(mEsPlayer, nullptr, nullptr);
          LogPlayerError(error);
        }

        DoStop();
      }
    }
  }

  ClearPackets();
}

} // namespace Plugin
} // namespace Dali
