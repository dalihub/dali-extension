#ifndef __DALI_EXTENSION_CAPTURE_H__
#define __DALI_EXTENSION_CAPTURE_H__

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

/**
 * @addtogroup CAPI_DALI_EXTENSION_FRAMEWORK_MODULE
 * @{
 */

// EXTERNAL HEADERS
#include <dali/dali.h>

// INTERNAL HEADERS

namespace Dali
{

namespace Extension
{

namespace Internal
{
class Capture;
}

/**
 * @brief Capture snapshots the current scene and save as a file.
 *
 * Applications should follow the example below to create capture :
 *
 * @code
 * Capture capture = Capture::New();
 * @endcode
 *
 * If required, you can also connect class member function to a signal :
 *
 * @code
 * capture.FinishedSignal().Connect(this, &CaptureSceneExample::OnCaptureFinished);
 * @endcode
 *
 * At the connected class member function, you can know whether capture finish state.
 *
 * @code
 * void CaptureSceneExample::OnCaptureFinished(Capture capture)
 * {
 *   if (capture.GetFinishState() == Capture::SUCCESSED)
 *   {
 *     // Do something
 *   }
 *   else
 *   {
 *     // Do something
 *   }
 * }
 * @endcode
 */
class DALI_IMPORT_API Capture : public BaseHandle
{
public:
  enum FinishState
  {
    SUCCESSED,
    FAILED
  };

  /**
   * @brief Typedef for signals sent by this class.
   */
  typedef Signal< void (Capture) > CaptureSignalType;

  /**
   * @brief Create an uninitialized Capture; this can be initialized with Actor::New().
   *
   * Calling member functions with an uninitialized Dali::Object is not allowed.
   */
  Capture();

  /**
   * @brief Create an initialized Capture.
   *
   * @return A handle to a newly allocated Dali resource.
   */
  static Capture New();

  /**
   * @brief Create an initialized Capture.
   *
   * @param[in] mode camera projection mode.
   * @return A handle to a newly allocated Dali resource.
   */
  static Capture New(Dali::Camera::ProjectionMode mode);

  /**
   * @brief Downcast an Object handle to Capture handle.
   *
   * If handle points to a Capture object the downcast produces valid
   * handle. If not the returned handle is left uninitialized.
   *
   * @param[in] handle to An object.
   * @return handle to a Capture object or an uninitialized handle.
   */
  static Capture DownCast( BaseHandle handle );

  /**
   * @brief Dali::Actor is intended as a base class.
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~Capture();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param[in] copy A reference to the copied handle.
   */
  Capture(const Capture& copy);

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @param[in] rhs  A reference to the copied handle.
   * @return A reference to this.
   */
  Capture& operator=(const Capture& rhs);

  /**
   * @brief Start capture and save the image as a file.
   *
   * @param[in] source source actor to be used for capture.
   * @param[in] size captured size.
   * @param[in] path image file path to be saved as a file.
   * @param[in] clearColor background color of captured scene
   */
  void Start(Actor source, const Vector2& size, const std::string &path, const Vector4& clearColor = Dali::Color::TRANSPARENT );

  /**
   * @brief Retrieve the capture finish status.
   *
   * @return Whether success or not.
   */
  FinishState GetFinishState();

  /**
   * @brief Get finished signal.
   *
   * @return finished signal instance.
   */
  CaptureSignalType& FinishedSignal();

public: // Not intended for application developers
  /**
   * @brief This constructor is used by New() methods.
   *
   * @param[in] internal A pointer to a newly allocated Dali resource.
   */
  explicit DALI_INTERNAL Capture(Internal::Capture* internal);
};

} // namespace Extension

} // namespace Dali

/**
 * @}
 */

#endif // __DALI_EXTENSION_CAPTURE_H__
