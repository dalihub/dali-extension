#ifndef __DALI_EXTENSION_INTERNAL_CAPTURE_H__
#define __DALI_EXTENSION_INTERNAL_CAPTURE_H__

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <string>
#include <tbm_surface.h>

// INTERNAL INCLUDES
#include <dali/dali.h>
#include <dali/public-api/object/ref-object.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/common/dali-common.h>
#include <dali-extension/devel-api/capture/capture.h>

namespace Dali
{

namespace Extension
{

namespace Internal
{

class Capture;
typedef IntrusivePtr<Capture> CapturePtr;

class Capture : public BaseObject, public ConnectionTracker
{
public:
  typedef unsigned char Pixel;
  typedef Pixel* PixelPtr;

public:
  /**
   * @brief Constructor.
   */
  Capture();
  Capture(Dali::Camera::ProjectionMode mode);

  /**
   * @copydoc Dali::Extension::New
   */
  static CapturePtr New();

  /**
   * @copydoc Dali::Extension::New
   */
  static CapturePtr New(Dali::Camera::ProjectionMode mode);

  /**
   * @copydoc Dali::Extension::Start
   */
  void Start(Actor source, const Vector2& size, const std::string &path, const Vector4& clearColor);

  /**
   * @copydoc Dali::Extension::GetFinishState
   */
  Dali::Extension::Capture::FinishState GetFinishState();

  /**
   * @copydoc Dali::Extension::FinishedSignal
   */
  Dali::Extension::Capture::CaptureSignalType& FinishedSignal();

protected:

  /**
   * @brief Second-phase constructor. Must be called immediately after creating a new Capture;
   */
  void Initialize(void);

  /**
   * @brief A reference counted object may only be deleted by calling Unreference()
   */
  virtual ~Capture();

private:
  /**
   * @brief Create surface.
   *
   * @param[in] size of surface.
   */
  void CreateSurface(const Vector2& size);

  /**
   * @brief Delete surface.
   */
  void DeleteSurface();

  /**
   * @brief Clear surface with color.
   *
   * @param[in] size of clear aread.
   */
  void ClearSurface(const Vector2& size);

  /**
   * @brief Query whether surface is created or not.
   *
   * @return True is surface is created.
   */
  bool IsSurfaceCreated();

  /**
   * @brief Create native image source.
   */
  void CreateNativeImageSource();

  /**
   * @brief Delete native image source.
   */
  void DeleteNativeImageSource();

  /**
   * @brief Query whether native image source is created or not.
   *
   * @return True is native image source is created.
   */
  bool IsNativeImageSourceCreated();

  /**
   * @brief Create frame buffer.
   */
  void CreateFrameBuffer();

  /**
   * @brief Delete frame buffer.
   */
  void DeleteFrameBuffer();

  /**
   * @brief Query whether frame buffer is created or not.
   *
   * @return True is frame buffer is created.
   */
  bool IsFrameBufferCreated();

  /**
   * @brief Setup render task.
   *
   * @param[in] source is captured.
   * @param[in] clearColor background color
   */
  void SetupRenderTask(Actor source, const Vector4& clearColor);

  /**
   * @brief Unset render task.
   */
  void UnsetRenderTask();

  /**
   * @brief Query whether render task is setup or not.
   *
   * @return True is render task is setup.
   */
  bool IsRenderTaskSetup();

  /**
   * @brief Setup resources for capture.
   *
   * @param[in] size is surface size.
   * @param[in] clearColor is clear color of surface.
   * @param[in] source is captured.
   */
  void SetupResources(const Vector2& size, const Vector4& clearColor, Actor source);

  /**
   * @brief Unset resources for capture.
   */
  void UnsetResources();

  /**
   * @brief Callback when render is finished.
   *
   * @param[in] task is used for capture.
   */
  void OnRenderFinished(RenderTask& task);

  /**
   * @brief Callback when timer is finished.
   *
   * @return True is timer start again.
   */
  bool OnTimeOut();

  /**
   * @brief Save framebuffer.
   *
   * @return True is success to save, false is fail.
   */
  bool Save();

private:

  // Undefined
  Capture(const Capture&);

  // Undefined
  Capture& operator=(const Capture& rhs);

private:
  Texture                                     mNativeTexture;
  FrameBuffer                                 mFrameBuffer;
  RenderTask                                  mRenderTask;
  Actor                                       mParent;
  Actor                                       mSource;
  CameraActor                                 mCameraActor;
  Camera::ProjectionMode                      mProjectionMode;
  Timer                                       mTimer;           ///< For timeout.
  Dali::Extension::Capture::CaptureSignalType mFinishedSignal;
  std::string                                 mPath;
  NativeImageSourcePtr                        mNativeImageSourcePtr;  ///< pointer to surface image
  tbm_surface_h                               mTbmSurface;
  Dali::Extension::Capture::FinishState       mFinishState;
};

}  // End of namespace Internal

// Helpers for public-api forwarding methods

inline Internal::Capture& GetImpl(Extension::Capture& captureWorker)
{
  DALI_ASSERT_ALWAYS(captureWorker && "Capture handle is empty");

  BaseObject& handle = captureWorker.GetBaseObject();

  return static_cast<Internal::Capture&>(handle);
}

inline const Internal::Capture& GetImpl(const Extension::Capture& captureWorker)
{
  DALI_ASSERT_ALWAYS(captureWorker && "Capture handle is empty");

  const BaseObject& handle = captureWorker.GetBaseObject();

  return static_cast<const Internal::Capture&>(handle);
}

}  // End of namespace Extension

}  // End of namespace Dali

#endif // __DALI_EXTENSION_INTERNAL_CAPTURE_H__
