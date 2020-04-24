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
#include <dali/public-api/math/rect.h>
#include <Elementary.h>

// CLASS HEADER
#include <dali-extension/internal/evas-plugin/evas-wrapper.h>


namespace Dali
{

namespace Extension
{

namespace Internal
{

namespace
{

const char* IMAGE_EVAS_OBJECT_NAME = "dali-evas-plugin";
const char* ELM_OBJECT_STYLE = "transparent";
const char* ELM_OBJECT_CONTAINER_PART_NAME = "elm.swallow.content";

} // unnamed namespace

EvasWrapper::EvasWrapper( Evas_Object* pluginParent, uint16_t width, uint16_t height, bool transparent )
: mEcoreEvas( nullptr ),
  mImageEvasObject( nullptr ),
  mAccessibilityEvasObject( nullptr ),
  mFocusEvasObject( nullptr )
{
  int intWidth = static_cast<int>( width );
  int intHeight = static_cast<int>( height );

  Evas* evas = evas_object_evas_get( pluginParent );
  mEcoreEvas = ecore_evas_ecore_evas_get( evas );

  // Create the image evas object
  mImageEvasObject = evas_object_image_filled_add( evas );
  evas_object_name_set( mImageEvasObject, IMAGE_EVAS_OBJECT_NAME );
  evas_object_image_content_hint_set(mImageEvasObject, EVAS_IMAGE_CONTENT_HINT_DYNAMIC);
  evas_object_image_alpha_set( mImageEvasObject, transparent ? EINA_TRUE : EINA_FALSE );
  evas_object_image_size_set( mImageEvasObject, intWidth, intHeight );
  evas_object_size_hint_weight_set( mImageEvasObject, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND );
  evas_object_size_hint_align_set( mImageEvasObject, EVAS_HINT_FILL, EVAS_HINT_FILL );

  // Create a button and set style as "focus", if does not want to show the focus, then "transparent"
  mFocusEvasObject = elm_button_add( pluginParent );

  // Don't need to show the focus boundary here
  elm_object_style_set( mFocusEvasObject, ELM_OBJECT_STYLE );

  // Set the image evas object to focus object, but event should not be propagated
  elm_object_part_content_set( mFocusEvasObject, ELM_OBJECT_CONTAINER_PART_NAME, mImageEvasObject );
  evas_object_propagate_events_set( mImageEvasObject, EINA_FALSE );

  // Set the evas object you want to make focusable as the content of the swallow part
  evas_object_size_hint_weight_set( mFocusEvasObject, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND );
  evas_object_size_hint_align_set( mFocusEvasObject, EVAS_HINT_FILL, EVAS_HINT_FILL );

  evas_object_move( mFocusEvasObject, 0, 0 );
  evas_object_resize( mFocusEvasObject, intWidth, intHeight );
  evas_object_show( mFocusEvasObject );

  // Register the elm access to image evas object
  mAccessibilityEvasObject = elm_access_object_register( mImageEvasObject, pluginParent );
}

EvasWrapper::~EvasWrapper()
{
  // Delete the elm focus evas object
  evas_object_del( mFocusEvasObject );
  mFocusEvasObject = NULL;

  // Unregister elm_access_object
  elm_access_object_unregister( mAccessibilityEvasObject );
  mAccessibilityEvasObject = NULL;

  // Delete the image evas object
  evas_object_del( mImageEvasObject );
  mImageEvasObject = NULL;
}

Ecore_Wl2_Window* EvasWrapper::GetNativeWindow() const
{
  return ecore_evas_wayland2_window_get( mEcoreEvas );
}

PositionSize EvasWrapper::GetGeometry() const
{
  PositionSize geometry;
  evas_object_geometry_get( mImageEvasObject, &geometry.x, &geometry.y, &geometry.width, &geometry.height );
  return geometry;
}

Evas_Object* EvasWrapper::GetRenderTarget() const
{
  return mImageEvasObject;
}

Evas_Object* EvasWrapper::GetAccessibilityTarget() const
{
  return mAccessibilityEvasObject;
}

Evas_Object* EvasWrapper::GetFocusTarget() const
{
  return mFocusEvasObject;
}

void EvasWrapper::SetFocus()
{
  evas_object_focus_set( mImageEvasObject, EINA_TRUE );
}

void EvasWrapper::BindTBMSurface( tbm_surface_h surface )
{
  Evas_Native_Surface nativeSurface;
  nativeSurface.type = EVAS_NATIVE_SURFACE_TBM;
  nativeSurface.version = EVAS_NATIVE_SURFACE_VERSION;
  nativeSurface.data.tbm.buffer = surface;
  nativeSurface.data.tbm.rot = 0;
  nativeSurface.data.tbm.ratio = 0;
  nativeSurface.data.tbm.flip = 0;

  evas_object_image_native_surface_set( mImageEvasObject, &nativeSurface );
}

void EvasWrapper::RequestRender()
{
  evas_object_image_pixels_dirty_set( mImageEvasObject, EINA_TRUE );
  ecore_evas_manual_render( mEcoreEvas );
}

}  // namespace Internal

}  // namespace Extension

}  // namespace Dali