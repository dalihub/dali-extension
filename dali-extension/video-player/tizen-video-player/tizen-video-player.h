#ifndef __DALI_TIZEN_VIDEO_PLAYER_PLUGIN_H__
#define __DALI_TIZEN_VIDEO_PLAYER_PLUGIN_H__

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
#include "tizen-video-constraint-helper.h"

// EXTERNAL INCLUDES
#include <dali/public-api/animation/constraints.h>
#include <player.h>
#include <list>
#include <string>
#include <unordered_map>

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
 *
 * This class extends VideoPlayerBase which contains common logic shared with EsVideoPlayer.
 * The template method pattern is applied, with platform-specific operations implemented in DoXxx methods.
 */
class TizenVideoPlayer : public VideoPlayerBase
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
   * @brief Construct a new TizenVideoPlayer using existing player handle.
   */
  TizenVideoPlayer(Dali::VideoPlayerPlugin::PlayerHandle playerHandle, Dali::VideoSyncMode syncMode, Dali::Actor syncActor);

  /**
   * @brief Destructor.
   * @SINCE_1_1.38
   */
  virtual ~TizenVideoPlayer();

protected:
  virtual bool IsPlayerReady() override;

  // Template Method Pattern: Primitive operations
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
   * @copydoc Dali::Plugin::VideoPlayerBase::DoSetDisplayMode()
   */
  virtual void DoSetDisplayMode(Dali::VideoPlayerPlugin::DisplayMode::Type mode) override;

  /**
   * @copydoc Dali::Plugin::VideoPlayerBase::DoSetCodecType()
   */
  virtual void DoSetCodecType(Dali::VideoPlayerPlugin::CodecType type) override;

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

  // Override base class methods that need specific implementation
  /**
   * @copydoc Dali::VideoPlayerPlugin::IsVideoTextureSupported()
   */
  bool IsVideoTextureSupported() override { return true; }

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetCodecType()
   */
  Dali::VideoPlayerPlugin::CodecType GetCodecType() const override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetDisplayMode()
   */
  Dali::VideoPlayerPlugin::DisplayMode::Type GetDisplayMode() const override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetMediaPlayer()
   */
  Any GetMediaPlayer() override { return DoGetMediaPlayer(); }

  /**
   * @copydoc Dali::VideoPlayerPlugin::StartSynchronization()
   */
  void StartSynchronization() override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::FinishSynchronization()
   */
  void FinishSynchronization() override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::RaiseAbove()
   */
  void RaiseAbove(Any videoSurface) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::LowerBelow()
   */
  void LowerBelow(Any videoSurface) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::RaiseToTop()
   */
  void RaiseToTop() override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::LowerToBottom()
   */
  void LowerToBottom() override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetVideoPlayerSurface()
   */
  Any GetVideoPlayerSurface() override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::SceneConnection()
   */
  void SceneConnection() override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::SceneDisconnection()
   */
  void SceneDisconnection() override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetRenderingTarget()
   */
  void SetRenderingTarget(Any target) override;

  // TizenVideoPlayer specific methods
  /**
   * @brief Initializes the video player for texture stream mode rendering.
   *
   * Configures the player to render video frames into a native image source
   * that can be used by the UI toolkit for texture-based rendering.
   *
   * @param[in] nativeImagePtr The native image source for video frame rendering
   */
  void InitializeTextureStreamMode(Dali::NativeImagePtr nativeImagePtr);

  /**
   * @brief Initializes the video player for underlay (window surface) mode rendering.
   *
   * Configures the player to render video directly to a wayland window surface
   * that appears underneath the UI layer with a transparent hole.
   *
   * @param[in] ecoreWlWindow The ecore wayland window handle for underlay rendering
   */
  void InitializeUnderlayMode(Ecore_Wl2_Window* ecoreWlWindow);

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetAutoRotationEnabled()
   */
  void SetAutoRotationEnabled(bool enable) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::IsAutoRotationEnabled()
   */
  bool IsAutoRotationEnabled() const override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetLetterBoxEnabled()
   */
  void SetLetterBoxEnabled(bool enable) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::IsLetterBoxEnabled()
   */
  bool IsLetterBoxEnabled() const override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetFrameInterpolationInterval()
   */
  void SetFrameInterpolationInterval(float intervalSeconds) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::EnableOffscreenFrameRendering()
   */
  void EnableOffscreenFrameRendering(bool useOffScreenFrame, Dali::NativeImagePtr previousFrameBuffernativeImagePtr, Dali::NativeImagePtr currentFrameBuffernativeImagePtr) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetVideoFrameBuffer()
   */
  void SetVideoFrameBuffer(Dali::NativeImagePtr source) override;

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
  friend void EmitPlaybackFinishedSignal(void* user_data);

  /**
   * @brief Destroys the video constraint used for synchronization.
   *
   * Cleans up the constraint that synchronizes the video player with the UI layer.
   */
  void DestroyVideoConstraint();

private:
  /**
   * @brief Gets current player state
   */
  void GetPlayerState(player_state_e* state) const;

  /**
   * @brief Initializes player for video rendering using wayland window surface
   */
  void InitializeVideoShell(Ecore_Wl2_Window* ecoreWlWindow);

  /**
   * @brief Destroys player handle
   */
  void DestroyPlayer();

  /**
   * @brief Create Video Shell Constraint for synchronization
   */
  void CreateVideoShellConstraint();

  /**
   * @brief Destroy Video Shell Constraint for synchronization
   */
  void DestroyVideoShellConstraint();

  // TizenVideoPlayer specific member variables (not in base class)
  player_h                   mPlayer;               ///< Tizen player handle
  player_state_e             mPlayerState;          ///< Tizen player state
  Dali::Vector4              mBackgroundColor;      ///< Current background color, which texturestream mode needs.
  RenderingTargetType        mTargetType;           ///< Current rendering target type

  sound_stream_info_h mStreamInfo;
  sound_stream_type_e mStreamType;

  Ecore_Wl2_Window*             mEcoreWlWindow;       ///< ecore native window handle
  Ecore_Wl2_Subsurface*         mEcoreSubVideoWindow; ///< ecore native subsurface for synchronization with video player
  Constraint                    mVideoSizePropertyConstraint;
  Constraint                    mVideoFrameBufferConstraint;
  Constraint                    mVideoFrameBufferProgressPropertyConstraint;
  Property::Index               mVideoFrameBufferProgressPropertyIndex;

  int32_t mVideoConstraintHelperId; ///< unique id for Rotation and Ratio constraint helper

  bool mIsMovedHandle; ///< the flag for moved the handle
  bool mIsSceneConnected;
  bool mIsExternalPlayer; ///< indicates if the player handle was externally injected

#ifdef OVER_TIZEN_VERSION_9
  Ecore_Wl2_VideoShell_Surface* mEcoreVideoShellSurface;
#endif
  Constraint                                mVideoShellSizePropertyConstraint;
  Property::Index                           mVideoShellSizePropertyIndex;
  Dali::IntrusivePtr<VideoConstraintHelper> mVideoConstraintHelper;
};

} // namespace Plugin
} // namespace Dali

#endif