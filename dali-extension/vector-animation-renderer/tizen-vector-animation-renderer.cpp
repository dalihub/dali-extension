/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali-extension/vector-animation-renderer/tizen-vector-animation-renderer.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/property-array.h>
#include <tbm_surface_internal.h>
#include <cstring> // for strlen()

// INTERNAL INCLUDES
#include <dali-extension/vector-animation-renderer/tizen-vector-animation-manager.h>

// The plugin factories
extern "C" DALI_EXPORT_API Dali::VectorAnimationRendererPlugin* CreateVectorAnimationRendererPlugin(void)
{
  return new Dali::Plugin::TizenVectorAnimationRenderer;
}

namespace Dali
{
namespace Plugin
{
namespace
{
const char* const PIXEL_AREA_UNIFORM_NAME("pixelArea");
const Vector4     FULL_TEXTURE_RECT(0.f, 0.f, 1.f, 1.f);

#if defined(DEBUG_ENABLED)
Debug::Filter* gVectorAnimationLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_VECTOR_ANIMATION");
#endif
} // unnamed namespace

TizenVectorAnimationRenderer::TizenVectorAnimationRenderer()
: mUrl(),
  mBuffers(),
  mMutex(),
  mRenderer(),
  mTexture(),
  mRenderedTexture(),
  mPreviousTexture(),
  mTargetSurface(),
  mVectorRenderer(),
  mUploadCompletedSignal(),
  mTbmQueue(NULL),
  mTotalFrameNumber(0),
  mWidth(0),
  mHeight(0),
  mDefaultWidth(0),
  mDefaultHeight(0),
  mFrameRate(60.0f),
  mLoadFailed(false),
  mResourceReady(false),
  mShaderChanged(false),
  mResourceReadyTriggered(false)
{
  TizenVectorAnimationManager::Get().AddEventHandler(*this);
}

TizenVectorAnimationRenderer::~TizenVectorAnimationRenderer()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  ResetBuffers();
  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "this = %p\n", this);
}

void TizenVectorAnimationRenderer::Finalize()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  TizenVectorAnimationManager::Get().RemoveEventHandler(*this);

  mRenderer.Reset();
  mTexture.Reset();
  mRenderedTexture.Reset();
  mPreviousTexture.Reset();
  mVectorRenderer.reset();

  mTargetSurface = nullptr;
  mTbmQueue      = NULL;

  mPropertyCallbacks.clear();

  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "[%p]\n", this);
}

bool TizenVectorAnimationRenderer::Load(const std::string& url)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  mUrl = url;

  mVectorRenderer = rlottie::Animation::loadFromFile(mUrl);
  if(!mVectorRenderer)
  {
    DALI_LOG_ERROR("Failed to load a Lottie file [%s] [%p]\n", mUrl.c_str(), this);
    mLoadFailed = true;
    return false;
  }

  mTotalFrameNumber = static_cast<uint32_t>(mVectorRenderer->totalFrame());
  mFrameRate        = static_cast<float>(mVectorRenderer->frameRate());

  size_t w, h;
  mVectorRenderer->size(w, h);
  mDefaultWidth  = static_cast<uint32_t>(w);
  mDefaultHeight = static_cast<uint32_t>(h);

  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "file [%s] [%p]\n", url.c_str(), this);

  return true;
}

void TizenVectorAnimationRenderer::SetRenderer(Renderer renderer)
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

void TizenVectorAnimationRenderer::SetSize(uint32_t width, uint32_t height)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mWidth == width && mHeight == height)
  {
    DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "Same size (%d, %d) [%p]\n", mWidth, mHeight, this);
    return;
  }

  if(mLoadFailed)
  {
    DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "Load is failed. Do not make texture [%p]\n", this);
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

  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "width = %d, height = %d [%p]\n", mWidth, mHeight, this);
}

