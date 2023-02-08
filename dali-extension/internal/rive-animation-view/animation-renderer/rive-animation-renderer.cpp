/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali-extension/internal/rive-animation-view/animation-renderer/rive-animation-renderer.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/file-loader.h>
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/property-array.h>
#include <tbm_surface_internal.h>
#include <time.h>
#include <cmath>
#include <cstring> // for strlen()
#include <rive/file.hpp>
#include <rive/node.hpp>
#include <rive/shapes/paint/color.hpp>
#include <rive/shapes/paint/fill.hpp>
#include <rive/shapes/paint/solid_color.hpp>
#include <rive/shapes/paint/stroke.hpp>

// INTERNAL INCLUDES
#include <dali-extension/internal/rive-animation-view/animation-renderer/rive-animation-renderer-manager.h>

namespace Dali
{
namespace Extension
{
namespace Internal
{
namespace
{
const char* const PIXEL_AREA_UNIFORM_NAME("pixelArea");

const Vector4 FULL_TEXTURE_RECT(0.f, 0.f, 1.f, 1.f);

#if defined(DEBUG_ENABLED)
Debug::Filter* gRiveAnimationLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_RIVE_ANIMATION");
#endif
} // unnamed namespace

RiveAnimationRenderer::RiveAnimationRenderer()
: mUrl(),
  mMutex(),
  mRenderer(),
  mTexture(),
  mRenderedTexture(),
  mPreviousTexture(),
  mTargetSurface(),
  mUploadCompletedSignal(),
  mTbmQueue(NULL),
  mArtboard(nullptr),
  mAnimation(nullptr),
  mStartFrameNumber(0),
  mTotalFrameNumber(0),
  mWidth(0),
  mHeight(0),
  mDefaultWidth(0),
  mDefaultHeight(0),
  mFrameRate(60.0f),
  mResourceReady(false),
  mShaderChanged(false),
  mResourceReadyTriggered(false),
  mRiveTizenAdapter(nullptr)
{
  mRiveTizenAdapter = new RiveTizen();
}

RiveAnimationRenderer::~RiveAnimationRenderer()
{
  Dali::Mutex::ScopedLock lock(mMutex);
  ClearRiveAnimations();

  if(mRiveTizenAdapter)
  {
    delete mRiveTizenAdapter;
  }

  mRiveTizenAdapter = nullptr;

  DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationRenderer::~RiveAnimationRenderer: this = %p\n", this);
}

void RiveAnimationRenderer::ClearRiveAnimations()
{
  mAnimations.clear();
}

void RiveAnimationRenderer::LoadRiveFile(const std::string& filename)
{
  std::streampos        length = 0;
  Dali::Vector<uint8_t> bytes;

  if(!Dali::FileLoader::ReadFile(filename, bytes))
  {
    DALI_LOG_ERROR("Failed to read all of %s", filename.c_str());
    return;
  }

  if(bytes.Size() == 0)
  {
    DALI_LOG_ERROR("Failed to load: empty file %s", filename.c_str());
    return;
  }

  ClearRiveAnimations();
  if(!mRiveTizenAdapter->loadRiveResource(&bytes[0], bytes.Size()))
  {
    DALI_LOG_ERROR("Failed to load resource file %s", filename.c_str());
    return;
  }

  mArtboard = mRiveTizenAdapter->getArtboard();

  for(unsigned int i = 0; i < mArtboard->animationCount(); i++)
  {
    auto               animation = mArtboard->animation(i);
    const std::string& name      = animation->name();
    mAnimations.emplace_back(Animation(mRiveTizenAdapter->createLinearAnimationInstance(i), name, false));
  }

  mAnimation = mArtboard->firstAnimation();
  if(mAnimation)
  {
    mStartFrameNumber = mAnimation->enableWorkArea() ? mAnimation->workStart() : 0;
    mTotalFrameNumber = mAnimation->enableWorkArea() ? mAnimation->workEnd() : mAnimation->duration();
    mTotalFrameNumber -= mStartFrameNumber;
  }
}

bool RiveAnimationRenderer::Load(const std::string& url)
{
  mUrl = url;
  LoadRiveFile(mUrl);
  RiveAnimationRendererManager::Get().AddEventHandler(*this);

  DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationRenderer::Initialize: file [%s] [%p]\n", url.c_str(), this);

  return true;
}

void RiveAnimationRenderer::Finalize()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  RiveAnimationRendererManager::Get().RemoveEventHandler(*this);

