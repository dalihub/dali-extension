#ifndef __DALI_TIZEN_VIDEO_CONSTRAINT_HELPER_H__
#define __DALI_TIZEN_VIDEO_CONSTRAINT_HELPER_H__

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

// INTERNAL INCLUDES

// EXTERNAL INCLUDES
#include <dali/devel-api/threading/mutex.h>
#include <dali/public-api/adaptor-framework/native-image-source.h>
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/math/vector2.h>
#include <dali/public-api/math/vector4.h>
#include <dali/public-api/object/ref-object.h>
#include <tbm_surface.h>

namespace Dali
{
namespace Plugin
{
class VideoConstraintHelper;
typedef Dali::IntrusivePtr<VideoConstraintHelper> VideoConstraintHelperPtr;

class VideoConstraintHelper : public Dali::RefObject
{
public:
  /**
   * @brief Creates a new instance of VideoConstraintHelper
   * @param[in] nativeImageSourcePtr Native image source pointer
   * @return A pointer to a newly allocated video constraint helper
   */
  static VideoConstraintHelperPtr New(Dali::NativeImageSourcePtr nativeImageSourcePtr);

  /**
   * @brief Sets the video info
   * @param[in] surface TBM surface
   * @param[in] orientation Orientation of the video
   * @param[in] width Width of the video
   * @param[in] height Height of the video
   */
  void SetInfo(tbm_surface_h surface, int orientation, int width, int height);

  /**
   * @brief Enables or disables auto rotation
   * @param[in] enable True to enable auto rotation, false otherwise. Default value is false.
   */
  void SetAutoRotationEnabled(bool enable);

  /**
   * @brief Checks if auto rotation is enabled
   * @return True if auto rotation is enabled, false otherwise. Default value is false.
   */
  bool IsAutoRotationEnabled() const;

  /**
   * @brief Enables or disables letter box
   * @param[in] enable True to enable letter box, false otherwise. Default value is false.
   */
  void SetLetterBoxEnabled(bool enable);

  /**
   * @brief Checks if letter box is enabled
   * @return True if letter box is enabled, false otherwise. Default value is false.
   */
  bool IsLetterBoxEnabled() const;

  /**
   * @brief Gets the video orientation matrix
   * @return OrientationMatrix of the video
   */
  Dali::Vector4 GetOrientationMatrix();

  /**
   * @brief Gets the video width and height
   * @return Size of the video in Vector2 format (width, height)
   */
  Dali::Vector2 RetriveSize();

  /**
   * @brief Updates the video
   */
  void UpdateVideo();

private:
  VideoConstraintHelper(Dali::NativeImageSourcePtr nativeImageSourcePtr);

  virtual ~VideoConstraintHelper();

private:
  Dali::NativeImageSourcePtr mNativeImageSourcePtr;

  Dali::Mutex   mConstraintMutex;
  tbm_surface_h mSurface;

  int mOrientation;
  int mWidth;
  int mHeight;

  bool mIsSetInfo;
  bool mIsAutoRotationEnabled;
  bool mIsLetterBoxEnabled;
};

} // namespace Plugin
} // namespace Dali

#endif // __DALI_TIZEN_VIDEO_CONSTRAINT_HELPER_H__
