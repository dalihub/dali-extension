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

#ifndef DALI_EXTENSION_VIDEO_PLAYER_BASE_H
#define DALI_EXTENSION_VIDEO_PLAYER_BASE_H

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/event-thread-callback.h>
#include <dali/devel-api/adaptor-framework/video-player-plugin.h>
#include <dali/devel-api/adaptor-framework/video-sync-mode.h>
#include <dali/devel-api/threading/mutex.h>
#include <dali/public-api/object/any.h>
#include <dali/public-api/object/weak-handle.h>
#include <dali/public-api/adaptor-framework/native-image.h>
#include <string>
#include <memory>
#include <queue>
#include <deque>
#include <functional>

namespace Dali
{
namespace Plugin
{

/**
 * @brief Base class for video player implementations.
 *
 * This class implements the Template Method pattern, providing a skeleton algorithm
 * for video playback operations while allowing subclasses to customize specific steps.
 * It contains common logic shared between some player.
 */
class VideoPlayerBase : public Dali::VideoPlayerPlugin, public Dali::ConnectionTracker
{
public:
  using Command = std::function<void()>;
  /**
   * @brief Constructor.
   *
   * @param[in] syncMode The synchronization mode between the UI and VideoPlayer
   * @param[in] syncActor The actor for synchronization with the video player
   */
  VideoPlayerBase(Dali::VideoSyncMode syncMode, Dali::Actor syncActor);

  /**
   * @brief Destructor.
   */
  virtual ~VideoPlayerBase();

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetUrl()
   */
  void SetUrl(const std::string& url) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetUrl()
   */
  std::string GetUrl() override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetLooping()
   */
  void SetLooping(bool looping) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::IsLooping()
   */
  bool IsLooping() override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::Play()
   */
  void Play() override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::Pause()
   */
  void Pause() override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::Stop()
   */
  void Stop() override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetMute()
   */
  void SetMute(bool mute) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::IsMuted()
   */
  bool IsMuted() override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetVolume()
   */
  void SetVolume(float left, float right) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetVolume()
   */
  void GetVolume(float& left, float& right) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetRenderingTarget()
   */
  void SetRenderingTarget(Any target) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetPlayPosition()
   */
  void SetPlayPosition(int millisecond) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetPlayPosition()
   */
  int GetPlayPosition() override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetDisplayArea()
   */
  void SetDisplayArea(DisplayArea area) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetDisplayRotation()
   */
  void SetDisplayRotation(Dali::VideoPlayerPlugin::DisplayRotation rotation) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetDisplayRotation()
   */
  Dali::VideoPlayerPlugin::DisplayRotation GetDisplayRotation() override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::FinishedSignal()
   */
  Dali::VideoPlayerPlugin::VideoPlayerSignalType& FinishedSignal() override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::EventSignal()
   */
  virtual Dali::VideoPlayerPlugin::VideoPlayerEventSignalType& EventSignal() override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetAutoRotationEnabled()
   */
  void SetAutoRotationEnabled(bool enable) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::IsAutoRotationEnabled() const
   */
  bool IsAutoRotationEnabled() const override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetLetterBoxEnabled()
   */
  void SetLetterBoxEnabled(bool enable) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::IsLetterBoxEnabled() const
   */
  bool IsLetterBoxEnabled() const override;

  /**
   * @brief Sets the frame interpolation interval for smooth video playback.
   *
   * The interpolation factor will progress from 0.0 to 1.0 over this duration.
   * This interval is applied after the next call to SetVideoFrameBuffer.
   *
   * @param[in] intervalSeconds The interpolation interval in seconds
   */
  void SetFrameInterpolationInterval(float intervalSeconds) override;

  /**
   * @brief Enables or disables offscreen frame rendering for video interpolation.
   *
   * When enabled, the video player will use offscreen rendering for frame interpolation,
   * which can improve visual quality for certain video content.
   *
   * @param[in] useOffScreenFrame True to enable offscreen frame rendering, false to disable
   * @param[in] previousFrameBufferNativeSourcePtr Native image for previous frame buffer
   * @param[in] currentFrameBufferNativeSourcePtr Native image for current frame buffer
   */
  void EnableOffscreenFrameRendering(bool useOffScreenFrame, Dali::NativeImagePtr previousFrameBufferNativeSourcePtr, Dali::NativeImagePtr currentFrameBufferNativeSourcePtr) override;

  /**
   * @brief Sets the video frame buffer for rendering.
   *
   * This method sets the native image that will be used as the frame buffer
   * for video rendering. The frame buffer contains the surface data for video playback.
   *
   * @param[in] source The video frame buffer source containing surface data
   */
  void SetVideoFrameBuffer(Dali::NativeImagePtr source) override;

  /**
   * @brief Seeks forward by the specified number of milliseconds.
   *
   * @param[in] millisecond The position for forward playback
   */
  void Forward(int millisecond);

  /**
   * @brief Seeks backward by the specified number of milliseconds.
   *
   * @param[in] millisecond The position for backward playback
   */
  void Backward(int millisecond);

