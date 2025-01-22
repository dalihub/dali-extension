/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include "tizen-video-constraint-helper.h"

// EXTERNAL INCLUDES
#include <tbm_surface_internal.h>

namespace Dali
{
namespace Plugin
{
VideoConstraintHelperPtr VideoConstraintHelper::New(Dali::NativeImageSourcePtr nativeImageSourcePtr)
{
  VideoConstraintHelperPtr ptr = new VideoConstraintHelper(nativeImageSourcePtr);
  return ptr;
}

VideoConstraintHelper::VideoConstraintHelper(Dali::NativeImageSourcePtr nativeImageSourcePtr)
: mNativeImageSourcePtr(nativeImageSourcePtr),
  mSurface(NULL),
  mOrientation(0.0f),
  mWidth(0),
  mHeight(0),
  mIsSetInfo(false),
  mIsAutoRotationEnabled(false),
  mIsLetterBoxEnabled(false)
{
}

VideoConstraintHelper::~VideoConstraintHelper()
{
}

void VideoConstraintHelper::SetInfo(tbm_surface_h surface, int orientation, int width, int height)
{
  Dali::Mutex::ScopedLock lock(mConstraintMutex);
  mIsSetInfo = true;
  if(mSurface != NULL)
  {
    tbm_surface_internal_unref(mSurface);
  }

  mSurface = surface;

  if(mSurface != NULL)
  {
    tbm_surface_internal_ref(mSurface);
  }

  // Orientation should be set to zero when auto rotation is disabled.
  mOrientation = mIsAutoRotationEnabled ? orientation : 0;
  if(mIsLetterBoxEnabled)
  {
    mWidth  = width;
    mHeight = height;
  }
}

void VideoConstraintHelper::UpdateVideo()
{
  {
    Dali::Mutex::ScopedLock lock(mConstraintMutex);
    if(mIsSetInfo && mNativeImageSourcePtr)
    {
      mNativeImageSourcePtr->SetSource(mSurface);
      if(mSurface != NULL)
      {
        tbm_surface_internal_unref(mSurface);
      }
      mSurface   = NULL;
      mIsSetInfo = false;
    }
  }
}

Dali::Vector4 VideoConstraintHelper::GetOrientationMatrix()
{
  Dali::Mutex::ScopedLock lock(mConstraintMutex);
  switch(mOrientation)
  {
    case 0:
    {
      return Dali::Vector4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    case 90:
    {
      return Dali::Vector4(0.0f, -1.0f, 1.0f, 0.0f);
    }
    case 180:
    {
      return Dali::Vector4(-1.0f, 0.0f, 0.0f, -1.0f);
    }
    case 270:
    {
      return Dali::Vector4(0.0f, 1.0f, -1.0f, 0.0f);
    }
    default:
    {
      return Dali::Vector4(1.0f, 0.0f, 0.0f, 1.0f);
    }
  }
}

Dali::Vector2 VideoConstraintHelper::RetriveSize()
{
  Dali::Mutex::ScopedLock lock(mConstraintMutex);
  if(mOrientation == 0 || mOrientation == 180)
  {
    return Dali::Vector2(mWidth, mHeight);
  }
  else
  {
    return Dali::Vector2(mHeight, mWidth);
  }
}

void VideoConstraintHelper::SetAutoRotationEnabled(bool enable)
{
  Dali::Mutex::ScopedLock lock(mConstraintMutex);
  mIsAutoRotationEnabled = enable;
  if(!mIsAutoRotationEnabled)
  {
    mOrientation = 0;
  }
}

bool VideoConstraintHelper::IsAutoRotationEnabled() const
{
  return mIsAutoRotationEnabled;
}

void VideoConstraintHelper::SetLetterBoxEnabled(bool enable)
{
  Dali::Mutex::ScopedLock lock(mConstraintMutex);
  mIsLetterBoxEnabled = enable;
  if(!mIsLetterBoxEnabled)
  {
    mWidth  = 0;
    mHeight = 0;
  }
}

bool VideoConstraintHelper::IsLetterBoxEnabled() const
{
  return mIsLetterBoxEnabled;
}

} // namespace Plugin
} // namespace Dali