bool TizenVectorAnimationRenderer::Render(uint32_t frameNumber)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(!mTbmQueue || !mVectorRenderer || !mTargetSurface)
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
    DALI_LOG_ERROR("TizenVectorAnimationRenderer::Render: tbm_surface_map is failed! [%d] [%p]\n", ret, this);
    tbm_surface_queue_cancel_dequeue(mTbmQueue, tbmSurface);
    return false;
  }

  unsigned char* buffer = info.planes[0].ptr;
  if(info.width != mWidth || info.height != mHeight || !buffer)
  {
    DALI_LOG_ERROR("TizenVectorAnimationRenderer::Render: Invalid tbm surface! [%d, %d, %p] [%p]\n", info.width, info.height, buffer, this);
    tbm_surface_unmap(tbmSurface);
    tbm_surface_queue_cancel_dequeue(mTbmQueue, tbmSurface);
    return false;
  }

  rlottie::Surface surface;
  bool             existing = false;

  if(!mResourceReady)
  {
    // Need to reset buffer list
    ResetBuffers();
  }
  else
  {
    for(auto&& iter : mBuffers)
    {
      if(iter.first == tbmSurface)
      {
        // Find the buffer in the existing list
        existing = true;
        surface  = iter.second;
        break;
      }
    }
  }

  if(!existing)
  {
    tbm_surface_internal_ref(tbmSurface);

    // Create Surface object
    surface = rlottie::Surface(reinterpret_cast<uint32_t*>(buffer), mWidth, mHeight, static_cast<size_t>(info.planes[0].stride));

    // Push the buffer
    mBuffers.push_back(SurfacePair(tbmSurface, surface));
  }

  // Render the frame
  mVectorRenderer->renderSync(frameNumber, surface);

  tbm_surface_unmap(tbmSurface);

  tbm_surface_queue_enqueue(mTbmQueue, tbmSurface);

  if(!mResourceReady)
  {
    mPreviousTexture        = mRenderedTexture; // It is used to destroy the object in the main thread.
    mRenderedTexture        = mTexture;
    mResourceReady          = true;
    mResourceReadyTriggered = true;

    TizenVectorAnimationManager::Get().TriggerEvent(*this);

    DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "Resource ready [current = %d] [%p]\n", frameNumber, this);
  }

  return true;
}

uint32_t TizenVectorAnimationRenderer::GetTotalFrameNumber() const
{
  return mTotalFrameNumber;
}

float TizenVectorAnimationRenderer::GetFrameRate() const
{
  return mFrameRate;
}

void TizenVectorAnimationRenderer::GetDefaultSize(uint32_t& width, uint32_t& height) const
{
  width  = mDefaultWidth;
  height = mDefaultHeight;
}

void TizenVectorAnimationRenderer::GetLayerInfo(Property::Map& map) const
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mVectorRenderer)
  {
    auto layerInfo = mVectorRenderer->layers();

    for(auto&& iter : layerInfo)
    {
      Property::Array frames;
      frames.PushBack(std::get<1>(iter));
      frames.PushBack(std::get<2>(iter));
      map.Add(std::get<0>(iter), frames);
    }
  }
}

bool TizenVectorAnimationRenderer::GetMarkerInfo(const std::string& marker, uint32_t& startFrame, uint32_t& endFrame) const
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mVectorRenderer)
  {
    auto markerList = mVectorRenderer->markers();
    for(auto&& iter : markerList)
    {
      if(std::get<0>(iter).compare(marker) == 0)
      {
        startFrame = static_cast<uint32_t>(std::get<1>(iter));
        endFrame   = static_cast<uint32_t>(std::get<2>(iter));
        return true;
      }
    }
  }
  return false;
}

void TizenVectorAnimationRenderer::InvalidateBuffer()
{
  Dali::Mutex::ScopedLock lock(mMutex);
  mResourceReady = false;
}

