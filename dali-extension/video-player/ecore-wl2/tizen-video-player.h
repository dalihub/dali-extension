#ifndef __DALI_TIZEN_VIDEO_PLAYER_PLUGIN_H__
#define __DALI_TIZEN_VIDEO_PLAYER_PLUGIN_H__

/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#include "tizen-video-constraint-helper.h"

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/video-player-plugin.h>
#include <dali/devel-api/adaptor-framework/video-sync-mode.h>
#include <dali/devel-api/threading/mutex.h>
#include <dali/public-api/adaptor-framework/native-image-source.h>
#include <dali/public-api/adaptor-framework/timer.h>
#include <dali/public-api/animation/constraints.h>
#include <dali/public-api/object/weak-handle.h>
#include <player.h>
#include <list>
#include <string>

#ifndef HAVE_WAYLAND
#define HAVE_WAYLAND
#endif
#include <Ecore_Wl2.h>
#include <player_internal.h>

namespace Dali
{
namespace Plugin
{
/**
 * @brief Implementation of the Tizen video player class which has Tizen platform dependency.
 * @SINCE_1_1.38
 */
class TizenVideoPlayer : public Dali::VideoPlayerPlugin, public Dali::ConnectionTracker
{
public:
  /**
   * @brief Video rendering target type
   * @SINCE_1_1.38
   */
  enum RenderingTargetType
  {
    WINDOW_SURFACE, ///< HW underlay @SINCE_1_9.28
    NATIVE_IMAGE    ///< texture stream @SINCE_1_9.28
  };

  /**
   * @brief Constructor.
   */
  TizenVideoPlayer(Dali::Actor actor, Dali::VideoSyncMode syncMode);

  /**
   * @brief Destructor.
   * @SINCE_1_1.38
   */
  virtual ~TizenVideoPlayer();

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetUrl()
   */
  virtual void SetUrl(const std::string& url);

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetUrl()
   */
  virtual std::string GetUrl();

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetLooping()
   */
  virtual void SetLooping(bool looping);

  /**
   * @copydoc Dali::VideoPlayerPlugin::IsLooping()
   */
  virtual bool IsLooping();

  /**
   * @copydoc Dali::VideoPlayerPlugin::Play()
   */
  virtual void Play();

  /**
   * @copydoc Dali::VideoPlayerPlugin::Pause()
   */
  virtual void Pause();

  /**
   * @copydoc Dali::VideoPlayerPlugin::Stop()
   */
  virtual void Stop();

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetMute()
   */
  virtual void SetMute(bool mute);

  /**
   * @copydoc Dali::VideoPlayerPlugin::IsMute()
   */
  virtual bool IsMuted();

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetVolume()
   */
  virtual void SetVolume(float left, float right);

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetVolume()
   */
  virtual void GetVolume(float& left, float& right);

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetRenderingTarget()
   */
  void SetRenderingTarget(Any target);

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetPlayPosition()
   */
  virtual void SetPlayPosition(int millisecond);

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetPlayPosition()
   */
  virtual int GetPlayPosition();

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetDisplayArea()
   */
  virtual void SetDisplayArea(DisplayArea area);

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetDisplayRotation()
   */
  virtual void SetDisplayRotation(Dali::VideoPlayerPlugin::DisplayRotation rotation);

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetDisplayRotation()
   */
  virtual Dali::VideoPlayerPlugin::DisplayRotation GetDisplayRotation();

  /**
   * @copydoc Dali::VideoPlayerPlugin::FinishedSignal()
   */
  virtual Dali::VideoPlayerPlugin::VideoPlayerSignalType& FinishedSignal();

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetAutoRotationEnabled()
   */
  virtual void SetAutoRotationEnabled(bool enable);

  /**
   * @copydoc Dali::VideoPlayerPlugin::IsAutoRotationEnabled() const
   */
  virtual bool IsAutoRotationEnabled() const;

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetLetterBoxEnabled()
   */
  virtual void SetLetterBoxEnabled(bool enable);

  /**
   * @copydoc Dali::VideoPlayerPlugin::IsLetterBoxEnabled() const
   */
  virtual bool IsLetterBoxEnabled() const;

  /**
   * @brief Push media packet with video frame image
   */
  void PushPacket(media_packet_h packet);

  /**
   * @brief Dali::VideoPlayer::Forward()
   */
  void Forward(int millisecond);

  /**
   * @brief Dali::VideoPlayer::Backward()
   */
  void Backward(int millisecond);

  /**
   * @brief Dali::VideoPlayer::IsVideoTextureSupported()
   */
  bool IsVideoTextureSupported();

  /**
   * @brief Dali::VideoPlayer::SetCodecType()
   */
  void SetCodecType(Dali::VideoPlayerPlugin::CodecType type);

  /**
   * @brief Dali::VideoPlayer::GetCodecType()
   */
  Dali::VideoPlayerPlugin::CodecType GetCodecType() const;

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetDisplayMode()
   */
  void SetDisplayMode(Dali::VideoPlayerPlugin::DisplayMode::Type mode);

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetDisplayMode()
   */
  Dali::VideoPlayerPlugin::DisplayMode::Type GetDisplayMode() const;

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetMediaPlayer()
   */
  Any GetMediaPlayer();

