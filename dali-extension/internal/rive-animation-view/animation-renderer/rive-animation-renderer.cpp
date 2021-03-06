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
#include <rive/node.hpp>
#include <rive/file.hpp>
#include <rive/tvg_renderer.hpp>
#include <rive/shapes/paint/fill.hpp>
#include <rive/shapes/paint/stroke.hpp>
#include <rive/shapes/paint/color.hpp>
#include <rive/shapes/paint/solid_color.hpp>

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
const char* const DEFAULT_SAMPLER_TYPENAME("sampler2D");
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
  mSwCanvas(nullptr),
  mFile(nullptr),
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
  mResourceReadyTriggered(false)
{
  tvg::Initializer::init(tvg::CanvasEngine::Sw, 0);
}

RiveAnimationRenderer::~RiveAnimationRenderer()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  ClearRiveAnimations();

  if(mFile)
  {
    delete mFile;
  }

  mSwCanvas->clear();
  tvg::Initializer::term(tvg::CanvasEngine::Sw);

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

  auto reader = rive::BinaryReader(&bytes[0], bytes.Size());

  if(mFile)
  {
    delete mFile;
  }

  auto result = rive::File::import(reader, &mFile);
  if(result != rive::ImportResult::success)
  {
    DALI_LOG_ERROR("Failed to import %s", filename.c_str());
    return;
  }

  mArtboard = mFile->artboard();
  mArtboard->advance(0.0f);

  ClearRiveAnimations();

  for(unsigned int i = 0; i < mArtboard->animationCount(); i++)
  {
    auto animation = mArtboard->animation(i);
    const std::string& name = animation->name();
    mAnimations.emplace_back(Animation(new rive::LinearAnimationInstance(animation), name, false));
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

  mTargetSurface = NativeImageSourceQueue::New(width, height, NativeImageSourceQueue::COLOR_DEPTH_DEFAULT);

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

bool RiveAnimationRenderer::Render(uint32_t frameNumber)
{
  Dali::Mutex::ScopedLock lock(mMutex);
  if(!mTbmQueue || !mTargetSurface || !mArtboard || mAnimations.empty())
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

  if(!mSwCanvas)
  {
    mSwCanvas = tvg::SwCanvas::gen();
    mSwCanvas->mempool(tvg::SwCanvas::MempoolPolicy::Individual);

  }
  mSwCanvas->clear();
  mSwCanvas->target((uint32_t*)buffer, info.planes[0].stride / 4, info.width, info.height, tvg::SwCanvas::ARGB8888);

  // Render Rive Frame
  frameNumber    = mStartFrameNumber + frameNumber;
  double elapsed = (float)frameNumber / 60.0f;

  for (auto& animation : mAnimations)
  {
    if(animation.enable)
    {
      animation.instance->time(elapsed);
      animation.instance->advance(0.0);
      animation.instance->apply(mArtboard);
    }
  }
  mArtboard->advance(0.0);

  rive::TvgRenderer renderer(mSwCanvas.get());
  renderer.save();
  renderer.align(rive::Fit::contain,
                 rive::Alignment::center,
                 rive::AABB(0, 0, info.width, info.height),
                 mArtboard->bounds());
  mArtboard->draw(&renderer);
  renderer.restore();

  if(mSwCanvas->draw() == tvg::Result::Success)
  {
    mSwCanvas->sync();
  }
  else
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

    DALI_LOG_INFO(gRiveAnimationLogFilter, Debug::Verbose, "RiveAnimationRenderer::Render: Resource ready [current = %d] [%p]\n", frameNumber, this);
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

  for(auto& animation :  mAnimations)
  {
    if(animation.name == animationName)
    {
      animation.enable = enable;
      return;
    }
  }
}

void RiveAnimationRenderer::SetShapeFillColor(const std::string& fillName, Vector4 color)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  auto colorInstance = mArtboard->find<rive::Fill>(fillName.c_str());
  if(!colorInstance)
  {
    return;
  }
  colorInstance->paint()->as<rive::SolidColor>()->colorValue(rive::colorARGB(color.a, color.r, color.g, color.b));
}

void RiveAnimationRenderer::SetShapeStrokeColor(const std::string& strokeName, Vector4 color)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  auto colorInstance = mArtboard->find<rive::Stroke>(strokeName.c_str());
  if(!colorInstance)
  {
    return;
  }
  colorInstance->paint()->as<rive::SolidColor>()->colorValue(rive::colorARGB(color.a, color.r, color.g, color.b));
}

void RiveAnimationRenderer::SetNodeOpacity(const std::string& nodeName, float opacity)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  auto node = mArtboard->find(nodeName.c_str());
  if(!node)
  {
    return;
  }
  auto nodeInstance = node->as<rive::Node>();
  nodeInstance->opacity(opacity);
}

void RiveAnimationRenderer::SetNodeScale(const std::string& nodeName, Vector2 scale)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  auto node = mArtboard->find(nodeName.c_str());
  if(!node)
  {
    return;
  }
  auto nodeInstance = node->as<rive::Node>();
  nodeInstance->scaleX(scale.x);
  nodeInstance->scaleY(scale.y);
}

void RiveAnimationRenderer::SetNodeRotation(const std::string& nodeName, Degree degree)
{
  Dali::Mutex::ScopedLock lock(mMutex);
  auto node = mArtboard->find(nodeName.c_str());
  if(!node)
  {
    return;
  }
  auto nodeInstance = node->as<rive::Node>();
  nodeInstance->rotation(degree.degree);
}

void RiveAnimationRenderer::SetNodePosition(const std::string& nodeName, Vector2 position)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  auto node = mArtboard->find(nodeName.c_str());
  if(!node)
  {
    return;
  }
  auto nodeInstance = node->as<rive::Node>();
  nodeInstance->x(position.x);
  nodeInstance->y(position.y);
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
  const char* fragmentPrefix = mTargetSurface->GetCustomFragmentPrefix();

  size_t prefixIndex = fragmentShader.find(Dali::Shader::GetShaderVersionPrefix());
  if(fragmentPrefix)
  {
    if(prefixIndex == std::string::npos)
    {
      fragmentShader = fragmentPrefix + fragmentShader;
    }
    else
    {
      fragmentShader.insert(prefixIndex + Dali::Shader::GetShaderVersionPrefix().length(), std::string(fragmentPrefix) + "\n");
    }
  }

  // Get custom sampler type name
  const char* customSamplerTypename = mTargetSurface->GetCustomSamplerTypename();
  if(customSamplerTypename)
  {
    size_t position = fragmentShader.find(DEFAULT_SAMPLER_TYPENAME);
    if(position != std::string::npos)
    {
      fragmentShader.replace(position, strlen(DEFAULT_SAMPLER_TYPENAME), customSamplerTypename);
    }
  }

  // Set the modified shader again
  Shader newShader = Shader::New(vertexShader, fragmentShader);
  newShader.RegisterProperty(PIXEL_AREA_UNIFORM_NAME, FULL_TEXTURE_RECT);

  mRenderer.SetShader(newShader);

  mShaderChanged = true;
}

} // namespace Internal

} // namespace Extension

} // namespace Dali