  /**
   * @copydoc Dali::VideoPlayerPlugin::IsVideoTextureSupported()
   */
  bool IsVideoTextureSupported() override;

  /**
   * @brief Posts a command to the message queue.
   * If the player is ready, it may be processed immediately, otherwise deferred.
   * @param[in] command The command to execute
   */
  void PostCommand(Command command);

  /**
   * @brief Processes all pending commands in the queue.
   * Subclasses can call this when the underlying player transitions to a READY state.
   */
  void ProcessCommandQueue();

  /**
   * @brief Protected method for callback functions to emit finished signal.
   *
   * This method allows callback functions (like EmitPlaybackFinishedSignal) to emit
   * the finished signal without needing direct access to the protected mFinishedSignal member.
   */
  void EmitFinishedSignal()
  {
    if(!mFinishedSignal.Empty())
    {
      mFinishedSignal.Emit();
    }
    if(!mEventSignal.Empty())
    {
      mEventSignal.Emit(Dali::VideoPlayerPlugin::PlayerEventType::PLAYBACK_FINISHED);
    }
  }

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetCodecType()
   */
  void SetCodecType(Dali::VideoPlayerPlugin::CodecType type) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetCodecType()
   */
  Dali::VideoPlayerPlugin::CodecType GetCodecType() const override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::SetDisplayMode()
   */
  void SetDisplayMode(Dali::VideoPlayerPlugin::DisplayMode::Type mode) override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetDisplayMode()
   */
  Dali::VideoPlayerPlugin::DisplayMode::Type GetDisplayMode() const override;

  /**
   * @copydoc Dali::VideoPlayerPlugin::GetMediaPlayer()
   */
  Any GetMediaPlayer() override;

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
   * @brief Triggers a UI update asynchronously from a worker thread.
   * This will eventually call DoUpdateUi() on the main event thread.
   */
  void TriggerUiUpdate();

  /**
   * @brief Pushes a new decoded packet to the queue.
   * Thread-safe. Called by decoding thread.
   */
  void PushPacket(void* packet);

protected:
  // Template Method Pattern: Primitive operations to be implemented by subclasses

  /**
   * @brief Initializes the UI update callback mechanism.
   * Subclasses should call this when they need to start triggering UI updates from worker threads.
   */
  void InitializeUiUpdateCallback();


  /**
   * @brief Called on the main UI thread when a UI update is triggered.
   * Subclasses should override this to perform the actual rendering update (e.g., setting native image source).
   */
  virtual void DoUpdateUi();

  /**
   * @brief Called to trigger continuous rendering for smooth video playback.
   * Subclasses can override this to keep rendering active.
   */
  virtual void KeepRendering() {}

  /**
   * @brief Checks if the player is in a safe state to process commands.
   * Default implementation returns true. Subclasses can override.
   * @return true if ready to process, false otherwise
   */
  virtual bool IsPlayerReady();

  /**
   * @brief Initializes the underlying player handle.
   *
   * This is a template method hook that subclasses must implement.
   */
  virtual void DoInitializePlayer() = 0;

  /**
   * @brief Platform-specific play implementation.
   *
   * This is a template method hook that subclasses must implement.
   */
  virtual void DoPlay() = 0;

  /**
   * @brief Platform-specific pause implementation.
   *
   * This is a template method hook that subclasses must implement.
   */
  virtual void DoPause() = 0;

  /**
   * @brief Platform-specific stop implementation.
   *
   * This is a template method hook that subclasses must implement.
   */
  virtual void DoStop() = 0;

  /**
   * @brief Platform-specific mute implementation.
   *
   * @param[in] mute True to mute, false to unmute
   * This is a template method hook that subclasses must implement.
   */
  virtual void DoSetMute(bool mute) = 0;

  /**
   * @brief Platform-specific volume implementation.
   *
   * @param[in] left Left channel volume (0.0 to 1.0)
   * @param[in] right Right channel volume (0.0 to 1.0)
   * This is a template method hook that subclasses must implement.
   */
  virtual void DoSetVolume(float left, float right) = 0;

  /**
   * @brief Platform-specific looping implementation.
   *
   * @param[in] looping True to enable looping, false to disable
   * This is a template method hook that subclasses must implement.
   */
  virtual void DoSetLooping(bool looping) = 0;

  /**
   * @brief Platform-specific URL setting implementation.
   *
   * @param[in] url The video URL
   * This is a template method hook that subclasses must implement.
   */
  virtual void DoSetUrl(const std::string& url) = 0;

  /**
   * @brief Platform-specific position getting implementation.
   *
   * @return Current playback position in milliseconds
   * This is a template method hook that subclasses must implement.
   */
  virtual int DoGetPlayPosition() = 0;

  /**
   * @brief Platform-specific position setting implementation.
   *
   * @param[in] millisecond The position in milliseconds
   * This is a template method hook that subclasses must implement.
   */
  virtual void DoSetPlayPosition(int millisecond) = 0;

  /**
   * @brief Platform-specific rotation setting implementation.
   *
   * @param[in] rotation The display rotation
   * This is a template method hook that subclasses must implement.
   */
  virtual void DoSetDisplayRotation(Dali::VideoPlayerPlugin::DisplayRotation rotation) = 0;

