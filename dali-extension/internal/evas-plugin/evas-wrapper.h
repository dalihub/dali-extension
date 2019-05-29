#ifndef __DALI_EXTENSION_INTERNAL_EVAS_WRAPPER__
#define __DALI_EXTENSION_INTERNAL_EVAS_WRAPPER__

/*
 * Copyright ( c ) 2019 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 ( the "License" );
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
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_Wl2.h>
#include <tbm_surface.h>

#include <dali/public-api/math/rect.h>

// INTERNAL INCLUDES

namespace Dali
{

typedef Rect<int> PositionSize;

namespace Extension
{

namespace Internal
{

/**
 * EvasWrapper is a class which holds a parent evas object for the EvasPlugin.
 * This creates essential evas ebjects (RenderTarget, AccessibiltyTarget, FocusTarget) to connect evas to Dali properly,
 * and also provides evas rendering interfaces.
 */
class EvasWrapper
{

public:

  /**
   * @brief Constructor
   * @param[in] pluginParent An parent evas object of a evas plugin
   * @param[in] width The initial width of the Dali view port
   * @param[in] height The initial height of the Dali view port
   * @param[in] transparent Whether the Evas object is transparent or not
   */
  EvasWrapper( Evas_Object* pluginParent, int width, int height, bool transparent );

  /**
   * @brief Destructor
   */
  ~EvasWrapper();

  /**
   * @brief Gets current native window object
   * @return The ecore window object
   */
  Ecore_Wl2_Window* GetNativeWindow() const;

  /**
   * @brief Gets the geometry information
   * @return The geometry information
   */
  PositionSize GetGeometry() const;

  /**
   * @brief Gets the rendering target evas object
   * @return The rendering target object
   */
  Evas_Object* GetRenderTarget() const;

  /**
   * @brief Gets the accessibility target evas object
   * @return The accessibility target object
   */
  Evas_Object* GetAccessibilityTarget() const;

  /**
   * @brief Gets the focus target evas object
   * @return The focus target object
   */
  Evas_Object* GetFocusTarget() const;

  /**
   * @brief Sets focus
   */
  void SetFocus();

  /**
   * @brief Bind a tbm surface
   */
  void BindTBMSurface( tbm_surface_h surface );

  /**
   * @brief Request rendering
   */
  void RequestRender();

private:

  Ecore_Evas*  mEcoreEvas;
  Evas_Object* mImageEvasObject;
  Evas_Object* mAccessibilityEvasObject;
  Evas_Object* mFocusEvasObject;
};

}  // namespace Internal

}  // namespace Extension

}  // namespace Dali

#endif // __DALI_EXTENSION_INTERNAL_EVAS_WRAPPER__