  mRenderer.Reset();
  mTexture.Reset();
  mRenderedTexture.Reset();
  mPreviousTexture.Reset();

  mTargetSurface = nullptr;
  mTbmQueue      = nullptr;

  DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationRenderer::Finalize: [%p]\n", this);
}

void RiveAnimationRenderer::SetRenderer(Renderer renderer)
{
  mRenderer      = renderer;
  mShaderChanged = false;

  if(mTargetSurface)
  {
    Dali::Mutex::ScopedLock lock(mMutex);

    if(mResourceReady && mRenderedTexture)
    {
      TextureSet textureSet = renderer.GetTextures();

      textureSet.SetTexture(0, mRenderedTexture);

      mUploadCompletedSignal.Emit();
    }

    SetShader();
  }
}

void RiveAnimationRenderer::SetSize(uint32_t width, uint32_t height)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mWidth == width && mHeight == height)
  {
    DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationRenderer::SetSize: Same size (%d, %d) [%p]\n", mWidth, mHeight, this);
    return;
  }

  mTargetSurface = NativeImageSourceQueue::New(width, height, NativeImageSourceQueue::ColorFormat::RGBA8888);

  mTexture = Texture::New(*mTargetSurface);

  if(mRenderer)
  {
    SetShader();
  }

  mTbmQueue = AnyCast<tbm_surface_queue_h>(mTargetSurface->GetNativeImageSourceQueue());

  mWidth  = width;
  mHeight = height;

  mResourceReady = false;

  // Reset the previous texture to destroy it in the main thread
  mPreviousTexture.Reset();

  DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationRenderer::SetSize: width = %d, height = %d [%p]\n", mWidth, mHeight, this);
}

bool RiveAnimationRenderer::Render(double elapsed)
{
  Dali::Mutex::ScopedLock lock(mMutex);
  if(!mTbmQueue || !mTargetSurface || mAnimations.empty())
  {
    return false;
  }

  int canDequeue = tbm_surface_queue_can_dequeue(mTbmQueue, 0);
  if(!canDequeue)
  {
    // Ignore the previous image which is inserted to the queue.
    mTargetSurface->IgnoreSourceImage();

    // Check again
    canDequeue = tbm_surface_queue_can_dequeue(mTbmQueue, 0);
    if(!canDequeue)
    {
      return false;
    }
  }

  tbm_surface_h tbmSurface;

  if(tbm_surface_queue_dequeue(mTbmQueue, &tbmSurface) != TBM_SURFACE_QUEUE_ERROR_NONE)
  {
    DALI_LOG_ERROR("Failed to dequeue a tbm_surface [%p]\n", this);
    return false;
  }

  tbm_surface_info_s info;
  int                ret = tbm_surface_map(tbmSurface, TBM_OPTION_WRITE, &info);
  if(ret != TBM_SURFACE_ERROR_NONE)
  {
    DALI_LOG_ERROR("RiveAnimationRenderer::Render: tbm_surface_map is failed! [%d] [%p]\n", ret, this);
    tbm_surface_queue_cancel_dequeue(mTbmQueue, tbmSurface);
    return false;
  }

  unsigned char* buffer = info.planes[0].ptr;
  if(!buffer)
  {
    DALI_LOG_ERROR("RiveAnimationRenderer::Render: tbm buffer pointer is null! [%p]\n", this);
    tbm_surface_unmap(tbmSurface);
    tbm_surface_queue_cancel_dequeue(mTbmQueue, tbmSurface);
    return false;
  }

  tbm_surface_internal_ref(tbmSurface);

  mRiveTizenAdapter->createCanvas(buffer, info.width, info.height, info.planes[0].stride / 4);

  // Render Rive animation by elapsed time
  for(auto& animation : mAnimations)
  {
    if(animation.instance)
    {
      if(animation.enable)
      {
        mRiveTizenAdapter->animationAdvanceApply(animation.instance.get(), elapsed);
      }
      else if(animation.elapsed >= 0.0f)
      {
        mRiveTizenAdapter->animationApply(animation.instance.get(), animation.elapsed);
      }
    }
  }

  if(!mRiveTizenAdapter->render(elapsed, info.width, info.height))
  {
    tbm_surface_unmap(tbmSurface);
    tbm_surface_queue_cancel_dequeue(mTbmQueue, tbmSurface);
    return false;
  }

  tbm_surface_unmap(tbmSurface);

  tbm_surface_queue_enqueue(mTbmQueue, tbmSurface);

  if(!mResourceReady)
  {
    mPreviousTexture        = mRenderedTexture; // It is used to destroy the object in the main thread.
    mRenderedTexture        = mTexture;
    mResourceReady          = true;
    mResourceReadyTriggered = true;

    RiveAnimationRendererManager::Get().TriggerEvent(*this);

    DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationRenderer::Render: Resource ready [%p]\n", this);
  }

  return true;
}