  /**
   * @brief Platform-specific rotation getting implementation.
   *
   * @return Current display rotation
   * This is a template method hook that subclasses must implement.
   */
  virtual Dali::VideoPlayerPlugin::DisplayRotation DoGetDisplayRotation() = 0;

  /**
   * @brief Platform-specific display area setting implementation.
   *
   * @param[in] area The display area
   * This is a template method hook that subclasses must implement.
   */
  virtual void DoSetDisplayArea(DisplayArea area) = 0;

  /**
   * @brief Platform-specific display mode setting implementation.
   *
   * @param[in] mode The display mode
   * This is a template method hook that subclasses must implement.
   */
  virtual void DoSetDisplayMode(Dali::VideoPlayerPlugin::DisplayMode::Type mode) = 0;

  /**
   * @brief Platform-specific codec type setting implementation.
   *
   * @param[in] type The codec type
   * This is a template method hook that subclasses must implement.
   */
  virtual void DoSetCodecType(Dali::VideoPlayerPlugin::CodecType type) = 0;

  /**
   * @brief Platform-specific texture stream mode initialization.
   *
   * @param[in] nativeImagePtr The native image pointer
   * This is a template method hook that subclasses must implement.
   */
  virtual void DoInitializeTextureStreamMode(Dali::NativeImagePtr nativeImagePtr) = 0;

  /**
   * @brief Platform-specific media player handle getting implementation.
   *
   * @return The media player handle as Any
   * This is a template method hook that subclasses must implement.
   */
  virtual Any DoGetMediaPlayer() = 0;

  /**
   * @brief Hook called after Play() operation completes.
   *
   * Subclasses can override this to perform post-play operations.
   */
  virtual void OnAfterPlay();

  /**
   * @brief Hook called after Pause() operation completes.
   *
   * Subclasses can override this to perform post-pause operations.
   */
  virtual void OnAfterPause();

  /**
   * @brief Hook called after Stop() operation completes.
   *
   * Subclasses can override this to perform post-stop operations.
   */
  virtual void OnAfterStop();

  /**
   * @brief Destroys a platform-specific media packet.
   * @param[in] packet The platform-specific packet handle
   */
  virtual void DestroyMediaPacket(void* packet) {}

  /**
   * @brief Extracts the tbm_surface_h from a platform-specific media packet.
   * @param[in] packet The platform-specific packet handle
   * @return The surface handle as Dali::Any
   */
  virtual Any GetSurfaceFromPacket(void* packet) { return Any(); }

  /**
   * @brief Clears all pending and used packets.
   */
  void ClearPackets();

  // Protected member variables accessible to subclasses
  std::string                                         mUrl;                        ///< The video file path
  Dali::VideoSyncMode                                 mSyncMode;                   ///< Synchronization mode
  Dali::WeakHandle<Dali::Actor>                       mSyncActor;                  ///< Actor for synchronization
  bool                                                mIsMuted;                    ///< Mute state
  float                                               mVolumeLeft;                 ///< Left channel volume
  float                                               mVolumeRight;                ///< Right channel volume
  bool                                                mIsLooping;                  ///< Looping state
  Dali::VideoPlayerPlugin::CodecType                  mCodecType;                  ///< Codec type
  Dali::VideoPlayerPlugin::DisplayMode::Type          mDisplayMode;                ///< Display mode
  Dali::VideoPlayerPlugin::DisplayRotation            mDisplayRotation;            ///< Display rotation
  Dali::VideoPlayerPlugin::VideoPlayerSignalType      mFinishedSignal;             ///< Finished signal
  Dali::NativeImagePtr                                mNativeImagePtr;             ///< Native image for video rendering
  bool                                                mIsAutoRotationEnabled;      ///< Auto rotation enabled state
  bool                                                mIsLetterBoxEnabled;         ///< Letter box enabled state
  float                                               mInterpolationInterval;      ///< Frame interpolation interval in seconds
  bool                                                mUseOffscreenFrameRendering; ///< Offscreen frame rendering enabled state
  Dali::NativeImagePtr                                mPreviousFrameBuffer;        ///< Previous frame buffer for interpolation
  Dali::NativeImagePtr                                mCurrentFrameBuffer;         ///< Current frame buffer for interpolation
  Dali::Mutex                                         mPacketMutex;                ///< Mutex for packet operations

  std::queue<Command>                                 mCommandQueue;               ///< Queue for asynchronous command processing
  Dali::Mutex                                         mCommandMutex;               ///< Mutex for command queue
  std::unique_ptr<Dali::EventThreadCallback>          mEventCallback;              ///< Callback for UI thread updates
  Dali::VideoPlayerPlugin::VideoPlayerEventSignalType mEventSignal;             ///< Centralized event signal

  std::deque<void*>                                   mPacketQueue;                ///< Pending decoded packets
  std::deque<void*>                                   mUsedPackets;                ///< Recently displayed packets (prevent tearing)
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_EXTENSION_VIDEO_PLAYER_BASE_H