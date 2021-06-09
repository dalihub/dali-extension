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
#include <dali-extension/vector-animation-renderer/tizen-rive-animation-renderer.h>

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
#include <rive/tvg_renderer.hpp>

// INTERNAL INCLUDES
#include <dali-extension/vector-animation-renderer/tizen-vector-animation-manager.h>

// The plugin factories
extern "C" DALI_EXPORT_API Dali::VectorAnimationRendererPlugin* CreateVectorAnimationRendererPlugin(void)
{
  return new Dali::Plugin::TizenRiveAnimationRenderer;
}

namespace Dali
{
namespace Plugin
{
namespace
{
const char* const DEFAULT_SAMPLER_TYPENAME("sampler2D");
const char* const PIXEL_AREA_UNIFORM_NAME("pixelArea");

const Vector4 FULL_TEXTURE_RECT(0.f, 0.f, 1.f, 1.f);

} // unnamed namespace

TizenRiveAnimationRenderer::TizenRiveAnimationRenderer()
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
  mAnimationInstance(nullptr),
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

TizenRiveAnimationRenderer::~TizenRiveAnimationRenderer()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mAnimationInstance)
  {
    delete mAnimationInstance;
  }

  if(mFile)
  {
    delete mFile;
  }

  mSwCanvas->clear();
  tvg::Initializer::term(tvg::CanvasEngine::Sw);

  DALI_LOG_RELEASE_INFO("TizenRiveAnimationRenderer::~TizenRiveAnimationRenderer: this = %p\n", this);
}

void TizenRiveAnimationRenderer::LoadRiveFile(const std::string& filename)
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

  if(mAnimationInstance)
  {
    delete mAnimationInstance;
    mAnimationInstance = nullptr;
  }

  mAnimation = mArtboard->firstAnimation();
  if(mAnimation)
  {
    mAnimationInstance = new rive::LinearAnimationInstance(mAnimation);
    mStartFrameNumber = mAnimation->enableWorkArea() ? mAnimation->workStart() : 0;
    mTotalFrameNumber = mAnimation->enableWorkArea() ? mAnimation->workEnd() : mAnimation->duration();
    mTotalFrameNumber -= mStartFrameNumber;
  }
}

bool TizenRiveAnimationRenderer::Load(const std::string& url)
{
  mUrl = url;
  LoadRiveFile(mUrl);
  TizenVectorAnimationManager::Get().AddEventHandler(*this);

  DALI_LOG_RELEASE_INFO("TizenRiveAnimationRenderer::Initialize: file [%s] [%p]\n", url.c_str(), this);

  return true;
}

void TizenRiveAnimationRenderer::Finalize()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  TizenVectorAnimationManager::Get().RemoveEventHandler(*this);

  mRenderer.Reset();
  mTexture.Reset();
  mRenderedTexture.Reset();
  mPreviousTexture.Reset();

  mTargetSurface = nullptr;
  mTbmQueue      = nullptr;

  DALI_LOG_RELEASE_INFO("TizenRiveAnimationRenderer::Finalize: [%p]\n", this);
}

void TizenRiveAnimationRenderer::SetRenderer(Renderer renderer)
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

void TizenRiveAnimationRenderer::SetSize(uint32_t width, uint32_t height)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mWidth == width && mHeight == height)
  {
    DALI_LOG_RELEASE_INFO("TizenRiveAnimationRenderer::SetSize: Same size (%d, %d) [%p]\n", mWidth, mHeight, this);
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

  DALI_LOG_RELEASE_INFO("TizenRiveAnimationRenderer::SetSize: width = %d, height = %d [%p]\n", mWidth, mHeight, this);
}

bool TizenRiveAnimationRenderer::Render(uint32_t frameNumber)
{
  Dali::Mutex::ScopedLock lock(mMutex);
  if(!mTbmQueue || !mTargetSurface || !mArtboard || !mAnimationInstance)
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
    DALI_LOG_ERROR("TizenRiveAnimationRenderer::Render: tbm_surface_map is failed! [%d] [%p]\n", ret, this);
    tbm_surface_queue_cancel_dequeue(mTbmQueue, tbmSurface);
    return false;
  }

  unsigned char* buffer = info.planes[0].ptr;
  if(!buffer)
  {
    DALI_LOG_ERROR("TizenRiveAnimationRenderer::Render: tbm buffer pointer is null! [%p]\n", this);
    tbm_surface_unmap(tbmSurface);
    tbm_surface_queue_cancel_dequeue(mTbmQueue, tbmSurface);
    return false;
  }

  tbm_surface_internal_ref(tbmSurface);

  if(!mSwCanvas)
  {
    mSwCanvas = tvg::SwCanvas::gen();
  }
  mSwCanvas->target((uint32_t*)buffer, info.planes[0].stride / 4, info.width, info.height, tvg::SwCanvas::ARGB8888);
  mSwCanvas->clear();

  // Render Rive Frame
  frameNumber    = mStartFrameNumber + frameNumber;
  double elapsed = (float)frameNumber / 60.0f;

  mAnimationInstance->time(elapsed);
  mAnimationInstance->advance(0.0);
  mAnimationInstance->apply(mArtboard);

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

    TizenVectorAnimationManager::Get().TriggerEvent(*this);

    DALI_LOG_RELEASE_INFO("TizenRiveAnimationRenderer::Render: Resource ready [current = %d] [%p]\n", frameNumber, this);
  }

  return true;
}

uint32_t TizenRiveAnimationRenderer::GetTotalFrameNumber() const
{
  return mTotalFrameNumber;
}

float TizenRiveAnimationRenderer::GetFrameRate() const
{
  return mFrameRate;
}

void TizenRiveAnimationRenderer::GetDefaultSize(uint32_t& width, uint32_t& height) const
{
  width  = mDefaultWidth;
  height = mDefaultHeight;
}

void TizenRiveAnimationRenderer::GetLayerInfo(Property::Map& map) const
{
  return;
}

//TODO: This should be modified later, GetMarkerInfo is not proper interface name
bool TizenRiveAnimationRenderer::GetMarkerInfo(const std::string& marker, uint32_t& startFrame, uint32_t& endFrame) const
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(!mAnimation)
  {
    return false;
  }

  startFrame = mStartFrameNumber;
  endFrame   = mTotalFrameNumber;

  return true;
}

void TizenRiveAnimationRenderer::IgnoreRenderedFrame()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mTargetSurface)
  {
    mTargetSurface->IgnoreSourceImage();
  }
}

VectorAnimationRendererPlugin::UploadCompletedSignalType& TizenRiveAnimationRenderer::UploadCompletedSignal()
{
  return mUploadCompletedSignal;
}

void TizenRiveAnimationRenderer::NotifyEvent()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mResourceReadyTriggered)
  {
    DALI_LOG_RELEASE_INFO("TizenRiveAnimationRenderer::NotifyEvent: Set Texture [%p]\n", this);

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

//TODO: unify SetShader with tizen-vector-animation-renderer's since code are totally identical
void TizenRiveAnimationRenderer::SetShader()
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

} // namespace Plugin

} // namespace Dali
