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
#include <dali-extension/vector-animation-renderer/tizen-vector-animation-renderer-x.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/rendering/texture-devel.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/property-array.h>

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
  mMutex(),
  mRenderer(),
  mTexture(),
  mLottieSurface(),
  mVectorRenderer(),
  mUploadCompletedSignal(),
  mTotalFrameNumber(0),
  mWidth(0),
  mHeight(0),
  mDefaultWidth(0),
  mDefaultHeight(0),
  mFrameRate(60.0f),
  mLoadFailed(false),
  mResourceReady(false),
  mShaderChanged(false),
  mResourceReadyTriggered(false),
  mRenderCallback(std::unique_ptr<EventThreadCallback>(new EventThreadCallback(MakeCallback(this, &TizenVectorAnimationRenderer::OnLottieRendered))))
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
  mVectorRenderer.reset();
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

void TizenVectorAnimationRenderer::OnLottieRendered()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if (mPixelBuffer)
  {
    PixelData pixelData = mPixelBuffer.CreatePixelData();
    mTexture.Upload(pixelData);
  }
}

void TizenVectorAnimationRenderer::SetRenderer(Renderer renderer)
{
  mRenderer      = renderer;
  mShaderChanged = false;

  if (mPixelBuffer)
  {
    Dali::Mutex::ScopedLock lock(mMutex);

    if (mResourceReady && mTexture)
    {
      TextureSet textureSet = renderer.GetTextures();

      textureSet.SetTexture(0, mTexture);

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

  mTexture = Texture::New(Dali::TextureType::TEXTURE_2D, Dali::Pixel::BGRA8888, width, height);

  mPixelBuffer = Dali::Devel::PixelBuffer::New(width, height, Dali::Pixel::BGRA8888);
  mLottieSurface = rlottie::Surface(reinterpret_cast<uint32_t*>(mPixelBuffer.GetBuffer()), width, height, static_cast<size_t>(mPixelBuffer.GetStride() * 4));

  if(mRenderer)
  {
    SetShader();
  }

  mWidth  = width;
  mHeight = height;

  mResourceReady = false;

  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "width = %d, height = %d [%p]\n", mWidth, mHeight, this);
}

bool TizenVectorAnimationRenderer::Render(uint32_t frameNumber)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if (!mVectorRenderer || !mPixelBuffer)
  {
    return false;
  }

  mVectorRenderer->renderSync(frameNumber, mLottieSurface);
  mRenderCallback->Trigger();

  if(!mResourceReady)
  {
    mResourceReady = true;
    mResourceReadyTriggered = true;

    TizenVectorAnimationManager::Get().TriggerEvent(*this);
  }

  return true;
}

void TizenVectorAnimationRenderer::RenderStopped()
{
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
  bool emitSignal = false;
  {
    Dali::Mutex::ScopedLock lock(mMutex);

    if(mResourceReadyTriggered)
    {
      DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "Set Texture [%p]\n", this);

      // Set texture
      if(mRenderer && mTexture)
      {
        TextureSet textureSet = mRenderer.GetTextures();
        textureSet.SetTexture(0, mTexture);
      }

      mResourceReadyTriggered = false;
      emitSignal              = true;
    }
  }
  if(emitSignal)
  {
    mUploadCompletedSignal.Emit();
  }
}

void TizenVectorAnimationRenderer::SetShader()
{
  if(mShaderChanged || !mTexture)
  {
    return;
  }

  // Not implemented

  mShaderChanged = true;
}

void TizenVectorAnimationRenderer::ResetBuffers()
{
  if (mPixelBuffer)
  {
    uint32_t bufferSize = mPixelBuffer.GetWidth() * mPixelBuffer.GetHeight() * Dali::Pixel::GetBytesPerPixel(mPixelBuffer.GetPixelFormat());
    memset(mPixelBuffer.GetBuffer(), 0, bufferSize);
  }
}

} // namespace Plugin

} // namespace Dali
