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
#include <dali-extension/vector-animation-renderer/vector-animation-renderer.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/hash.h>
#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/texture-integ.h>
#include <dali/public-api/object/property-array.h>

#include <cstring> // for strlen()

// INTERNAL INCLUDES
#include <dali-extension/vector-animation-renderer/vector-animation-plugin-manager.h>

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

VectorAnimationRenderer::VectorAnimationRenderer()
: mUrl(),
  mMutex(),
  mRenderingDataMutex(),
  mRenderer(),
  mVectorRenderer(),
  mUploadCompletedSignal(),
  mTotalFrameNumber(0),
  mDefaultWidth(0),
  mDefaultHeight(0),
  mFrameRate(60.0f),
  mLoadFailed(false),
  mResourceReady(false),
  mShaderChanged(false),
  mResourceReadyTriggered(false),
  mEnableFixedCache(false)
{
  VectorAnimationPluginManager::Get().AddEventHandler(*this);
}

VectorAnimationRenderer::~VectorAnimationRenderer()
{
}

void VectorAnimationRenderer::Finalize()
{
  VectorAnimationPluginManager::Get().RemoveEventHandler(*this);

  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    mRenderingData[0]->mTexture.Reset();
    mRenderingData[1]->mTexture.Reset();

    OnFinalize();
  }

  mRenderer.Reset();

  Dali::Mutex::ScopedLock lock(mMutex);

  mVectorRenderer.reset();
  mPropertyCallbacks.clear();

  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "[%p]\n", this);
}

// Called by VectorAnimationTaskThread
bool VectorAnimationRenderer::Load(const std::string& url)
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

// Called by VectorAnimationTaskThread
bool VectorAnimationRenderer::Load(const Dali::Vector<uint8_t>& data)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  std::string jsonData(data.Begin(), data.End());    ///< Convert from raw buffer to string.
  auto        hashValue = Dali::CalculateHash(data); ///< Will be used for rlottie internal cache system.

  mVectorRenderer = rlottie::Animation::loadFromData(std::move(jsonData), std::to_string(hashValue));
  if(!mVectorRenderer)
  {
    DALI_LOG_ERROR("Failed to load a Lottie data [data size : %zu byte] [%p]\n", data.Size(), this);
    mLoadFailed = true;
    return false;
  }

  mTotalFrameNumber = static_cast<uint32_t>(mVectorRenderer->totalFrame());
  mFrameRate        = static_cast<float>(mVectorRenderer->frameRate());

  size_t w, h;
  mVectorRenderer->size(w, h);
  mDefaultWidth  = static_cast<uint32_t>(w);
  mDefaultHeight = static_cast<uint32_t>(h);

  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "data [data size : %zu byte] [%p]\n", data.Size(), this);

  return true;
}

void VectorAnimationRenderer::SetRenderer(Renderer renderer)
{
  mRenderer      = renderer;
  mShaderChanged = false;

  if(IsTargetPrepared())
  {
    Dali::Mutex::ScopedLock lock(mMutex);

    if(IsRenderReady())
    {
      TextureSet textureSet = renderer.GetTextures();

      textureSet.SetTexture(0, GetTargetTexture());

      mUploadCompletedSignal.Emit();
    }

    SetShader(mCurrentDataIndex);
  }
}

void VectorAnimationRenderer::SetSize(uint32_t width, uint32_t height)
{
  if(!Stage::IsCoreThread())
  {
    DALI_LOG_ERROR("SetSize should be called by Core Thread.\n");
    return;
  }

  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);

    if(mRenderingData[mCurrentDataIndex]->mWidth == width && mRenderingData[mCurrentDataIndex]->mHeight == height)
    {
      return;
    }

    // If updated data is not used yet, do not change current data index.
    if(mIsDataActivated)
    {
      mCurrentDataIndex = 1u - mCurrentDataIndex;
    }
    std::shared_ptr<RenderingData> updatedData = mRenderingData[mCurrentDataIndex];

    updatedData->mWidth  = width;
    updatedData->mHeight = height;

    PrepareTarget(mCurrentDataIndex);

    if(mRenderer)
    {
      SetShader(mCurrentDataIndex);
    }

    OnSetSize(mCurrentDataIndex);

    mIsDataActivated = false;
    SetRenderingDataUpdated(true);

    DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "width = %d, height = %d [%p]\n", updatedData->mWidth, updatedData->mHeight, this);
  }
}

