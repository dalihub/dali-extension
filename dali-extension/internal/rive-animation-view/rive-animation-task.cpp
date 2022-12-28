/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali-extension/internal/rive-animation-view/rive-animation-task.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/math/math-utils.h>
#include <dali/public-api/object/property-array.h>

// INTERNAL INCLUDES
#include <dali-extension/internal/rive-animation-view/rive-animation-manager.h>
#include <dali-extension/internal/rive-animation-view/rive-animation-thread.h>

namespace Dali
{
namespace Extension
{
namespace Internal
{
namespace
{
constexpr auto MICROSECONDS_PER_SECOND(1e+6);

#if defined(DEBUG_ENABLED)
Debug::Filter* gRiveAnimationLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_RIVE_ANIMATION");
#endif
} // unnamed namespace

RiveAnimationTask::RiveAnimationTask()
: mUrl(),
  mVectorRenderer(new RiveAnimationRenderer()),
  mAnimationData(),
  mRiveAnimationThread(RiveAnimationManager::GetInstance().GetRiveAnimationThread()),
  mConditionalWait(),
  mAnimationFinishedTrigger(),
  mPlayState(PlayState::STOPPED),
  mNextFrameStartTime(),
  mFrameDurationMicroSeconds(MICROSECONDS_PER_SECOND / 60.0f),
  mFrameRate(60.0f),
  mCurrentFrame(0),
  mTotalFrame(0),
  mStartFrame(0),
  mEndFrame(0),
  mDroppedFrames(0),
  mWidth(0),
  mHeight(0),
  mAnimationDataIndex(0),
  mUpdateFrameNumber(false),
  mNeedAnimationFinishedTrigger(true),
  mAnimationDataUpdated(false),
  mDestroyTask(false)
{
}

RiveAnimationTask::~RiveAnimationTask()
{
  DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationTask::~RiveAnimationTask: destructor [%p]\n", this);
}

void RiveAnimationTask::Finalize()
{
  ConditionalWait::ScopedLock lock(mConditionalWait);

  // Release some objects in the main thread
  if(mAnimationFinishedTrigger)
  {
    mAnimationFinishedTrigger.reset();
  }

  mVectorRenderer->Finalize();

  mDestroyTask = true;
}

bool RiveAnimationTask::Load(const std::string& url)
{
  mUrl = url;

  if(!mVectorRenderer->Load(mUrl))
  {
    DALI_LOG_ERROR("RiveAnimationTask::Load: Load failed [%s]\n", mUrl.c_str());
    return false;
  }

  mTotalFrame = mVectorRenderer->GetTotalFrameNumber();

  mEndFrame = mTotalFrame - 1;

  mFrameRate                 = mVectorRenderer->GetFrameRate();
  mFrameDurationMicroSeconds = MICROSECONDS_PER_SECOND / mFrameRate;

  uint32_t width, height;
  mVectorRenderer->GetDefaultSize(width, height);

  SetSize(width, height);

  DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationTask::Load: file = %s [%d frames, %f fps] [%p]\n", mUrl.c_str(), mTotalFrame, mFrameRate, this);

  return true;
}

void RiveAnimationTask::SetRenderer(Renderer renderer)
{
  ConditionalWait::ScopedLock lock(mConditionalWait);

  mVectorRenderer->SetRenderer(renderer);

  DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationTask::SetRenderer [%p]\n", this);
}

void RiveAnimationTask::SetAnimationData(const AnimationData& data)
{
  ConditionalWait::ScopedLock lock(mConditionalWait);

  DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationTask::SetAnimationData [%p]\n", this);

  uint32_t index = mAnimationDataIndex == 0 ? 1 : 0; // Use the other buffer

  mAnimationData[index] = data;
  mAnimationDataUpdated = true;

  if(data.resendFlag & RiveAnimationTask::RESEND_SIZE)
  {
    // The size should be changed in the main thread.
    SetSize(data.width, data.height);
  }

  mRiveAnimationThread.AddTask(this);
}

void RiveAnimationTask::SetSize(uint32_t width, uint32_t height)
{
  if(mWidth != width || mHeight != height)
  {
    mVectorRenderer->SetSize(width, height);

    mWidth  = width;
    mHeight = height;

    DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationTask::SetSize: width = %d, height = %d [%p]\n", width, height, this);
  }
}

void RiveAnimationTask::PlayAnimation()
{
  if(mPlayState != PlayState::PLAYING)
  {
    mNeedAnimationFinishedTrigger = true;
    mUpdateFrameNumber            = false;
    mPlayState                    = PlayState::PLAYING;

    DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationTask::PlayAnimation: Play [%p]\n", this);
  }
}

void RiveAnimationTask::StopAnimation()
{
  if(mPlayState != PlayState::STOPPING)
  {
    mNeedAnimationFinishedTrigger = false;
    mPlayState                    = PlayState::STOPPING;

    DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationTask::StopAnimation: Stop [%p]\n", this);
  }
}

void RiveAnimationTask::PauseAnimation()
{
  if(mPlayState == PlayState::PLAYING)
  {
    mPlayState = PlayState::PAUSED;

    DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationTask::PauseAnimation: Pause [%p]\n", this);
  }
}

void RiveAnimationTask::EnableAnimation(const std::string& animationName, bool enable)
{
  mVectorRenderer->EnableAnimation(animationName, enable);
}

void RiveAnimationTask::SetAnimationElapsedTime(const std::string& animationName, float elapsed)
{
  mVectorRenderer->SetAnimationElapsedTime(animationName, elapsed);
}

void RiveAnimationTask::SetShapeFillColor(const std::string& fillName, Vector4 color)
{
  mVectorRenderer->SetShapeFillColor(fillName, color);
}

void RiveAnimationTask::SetShapeStrokeColor(const std::string& strokeName, Vector4 color)
{
  mVectorRenderer->SetShapeStrokeColor(strokeName, color);
}

void RiveAnimationTask::SetNodeOpacity(const std::string& nodeName, float opacity)
{
  mVectorRenderer->SetNodeOpacity(nodeName, opacity);
}

void RiveAnimationTask::SetNodeScale(const std::string& nodeName, Vector2 scale)
{
  mVectorRenderer->SetNodeScale(nodeName, scale);
}

void RiveAnimationTask::SetNodeRotation(const std::string& nodeName, Degree degree)
{
  mVectorRenderer->SetNodeRotation(nodeName, degree);
}

void RiveAnimationTask::SetNodePosition(const std::string& nodeName, Vector2 position)
{
  mVectorRenderer->SetNodePosition(nodeName, position);
}

void RiveAnimationTask::SetAnimationFinishedCallback(EventThreadCallback* callback)
{
  ConditionalWait::ScopedLock lock(mConditionalWait);
  if(callback)
  {
    mAnimationFinishedTrigger = std::unique_ptr<EventThreadCallback>(callback);
  }
}


void RiveAnimationTask::GetDefaultSize(uint32_t& width, uint32_t& height) const
{
  mVectorRenderer->GetDefaultSize(width, height);
}

RiveAnimationTask::UploadCompletedSignalType& RiveAnimationTask::UploadCompletedSignal()
{
  return mVectorRenderer->UploadCompletedSignal();
}

bool RiveAnimationTask::Rasterize()
{
  bool     stopped = false;
  uint32_t currentFrame;

  {
    ConditionalWait::ScopedLock lock(mConditionalWait);
    if(mDestroyTask)
    {
      // The task will be destroyed. We don't need rasterization.
      return false;
    }
  }

  ApplyAnimationData();

  if(mPlayState == PlayState::PLAYING && mUpdateFrameNumber)
  {
    mCurrentFrame = mCurrentFrame + mDroppedFrames + 1;
    Dali::ClampInPlace(mCurrentFrame, mStartFrame, mEndFrame);
  }

  currentFrame = mCurrentFrame;

  mUpdateFrameNumber = true;

  if(mPlayState == PlayState::STOPPING)
  {
    currentFrame  = mCurrentFrame;
    stopped       = true;
  }
  else if(mPlayState == PlayState::PLAYING)
  {
    bool animationFinished = false;

    if(currentFrame >= mEndFrame) // last frame
    {
      //FIXME: Rive animation can have more than two animations,
      //so This is not suitable for rive animation.
      //The animation finish policy should be changed.
      //animationFinished = true; // end of animation
      mCurrentFrame = mStartFrame;
    }

    if(animationFinished)
    {
      mPlayState = PlayState::STOPPING;
    }
  }

  // Rasterize
  bool renderSuccess = false;
  if(mVectorRenderer)
  {
    auto currentTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = currentTime - mEndTime;
    mEndTime = currentTime;

    renderSuccess = mVectorRenderer->Render(elapsed.count());
    if(!renderSuccess)
    {
      DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationTask::Rasterize: Rendering failed. Try again later.[%d] [%p]\n", currentFrame, this);
      mUpdateFrameNumber = false;
    }
  }

  if(stopped && renderSuccess)
  {
    mPlayState   = PlayState::STOPPED;

    // Animation is finished
    {
      ConditionalWait::ScopedLock lock(mConditionalWait);
      if(mNeedAnimationFinishedTrigger && mAnimationFinishedTrigger)
      {
        mAnimationFinishedTrigger->Trigger();
      }
    }

    DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationTask::Rasterize: Animation is finished [current = %d] [%p]\n", currentFrame, this);
  }

  bool keepAnimation = true;
  if(mPlayState == PlayState::PAUSED || mPlayState == PlayState::STOPPED)
  {
    keepAnimation = false;
  }

  return keepAnimation;
}

RiveAnimationTask::TimePoint RiveAnimationTask::CalculateNextFrameTime(bool renderNow)
{
  // std::chrono::time_point template has second parameter duration which defaults to the std::chrono::system_clock supported
  // duration. In some C++11 implementations it is a milliseconds duration, so it fails to compile unless mNextFrameStartTime
  // is casted to use the default duration.
  mNextFrameStartTime = std::chrono::time_point_cast<TimePoint::duration>(mNextFrameStartTime + std::chrono::microseconds(mFrameDurationMicroSeconds));
  auto current        = std::chrono::system_clock::now();
  if(renderNow)
  {
    mNextFrameStartTime = current;
    mDroppedFrames      = 0;
  }
  else if(mNextFrameStartTime < current)
  {
    uint32_t droppedFrames = 0;

    while(current > std::chrono::time_point_cast<TimePoint::duration>(mNextFrameStartTime + std::chrono::microseconds(mFrameDurationMicroSeconds)))
    {
      droppedFrames++;
      mNextFrameStartTime = std::chrono::time_point_cast<TimePoint::duration>(mNextFrameStartTime + std::chrono::microseconds(mFrameDurationMicroSeconds));
    }

    mNextFrameStartTime = current;
    mDroppedFrames      = droppedFrames;
  }

  return mNextFrameStartTime;
}

RiveAnimationTask::TimePoint RiveAnimationTask::GetNextFrameTime()
{
  return mNextFrameStartTime;
}

void RiveAnimationTask::ApplyAnimationData()
{
  uint32_t index;

  {
    ConditionalWait::ScopedLock lock(mConditionalWait);

    if(!mAnimationDataUpdated || mAnimationData[mAnimationDataIndex].resendFlag != 0)
    {
      // Data is not updated or the previous data is not applied yet.
      return;
    }

    mAnimationDataIndex   = mAnimationDataIndex == 0 ? 1 : 0; // Swap index
    mAnimationDataUpdated = false;

    index = mAnimationDataIndex;
  }

  if(mAnimationData[index].resendFlag & RiveAnimationTask::RESEND_PLAY_STATE)
  {
    if(mAnimationData[index].playState == Extension::RiveAnimationView::PlayState::PLAYING)
    {
      PlayAnimation();
    }
    else if(mAnimationData[index].playState == Extension::RiveAnimationView::PlayState::PAUSED)
    {
      PauseAnimation();
    }
    else if(mAnimationData[index].playState == Extension::RiveAnimationView::PlayState::STOPPED)
    {
      StopAnimation();
    }
  }

  if(mAnimationData[index].resendFlag & RiveAnimationTask::RESEND_ENABLE_ANIMATION)
  {
    for(auto& animation : mAnimationData[index].animations)
    {
      EnableAnimation(animation.first, animation.second);
    }
  }

  if(mAnimationData[index].resendFlag & RiveAnimationTask::RESEND_ANIMATION_ELAPSED_TIME)
  {
    for(auto& elapsedTime : mAnimationData[index].elapsedTimes)
    {
      SetAnimationElapsedTime(elapsedTime.first, elapsedTime.second);
    }
  }

  if(mAnimationData[index].resendFlag & RiveAnimationTask::RESEND_FILL_COLOR)
  {
    for(auto& fillColor : mAnimationData[index].fillColors)
    {
      SetShapeFillColor(fillColor.first, fillColor.second);
    }
  }

  if(mAnimationData[index].resendFlag & RiveAnimationTask::RESEND_STROKE_COLOR)
  {
    for(auto& strokeColor : mAnimationData[index].strokeColors)
    {
      SetShapeStrokeColor(strokeColor.first, strokeColor.second);
    }
  }

  if(mAnimationData[index].resendFlag & RiveAnimationTask::RESEND_OPACITY)
  {
    for(auto& opacity : mAnimationData[index].opacities)
    {
      SetNodeOpacity(opacity.first, opacity.second);
    }
  }

  if(mAnimationData[index].resendFlag & RiveAnimationTask::RESEND_SCALE)
  {
    for(auto& scale : mAnimationData[index].scales)
    {
      SetNodeScale(scale.first, scale.second);
    }
  }

  if(mAnimationData[index].resendFlag & RiveAnimationTask::RESEND_ROTATION)
  {
    for(auto& rotation : mAnimationData[index].rotations)
    {
      SetNodeRotation(rotation.first, rotation.second);
    }
  }

  if(mAnimationData[index].resendFlag & RiveAnimationTask::RESEND_POSITION)
  {
    for(auto& position : mAnimationData[index].positions)
    {
      SetNodePosition(position.first, position.second);
    }
  }

  mAnimationData[index].animations.clear();
  mAnimationData[index].elapsedTimes.clear();
  mAnimationData[index].fillColors.clear();
  mAnimationData[index].strokeColors.clear();
  mAnimationData[index].opacities.clear();
  mAnimationData[index].scales.clear();
  mAnimationData[index].rotations.clear();
  mAnimationData[index].positions.clear();
  mAnimationData[index].resendFlag = 0;
}

} // namespace Internal

} // namespace Toolkit

} // namespace Dali