void TizenVectorAnimationRenderer::AddPropertyValueCallback(const std::string& keyPath, VectorProperty property, CallbackBase* callback, int32_t id)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  mPropertyCallbacks.push_back(std::unique_ptr<CallbackBase>(callback));

  switch(property)
  {
    case VectorProperty::FILL_COLOR:
    {
      mVectorRenderer->setValue<rlottie::Property::FillColor>(keyPath,
                                                              [property, callback, id](const rlottie::FrameInfo& info) {
                                                                Property::Value value = CallbackBase::ExecuteReturn<Property::Value>(*callback, id, property, info.curFrame());
                                                                Vector3         color;
                                                                if(value.Get(color))
                                                                {
                                                                  return rlottie::Color(color.r, color.g, color.b);
                                                                }
                                                                return rlottie::Color(1.0f, 1.0f, 1.0f);
                                                              });
      break;
    }
    case VectorProperty::FILL_OPACITY:
    {
      mVectorRenderer->setValue<rlottie::Property::FillOpacity>(keyPath,
                                                                [property, callback, id](const rlottie::FrameInfo& info) {
                                                                  Property::Value value = CallbackBase::ExecuteReturn<Property::Value>(*callback, id, property, info.curFrame());
                                                                  float           opacity;
                                                                  if(value.Get(opacity))
                                                                  {
                                                                    return opacity * 100;
                                                                  }
                                                                  return 100.0f;
                                                                });
      break;
    }
    case VectorProperty::STROKE_COLOR:
    {
      mVectorRenderer->setValue<rlottie::Property::StrokeColor>(keyPath,
                                                                [property, callback, id](const rlottie::FrameInfo& info) {
                                                                  Property::Value value = CallbackBase::ExecuteReturn<Property::Value>(*callback, id, property, info.curFrame());
                                                                  Vector3         color;
                                                                  if(value.Get(color))
                                                                  {
                                                                    return rlottie::Color(color.r, color.g, color.b);
                                                                  }
                                                                  return rlottie::Color(1.0f, 1.0f, 1.0f);
                                                                });
      break;
    }
    case VectorProperty::STROKE_OPACITY:
    {
      mVectorRenderer->setValue<rlottie::Property::StrokeOpacity>(keyPath,
                                                                  [property, callback, id](const rlottie::FrameInfo& info) {
                                                                    Property::Value value = CallbackBase::ExecuteReturn<Property::Value>(*callback, id, property, info.curFrame());
                                                                    float           opacity;
                                                                    if(value.Get(opacity))
                                                                    {
                                                                      return opacity * 100;
                                                                    }
                                                                    return 100.0f;
                                                                  });
      break;
    }
    case VectorProperty::STROKE_WIDTH:
    {
      mVectorRenderer->setValue<rlottie::Property::StrokeWidth>(keyPath,
                                                                [property, callback, id](const rlottie::FrameInfo& info) {
                                                                  Property::Value value = CallbackBase::ExecuteReturn<Property::Value>(*callback, id, property, info.curFrame());
                                                                  float           width;
                                                                  if(value.Get(width))
                                                                  {
                                                                    return width;
                                                                  }
                                                                  return 1.0f;
                                                                });
      break;
    }
    case VectorProperty::TRANSFORM_ANCHOR:
    {
      mVectorRenderer->setValue<rlottie::Property::TrAnchor>(keyPath,
                                                             [property, callback, id](const rlottie::FrameInfo& info) {
                                                               Property::Value value = CallbackBase::ExecuteReturn<Property::Value>(*callback, id, property, info.curFrame());
                                                               Vector2         point;
                                                               if(value.Get(point))
                                                               {
                                                                 return rlottie::Point(point.x, point.y);
                                                               }
                                                               return rlottie::Point(0.0f, 0.0f);
                                                             });
      break;
    }
    case VectorProperty::TRANSFORM_POSITION:
    {
      mVectorRenderer->setValue<rlottie::Property::TrPosition>(keyPath,
                                                               [property, callback, id](const rlottie::FrameInfo& info) {
                                                                 Property::Value value = CallbackBase::ExecuteReturn<Property::Value>(*callback, id, property, info.curFrame());
                                                                 Vector2         position;
                                                                 if(value.Get(position))
                                                                 {
                                                                   return rlottie::Point(position.x, position.y);
                                                                 }
                                                                 return rlottie::Point(0.0f, 0.0f);
                                                               });
      break;
    }
    case VectorProperty::TRANSFORM_SCALE:
    {
      mVectorRenderer->setValue<rlottie::Property::TrScale>(keyPath,
                                                            [property, callback, id](const rlottie::FrameInfo& info) {
                                                              Property::Value value = CallbackBase::ExecuteReturn<Property::Value>(*callback, id, property, info.curFrame());
                                                              Vector2         scale;
                                                              if(value.Get(scale))
                                                              {
                                                                return rlottie::Size(scale.x, scale.y);
                                                              }
                                                              return rlottie::Size(100.0f, 100.0f);
                                                            });
      break;
    }
    case VectorProperty::TRANSFORM_ROTATION:
    {
      mVectorRenderer->setValue<rlottie::Property::TrRotation>(keyPath,
                                                               [property, callback, id](const rlottie::FrameInfo& info) {
                                                                 Property::Value value = CallbackBase::ExecuteReturn<Property::Value>(*callback, id, property, info.curFrame());
                                                                 float           rotation;
                                                                 if(value.Get(rotation))
                                                                 {
                                                                   return rotation;
                                                                 }
                                                                 return 0.0f;
                                                               });
      break;
    }
    case VectorProperty::TRANSFORM_OPACITY:
    {
      mVectorRenderer->setValue<rlottie::Property::TrOpacity>(keyPath,
                                                              [property, callback, id](const rlottie::FrameInfo& info) {
                                                                Property::Value value = CallbackBase::ExecuteReturn<Property::Value>(*callback, id, property, info.curFrame());
                                                                float           opacity;
                                                                if(value.Get(opacity))
                                                                {
                                                                  return opacity * 100;
                                                                }
                                                                return 100.0f;
                                                              });
      break;
    }
  }
}

VectorAnimationRendererPlugin::UploadCompletedSignalType& TizenVectorAnimationRenderer::UploadCompletedSignal()
{
  return mUploadCompletedSignal;
}

void TizenVectorAnimationRenderer::NotifyEvent()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mResourceReadyTriggered)
  {
    DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "Set Texture [%p]\n", this);

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

void TizenVectorAnimationRenderer::SetShader()
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

void TizenVectorAnimationRenderer::ResetBuffers()
{
  for(auto&& iter : mBuffers)
  {
    tbm_surface_internal_unref(iter.first);
  }
  mBuffers.clear();
}

} // namespace Plugin

} // namespace Dali