uint32_t VectorAnimationRenderer::GetTotalFrameNumber() const
{
  return mTotalFrameNumber;
}

float VectorAnimationRenderer::GetFrameRate() const
{
  return mFrameRate;
}

void VectorAnimationRenderer::GetDefaultSize(uint32_t& width, uint32_t& height) const
{
  width  = mDefaultWidth;
  height = mDefaultHeight;
}

void VectorAnimationRenderer::GetLayerInfo(Property::Map& map) const
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

bool VectorAnimationRenderer::GetMarkerInfo(const std::string& marker, uint32_t& startFrame, uint32_t& endFrame) const
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

void VectorAnimationRenderer::GetMarkerInfo(Property::Map& map) const
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mVectorRenderer)
  {
    auto markerList = mVectorRenderer->markers();
    for(auto&& iter : markerList)
    {
      Property::Array frames;
      frames.PushBack(std::get<1>(iter));
      frames.PushBack(std::get<2>(iter));
      map.Add(std::get<0>(iter), frames);
    }
  }
}

void VectorAnimationRenderer::InvalidateBuffer()
{
  Dali::Mutex::ScopedLock lock(mMutex);
  mResourceReady = false;
}

void VectorAnimationRenderer::AddPropertyValueCallback(const std::string& keyPath, VectorProperty property, CallbackBase* callback, int32_t id)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  mPropertyCallbacks.push_back(std::unique_ptr<CallbackBase>(callback));

  switch(property)
  {
    case VectorProperty::FILL_COLOR:
    {
      mVectorRenderer->setValue<rlottie::Property::FillColor>(keyPath,
                                                              [property, callback, id](const rlottie::FrameInfo& info)
                                                              {
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
                                                                [property, callback, id](const rlottie::FrameInfo& info)
                                                                {
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
                                                                [property, callback, id](const rlottie::FrameInfo& info)
                                                                {
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
                                                                  [property, callback, id](const rlottie::FrameInfo& info)
                                                                  {
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
                                                                [property, callback, id](const rlottie::FrameInfo& info)
                                                                {
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
                                                             [property, callback, id](const rlottie::FrameInfo& info)
                                                             {
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
                                                               [property, callback, id](const rlottie::FrameInfo& info)
                                                               {
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
                                                            [property, callback, id](const rlottie::FrameInfo& info)
                                                            {
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
                                                               [property, callback, id](const rlottie::FrameInfo& info)
                                                               {
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
                                                              [property, callback, id](const rlottie::FrameInfo& info)
                                                              {
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

void VectorAnimationRenderer::KeepRasterizedBuffer()
{
  Dali::Mutex::ScopedLock lock(mMutex);
  mEnableFixedCache = true;
  mDecodedBuffers.clear();
}

VectorAnimationRendererPlugin::UploadCompletedSignalType& VectorAnimationRenderer::UploadCompletedSignal()
{
  return mUploadCompletedSignal;
}

void VectorAnimationRenderer::NotifyEvent()
{
  bool emitSignal = false;
  {
    Dali::Mutex::ScopedLock lock(mMutex);

    if(mResourceReadyTriggered)
    {
      DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "Set Texture [%p]\n", this);

      // Set texture
      if(mRenderer && GetTargetTexture())
      {
        TextureSet textureSet = mRenderer.GetTextures();
        textureSet.SetTexture(0, GetTargetTexture());
      }

      mResourceReadyTriggered = false;
      emitSignal              = true;
    }

    OnNotify();
    mRenderingData[1u - mCurrentDataIndex]->mTexture.Reset();
  }
  if(emitSignal)
  {
    mUploadCompletedSignal.Emit();
  }
}

void VectorAnimationRenderer::SetRenderingDataUpdated(bool renderingDataUpdated)
{
  if(!Stage::IsCoreThread())
  {
    DALI_LOG_ERROR("SetRenderingDataUpdated should be called by Core Thread.\n");
    return;
  }
  mIsRenderingDataUpdated = renderingDataUpdated;
}

} // namespace Plugin

} // namespace Dali
