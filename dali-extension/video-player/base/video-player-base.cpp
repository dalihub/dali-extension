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
#include <dali/integration-api/debug.h>
#include <dali/devel-api/common/stage.h>

namespace Dali
{
namespace Plugin
{

VideoPlayerBase::VideoPlayerBase(Dali::VideoSyncMode syncMode, Dali::Actor syncActor)
: mSyncMode(syncMode),
  mSyncActor(syncActor),
  mIsMuted(false),
  mVolumeLeft(1.0f),
  mVolumeRight(1.0f),
  mIsLooping(false),
  mCodecType(Dali::VideoPlayerPlugin::CodecType::DEFAULT),
  mDisplayMode(Dali::VideoPlayerPlugin::DisplayMode::FULL_SCREEN),
  mDisplayRotation(Dali::VideoPlayerPlugin::DisplayRotation::ROTATION_NONE),
  mIsAutoRotationEnabled(false),
  mIsLetterBoxEnabled(false),
  mInterpolationInterval(0.0f),
  mUseOffscreenFrameRendering(false),
  mPacketMutex(),
  mEventCallback(nullptr)
{
}

VideoPlayerBase::~VideoPlayerBase()
{
}

void VideoPlayerBase::SetUrl(const std::string& url)
{
  mUrl = url;
  PostCommand([this, url]() {
    DoSetUrl(url);
  });
}

std::string VideoPlayerBase::GetUrl()
{
  return mUrl;
}

void VideoPlayerBase::SetLooping(bool looping)
{
  mIsLooping = looping;
  PostCommand([this, looping]() {
    DoSetLooping(looping);
  });
}

bool VideoPlayerBase::IsLooping()
{
  return mIsLooping;
}

void VideoPlayerBase::Play()
{
  PostCommand([this]() {
    DoPlay();
    OnAfterPlay();
  });
}

void VideoPlayerBase::Pause()
{
  PostCommand([this]() {
    DoPause();
    OnAfterPause();
  });
}

void VideoPlayerBase::Stop()
{
  PostCommand([this]() {
    DoStop();
    OnAfterStop();
  });
}

void VideoPlayerBase::SetMute(bool mute)
{
  mIsMuted = mute;
  PostCommand([this, mute]() {
    DoSetMute(mute);
  });
}

bool VideoPlayerBase::IsMuted()
{
  return mIsMuted;
}

void VideoPlayerBase::SetVolume(float left, float right)
{
  mVolumeLeft = left;
  mVolumeRight = right;
  PostCommand([this, left, right]() {
    DoSetVolume(left, right);
  });
}

void VideoPlayerBase::GetVolume(float& left, float& right)
{
  left = mVolumeLeft;
  right = mVolumeRight;
}

void VideoPlayerBase::SetRenderingTarget(Any target)
{
  if(target.Empty())
  {
    DALI_LOG_ERROR("VideoPlayerBase::SetRenderingTarget: Empty target\n");
    return;
  }

  Dali::NativeImagePtr nativeImage = Dali::AnyCast<Dali::NativeImagePtr>(target);
  if(nativeImage)
  {
    mNativeImagePtr = nativeImage;
    DoInitializeTextureStreamMode(nativeImage);
  }
}

void VideoPlayerBase::SetPlayPosition(int millisecond)
{
  PostCommand([this, millisecond]() {
    DoSetPlayPosition(millisecond);
  });
}

int VideoPlayerBase::GetPlayPosition()
{
  return DoGetPlayPosition();
}

void VideoPlayerBase::SetDisplayArea(DisplayArea area)
{
  PostCommand([this, area]() {
    DoSetDisplayArea(area);
  });
}

void VideoPlayerBase::SetDisplayRotation(Dali::VideoPlayerPlugin::DisplayRotation rotation)
{
  mDisplayRotation = rotation;
  PostCommand([this, rotation]() {
    DoSetDisplayRotation(rotation);
  });
}

Dali::VideoPlayerPlugin::DisplayRotation VideoPlayerBase::GetDisplayRotation()
{
  return DoGetDisplayRotation();
}

Dali::VideoPlayerPlugin::VideoPlayerSignalType& VideoPlayerBase::FinishedSignal()
{
  return mFinishedSignal;
}

Dali::VideoPlayerPlugin::VideoPlayerEventSignalType& VideoPlayerBase::EventSignal()
{
  return mEventSignal;
}

void VideoPlayerBase::SetAutoRotationEnabled(bool enable)
{
  mIsAutoRotationEnabled = enable;
}

bool VideoPlayerBase::IsAutoRotationEnabled() const
{
  return mIsAutoRotationEnabled;
}

void VideoPlayerBase::SetLetterBoxEnabled(bool enable)
{
  mIsLetterBoxEnabled = enable;
}

bool VideoPlayerBase::IsLetterBoxEnabled() const
{
  return mIsLetterBoxEnabled;
}

void VideoPlayerBase::SetFrameInterpolationInterval(float intervalSeconds)
{
  mInterpolationInterval = intervalSeconds;
}

void VideoPlayerBase::EnableOffscreenFrameRendering(bool useOffScreenFrame, Dali::NativeImagePtr previousFrameBufferNativeImagePtr, Dali::NativeImagePtr currentFrameBufferNativeImagePtr)
{
  mUseOffscreenFrameRendering = useOffScreenFrame;
  mPreviousFrameBuffer = previousFrameBufferNativeImagePtr;
  mCurrentFrameBuffer = currentFrameBufferNativeImagePtr;
}

void VideoPlayerBase::SetVideoFrameBuffer(Dali::NativeImagePtr source)
{
  mNativeImagePtr = source;
}

void VideoPlayerBase::Forward(int millisecond)
{
  int currentPosition = GetPlayPosition();
  int newPosition = currentPosition + millisecond;
  SetPlayPosition(newPosition);
}

void VideoPlayerBase::Backward(int millisecond)
{
  int currentPosition = GetPlayPosition();
  int newPosition = currentPosition - millisecond;
  if(newPosition < 0)
  {
    newPosition = 0;
  }
  SetPlayPosition(newPosition);
}

bool VideoPlayerBase::IsVideoTextureSupported()
{
  return false;
}

void VideoPlayerBase::SetCodecType(Dali::VideoPlayerPlugin::CodecType type)
{
  mCodecType = type;
  PostCommand([this, type]() {
    DoSetCodecType(type);
  });
}

Dali::VideoPlayerPlugin::CodecType VideoPlayerBase::GetCodecType() const
{
  return mCodecType;
}

void VideoPlayerBase::SetDisplayMode(Dali::VideoPlayerPlugin::DisplayMode::Type mode)
{
  mDisplayMode = mode;
  PostCommand([this, mode]() {
    DoSetDisplayMode(mode);
  });
}

Dali::VideoPlayerPlugin::DisplayMode::Type VideoPlayerBase::GetDisplayMode() const
{
  return mDisplayMode;
}

Any VideoPlayerBase::GetMediaPlayer()
{
  return DoGetMediaPlayer();
}

void VideoPlayerBase::StartSynchronization()
{
}

void VideoPlayerBase::FinishSynchronization()
{
}

void VideoPlayerBase::RaiseAbove(Any videoSurface)
{
}

void VideoPlayerBase::LowerBelow(Any videoSurface)
{
}

void VideoPlayerBase::RaiseToTop()
{
}

void VideoPlayerBase::LowerToBottom()
{
}

Any VideoPlayerBase::GetVideoPlayerSurface()
{
  return Any();
}

void VideoPlayerBase::SceneConnection()
{
}

void VideoPlayerBase::SceneDisconnection()
{
}

bool VideoPlayerBase::IsPlayerReady()
{
  return true; // Default to always ready unless subclasses override
}

void VideoPlayerBase::PostCommand(Command command)
{
  {
    Dali::Mutex::ScopedLock lock(mCommandMutex);
    mCommandQueue.push(command);
  }

  // Try to process immediately if player is ready
  if (IsPlayerReady())
  {
    ProcessCommandQueue();
  }
}

void VideoPlayerBase::ProcessCommandQueue()
{
  while (true)
  {
    Command command;
    {
      Dali::Mutex::ScopedLock lock(mCommandMutex);
      if (mCommandQueue.empty() || !IsPlayerReady())
      {
        break;
      }
      command = mCommandQueue.front();
      mCommandQueue.pop();
    }

    // Execute command outside the lock to avoid deadlocks
    if (command)
    {
      command();
    }
  }
}

// Template Method Pattern: Hooks that can be overridden by subclasses
void VideoPlayerBase::OnAfterPlay()
{
}

void VideoPlayerBase::OnAfterPause()
{
}

void VideoPlayerBase::OnAfterStop()
{
}

void VideoPlayerBase::InitializeUiUpdateCallback()
{
  if (!mEventCallback)
  {
    mEventCallback = std::unique_ptr<Dali::EventThreadCallback>(
      new Dali::EventThreadCallback(MakeCallback(this, &VideoPlayerBase::DoUpdateUi)));
  }
}

void VideoPlayerBase::TriggerUiUpdate()
{
  if (mEventCallback)
  {
    mEventCallback->Trigger();
  }
}

void VideoPlayerBase::PushPacket(void* packet)
{
  Dali::Mutex::ScopedLock lock(mPacketMutex);
  mPacketQueue.push_back(packet);
}

void VideoPlayerBase::ClearPackets()
{
  Dali::Mutex::ScopedLock lock(mPacketMutex);
  for(void* packet : mPacketQueue)
  {
    DestroyMediaPacket(packet);
  }
  mPacketQueue.clear();

  for(void* packet : mUsedPackets)
  {
    DestroyMediaPacket(packet);
  }
  mUsedPackets.clear();
}

void VideoPlayerBase::DoUpdateUi()
{
  Dali::Mutex::ScopedLock lock(mPacketMutex);

  if(mPacketQueue.empty())
  {
    return;
  }

  void* nextPacket = mPacketQueue.front();
  mPacketQueue.pop_front();

  if(!nextPacket)
  {
    return;
  }

  Any surface = GetSurfaceFromPacket(nextPacket);
  if(!surface.Empty() && mNativeImagePtr)
  {
    mNativeImagePtr->SetSource(surface);
    Dali::Stage::GetCurrent().KeepRendering(0.0f);
  }
  else if (surface.Empty())
  {
    DestroyMediaPacket(nextPacket);
    return; // Failed to extract surface
  }

  // Store in used packets to keep it alive (required by Wayland/TBM to prevent tearing)
  mUsedPackets.push_back(nextPacket);

  // Keep a maximum of 3 used packets
  const size_t MAX_USED_PACKETS = 3;
  while(mUsedPackets.size() > MAX_USED_PACKETS)
  {
    void* oldPacket = mUsedPackets.front();
    mUsedPackets.pop_front();
    DestroyMediaPacket(oldPacket);
  }
}

} // namespace Plugin
} // namespace Dali