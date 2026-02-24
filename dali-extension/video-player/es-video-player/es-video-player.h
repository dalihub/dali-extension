#ifndef DALI_EXTENSION_VIDEO_PLAYER_ES_VIDEO_PLAYER_H
#define DALI_EXTENSION_VIDEO_PLAYER_ES_VIDEO_PLAYER_H

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

// INTERNAL INCLUDES
#include <video-player-base.h>

// EXTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/timer.h>
#include <esplusplayer_capi/esplusplayer_capi.h>
#include <esplusplayer_capi/esplusplayer_internal.h>
#include <string>
#include <vector>
#include <memory>

namespace Dali
{
namespace Plugin
{

/**
 * @brief Implementation of the ES+ video player class using esplayer handle.
 *
 * This class provides video playback functionality using the esplusplayer handle.
 * It supports external player handle injection and actor-based synchronization.
 * It extends VideoPlayerBase which contains common logic shared with TizenVideoPlayer.
 */
class EsVideoPlayer : public VideoPlayerBase
{
public:
  /**
   * @brief Constructor with external player handle and actor for synchronization.
   *
   * Creates a new EsVideoPlayer instance using an externally created esplusplayer handle.
   The actor parameter enables synchronization features between the UI and video player.
   *
   * @param[in] playerHandle The externally created esplusplayer handle with type information
   * @param[in] syncMode The synchronization mode between the UI and VideoPlayer
   * @param[in] syncActor The actor for synchronization with the video player. Can be empty if synchronization is not needed.
   */
  EsVideoPlayer(Dali::VideoPlayerPlugin::PlayerHandle playerHandle, Dali::VideoSyncMode syncMode, Dali::Actor syncActor);

  /**
   * @brief Destructor.
   */
  virtual ~EsVideoPlayer();

protected:

// Template Method Pattern: Primitive operations
  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::IsPlayerReady()
   */
  virtual bool IsPlayerReady() override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::DoInitializePlayer()
   */
  virtual void DoInitializePlayer() override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::DoPlay()
   */
  virtual void DoPlay() override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::DoPause()
   */
  virtual void DoPause() override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::DoStop()
   */
  virtual void DoStop() override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::DoSetMute()
   */
  virtual void DoSetMute(bool mute) override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::DoSetVolume()
   */
  virtual void DoSetVolume(float left, float right) override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::DoSetLooping()
   */
  virtual void DoSetLooping(bool looping) override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::DoSetUrl()
   */
  virtual void DoSetUrl(const std::string& url) override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::DoGetPlayPosition()
   */
  virtual int DoGetPlayPosition() override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::DoSetPlayPosition()
   */
  virtual void DoSetPlayPosition(int millisecond) override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::DoSetDisplayRotation()
   */
  virtual void DoSetDisplayRotation(Dali::VideoPlayerPlugin::DisplayRotation rotation) override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::DoGetDisplayRotation()
   */
  virtual Dali::VideoPlayerPlugin::DisplayRotation DoGetDisplayRotation() override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::DoSetDisplayArea()
   */
  virtual void DoSetDisplayArea(DisplayArea area) override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::DoInitializeTextureStreamMode()
   */
  virtual void DoInitializeTextureStreamMode(Dali::NativeImagePtr nativeImagePtr) override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::DoGetMediaPlayer()
   */
  virtual Any DoGetMediaPlayer() override;

  // Hook methods for post-processing
  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::OnAfterPlay()
   */
  virtual void OnAfterPlay() override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::OnAfterPause()
   */
  virtual void OnAfterPause() override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::OnAfterStop()
   */
  virtual void OnAfterStop() override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::DoSetCodecType()
   */
  virtual void DoSetCodecType(Dali::VideoPlayerPlugin::CodecType type) override {}

  // Override base class methods that need specific implementation
  /**
   * @copydoc Dali::VideoPlayerPlugin::IsVideoTextureSupported()
   */
  bool IsVideoTextureSupported() override { return true; }

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetCodecType()
   */
  Dali::VideoPlayerPlugin::CodecType GetCodecType() const override { return mCodecType; }

  /**
   * @copydoc Dali::VideoPlayerPlugin::DoSetDisplayMode()
   */
  virtual void DoSetDisplayMode(Dali::VideoPlayerPlugin::DisplayMode::Type mode) override {}

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetDisplayMode()
   */
  Dali::VideoPlayerPlugin::DisplayMode::Type GetDisplayMode() const override { return mDisplayMode; }

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetMediaPlayer()
   */
  Any GetMediaPlayer() override { return Any((void*)mEsPlayer); }

  /**
   * @copydoc Dali::VideoPlayerPlugin::StartSynchronization()
   */
  void StartSynchronization() override { /* EsVideoPlayer specific implementation */ }

  /**
   * @copydoc Dali::VideoPlayerPlugin::FinishSynchronization()
   */
  void FinishSynchronization() override { /* EsVideoPlayer specific implementation */ }

  /**
   * @copydoc Dali::VideoPlayerPlugin::RaiseAbove()
   */
  void RaiseAbove(Any videoSurface) override { /* EsVideoPlayer specific implementation */ }

  /**
   * @copydoc Dali::VideoPlayerPlugin::LowerBelow()
   */
  void LowerBelow(Any videoSurface) override { /* EsVideoPlayer specific implementation */ }

  /**
   * @copydoc Dali::VideoPlayerPlugin::RaiseToTop()
   */
  void RaiseToTop() override { /* EsVideoPlayer specific implementation */ }

  /**
   @copydoc Dali::VideoPlayerPlugin::LowerToBottom()
   */
  void LowerToBottom() override { /* EsVideoPlayer specific implementation */ }

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetVideoPlayerSurface()
   */
  Any GetVideoPlayerSurface() override { return Any(); }

  /**
   * @copydoc Dali::VideoPlayerPlugin::SceneConnection()
   */
  void SceneConnection() override { /* EsVideoPlayer specific implementation */ }

  /**
   * @copydoc Dali::VideoPlayerPlugin::SceneDisconnection()
   */
  void SceneDisconnection() override { /* EsVideoPlayer specific implementation */ }

protected:
  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::DestroyMediaPacket()
   */
  virtual void DestroyMediaPacket(void* packet) override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::GetSurfaceFromPacket()
   */
  virtual Any GetSurfaceFromPacket(void* packet) override;

private:

  /**
   * @brief Logs player error messages.
   *
   * Converts esplusplayer error codes to human-readable log messages
   * for debugging purposes.
   *
   * @param[in] error The error code from esplusplayer
   */
  void LogPlayerError(int error);

  /**
   * @brief Destroys the esplusplayer instance.
   *
   * Properly cleans up all esplusplayer resources and shuts down the player.
   * Called during destructor or when the player needs to be recreated.
   */
  void DestroyPlayer();

  // EsVideoPlayer specific member variables
  esplusplayer_handle mEsPlayer;
  Dali::Mutex                                     mPacketMutex;
  std::vector<esplusplayer_decoded_video_packet*> mPacketVector;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_EXTENSION_VIDEO_PLAYER_ES_VIDEO_PLAYER_H