uint32_t RiveAnimationRenderer::GetTotalFrameNumber() const
{
  return mTotalFrameNumber;
}

float RiveAnimationRenderer::GetFrameRate() const
{
  return mFrameRate;
}

void RiveAnimationRenderer::GetDefaultSize(uint32_t& width, uint32_t& height) const
{
  width  = mDefaultWidth;
  height = mDefaultHeight;
}

void RiveAnimationRenderer::EnableAnimation(const std::string& animationName, bool enable)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  for(unsigned int i = 0; i < mAnimations.size(); i++)
  {
    if(mAnimations[i].name == animationName)
    {
      if(mAnimations[i].instance)
      {
        mAnimations[i].instance.reset(mRiveTizenAdapter->createLinearAnimationInstance(i));
      }
      mAnimations[i].enable = enable;
      return;
    }
  }
}

void RiveAnimationRenderer::SetAnimationElapsedTime(const std::string& animationName, float elapsed)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  for(auto& animation : mAnimations)
  {
    if(animation.name == animationName)
    {
      animation.elapsed = elapsed;
      return;
    }
  }
}

void RiveAnimationRenderer::SetShapeFillColor(const std::string& fillName, Vector4 color)
{
  Dali::Mutex::ScopedLock lock(mMutex);
  mRiveTizenAdapter->setShapeFillColor(fillName, color.a, color.r, color.g, color.b);
}

void RiveAnimationRenderer::SetShapeStrokeColor(const std::string& strokeName, Vector4 color)
{
  Dali::Mutex::ScopedLock lock(mMutex);
  mRiveTizenAdapter->setShapeStrokeColor(strokeName, color.a, color.r, color.g, color.b);
}

void RiveAnimationRenderer::SetNodeOpacity(const std::string& nodeName, float opacity)
{
  Dali::Mutex::ScopedLock lock(mMutex);
  mRiveTizenAdapter->setNodeOpacity(nodeName, opacity);
}

void RiveAnimationRenderer::SetNodeScale(const std::string& nodeName, Vector2 scale)
{
  Dali::Mutex::ScopedLock lock(mMutex);
  mRiveTizenAdapter->setNodeScale(nodeName, scale.x, scale.y);
}

void RiveAnimationRenderer::SetNodeRotation(const std::string& nodeName, Degree degree)
{
  Dali::Mutex::ScopedLock lock(mMutex);
  mRiveTizenAdapter->setNodeRotation(nodeName, degree.degree);
}