  /**
   * @copydoc Dali::VideoPlayerPlugin::StartSynchronization()
   */
  void StartSynchronization();

  /**
   * @copydoc Dali::VideoPlayerPlugin::FinishSynchronization()
   */
  void FinishSynchronization();

  /**
   * @copydoc Dali::VideoPlayerPlugin::RaiseAbove()
   */
  void RaiseAbove(Any videoSurface);

  /**
   * @copydoc Dali::VideoPlayerPlugin::LowerBelow()
   */
  void LowerBelow(Any videoSurface);

  /**
   * @copydoc Dali::VideoPlayerPlugin::RaiseToTop()
   */
  void RaiseToTop();

  /**
   * @copydoc Dali::VideoPlayerPlugin::LowerToBottom()
   */
  void LowerToBottom();

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetVideoPlayerSurface()
   */
  Any GetVideoPlayerSurface();

  /**
   * @copydoc Dali::VideoPlayerPlugin::SceneConnection()
   */
  void SceneConnection();

  /**
   * @copydoc Dali::VideoPlayerPlugin::SceneDisconnection()
   */
  void SceneDisconnection();

private:
  /**
   * @brief Update video frame image to native image source
   */
  void Update();

  /**
   * @brief Gets current player state
   */
  void GetPlayerState(player_state_e* state) const;

  /**
   * @brief Destroy all packests, which this plugin stores
   */
  void DestroyPackets();

  /**
   * @brief Initializes player for video rendering using native image source
   */
  void InitializeTextureStreamMode(Dali::NativeImageSourcePtr nativeImageSourcePtr);

  /**
   * @brief Initializes player for video rendering using wayland window surface
   */
  void InitializeUnderlayMode(Ecore_Wl2_Window* ecoreWlWindow);

  /**
   * @brief Destroys player handle
   */
  void DestroyPlayer();

  /**
   * @brief Initializes Video Shell for synchronization
   */
  void InitializeVideoShell(Ecore_Wl2_Window* ecoreWlWindow);

  /**
   * @brief Create Video Shell Constraint for synchronization
   */
  void CreateVideoShellConstraint();

  /**
   * @brief Destroy Video Shell Constraint for synchronization
   */
  void DestroyVideoShellConstraint();

  /**
   * @brief Create Constraint for oriention and ratio of video
   */
  void CreateVideoConstraint(Dali::NativeImageSourcePtr nativeImageSourcePtr);

  /**
   * @brief Destroy Constraint for oriention and ratio of video
   */
  void DestroyVideoConstraint();

private:
  std::string                mUrl;                  ///< The video file path
  player_h                   mPlayer;               ///< Tizen player handle
  player_state_e             mPlayerState;          ///< Tizen player state
  media_packet_h             mPacket;               ///< Media packet handle with tbm surface of current video frame image
  Dali::NativeImageSourcePtr mNativeImageSourcePtr; ///< native image source for video rendering
  Dali::Vector4              mBackgroundColor;      ///< Current background color, which texturestream mode needs.
  RenderingTargetType        mTargetType;           ///< Current rendering target type

  Dali::Mutex               mPacketMutex;
  std::list<media_packet_h> mPacketList; ///< Container for media packet handle from Tizen player callback

  sound_stream_info_h mStreamInfo;
  sound_stream_type_e mStreamType;

  player_codec_type_e mCodecType;

  Ecore_Wl2_Window*             mEcoreWlWindow;       ///< ecore native window handle
  Ecore_Wl2_Subsurface*         mEcoreSubVideoWindow; ///< ecore native subsurface for synchronization with video player
  Dali::WeakHandle<Dali::Actor> mSyncActor;
  Constraint                    mVideoSizePropertyConstraint;
  Constraint                    mVideoRotationConstraint;
  Constraint                    mVideoLetterBoxConstraint;
  Property::Index               mVideoSizePropertyIndex;
  Property::Index               mVideoRotationPropertyIndex;
  Property::Index               mVideoRatioPropertyIndex;
  Dali::VideoSyncMode           mSyncMode;

  int32_t mVideoConstraintHelperId; ///< unique id for Rotation and Ratio constraint helper

  bool mIsMovedHandle; ///< the flag for moved the handle
  bool mIsSceneConnected;

#ifdef OVER_TIZEN_VERSION_9
  Ecore_Wl2_VideoShell_Surface* mEcoreVideoShellSurface;
#endif
  Constraint                                mVideoShellSizePropertyConstraint;
  Property::Index                           mVideoShellSizePropertyIndex;
  Dali::IntrusivePtr<VideoConstraintHelper> mVideoConstraintHelper;

public:
  Dali::VideoPlayerPlugin::VideoPlayerSignalType mFinishedSignal;
};

} // namespace Plugin
} // namespace Dali

#endif