void RiveAnimationRenderer::SetNodePosition(const std::string& nodeName, Vector2 position)
{
  Dali::Mutex::ScopedLock lock(mMutex);
  mRiveTizenAdapter->setNodePosition(nodeName, position.x, position.y);
}

void RiveAnimationRenderer::PointerMove(float x, float y)
{
#if !defined(OS_TIZEN_TV)
  Dali::Mutex::ScopedLock lock(mMutex);
  mRiveTizenAdapter->pointerMove(x, y);
#endif
}

void RiveAnimationRenderer::PointerDown(float x, float y)
{
#if !defined(OS_TIZEN_TV)
  Dali::Mutex::ScopedLock lock(mMutex);
  mRiveTizenAdapter->pointerDown(x, y);
#endif
}

void RiveAnimationRenderer::PointerUp(float x, float y)
{
#if !defined(OS_TIZEN_TV)
  Dali::Mutex::ScopedLock lock(mMutex);
  mRiveTizenAdapter->pointerUp(x, y);
#endif
}

bool RiveAnimationRenderer::SetNumberState(const std::string& stateMachineName, const std::string& inputName, float value)
{
#if !defined(OS_TIZEN_TV)
  Dali::Mutex::ScopedLock lock(mMutex);
  return mRiveTizenAdapter->setNumberState(stateMachineName, inputName, value);
#else
  return false;
#endif
}

bool RiveAnimationRenderer::SetBooleanState(const std::string& stateMachineName, const std::string& inputName, bool value)
{
#if !defined(OS_TIZEN_TV)
  Dali::Mutex::ScopedLock lock(mMutex);
  return mRiveTizenAdapter->setBooleanState(stateMachineName, inputName, value);
#else
  return false;
#endif
}

bool RiveAnimationRenderer::FireState(const std::string& stateMachineName, const std::string& inputName)
{
#if !defined(OS_TIZEN_TV)
  Dali::Mutex::ScopedLock lock(mMutex);
  return mRiveTizenAdapter->fireState(stateMachineName, inputName);
#else
  return false;
#endif
}

void RiveAnimationRenderer::IgnoreRenderedFrame()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mTargetSurface)
  {
    mTargetSurface->IgnoreSourceImage();
  }
}

RiveAnimationRenderer::UploadCompletedSignalType& RiveAnimationRenderer::UploadCompletedSignal()
{
  return mUploadCompletedSignal;
}

void RiveAnimationRenderer::NotifyEvent()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mResourceReadyTriggered)
  {
    DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationRenderer::NotifyEvent: Set Texture [%p]\n", this);

    // Set texture
    if(mRenderer && mRenderedTexture)
    {
      TextureSet textureSet = mRenderer.GetTextures();
      textureSet.SetTexture(0, mRenderedTexture);
    }

    mResourceReadyTriggered = false;

    mUploadCompletedSignal.Emit();
  }

  mPreviousTexture.Reset();
}

void RiveAnimationRenderer::SetShader()
{
  if(mShaderChanged)
  {
    return;
  }

  Shader shader = mRenderer.GetShader();

  std::string fragmentShader;
  std::string vertexShader;

  // Get the current fragment shader source
  Property::Value program = shader.GetProperty(Shader::Property::PROGRAM);
  Property::Map*  map     = program.GetMap();
  if(map)
  {
    Property::Value* fragment = map->Find("fragment");
    if(fragment)
    {
      fragmentShader = fragment->Get<std::string>();
    }

    Property::Value* vertex = map->Find("vertex");
    if(vertex)
    {
      vertexShader = vertex->Get<std::string>();
    }
  }

  // Get custom fragment shader prefix
  mTargetSurface->ApplyNativeFragmentShader(fragmentShader);

  // Set the modified shader again
  Shader newShader = Shader::New(vertexShader, fragmentShader);
  newShader.RegisterProperty(PIXEL_AREA_UNIFORM_NAME, FULL_TEXTURE_RECT);

  mRenderer.SetShader(newShader);

  mShaderChanged = true;
}

} // namespace Internal

} // namespace Extension

} // namespace Dali
