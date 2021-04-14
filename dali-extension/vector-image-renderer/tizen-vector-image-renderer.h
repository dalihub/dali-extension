#ifndef DALI_TIZEN_VECTOR_IMAGE_RENDERER_PLUGIN_H
#define DALI_TIZEN_VECTOR_IMAGE_RENDERER_PLUGIN_H

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/vector-image-renderer-plugin.h>
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <memory>
#include <thorvg.h>

namespace Dali
{

namespace Plugin
{

/**
 * @brief Implementation of the Tizen vector image renderer class which has Tizen platform dependency.
 */
class TizenVectorImageRenderer : public Dali::VectorImageRendererPlugin
{
public:

  /**
   * @brief Constructor.
   */
  TizenVectorImageRenderer();

  /**
   * @brief Destructor.
   */
  virtual ~TizenVectorImageRenderer();

  /**
   * @copydoc Dali::VectorImageRendererPlugin::Load()
   */
  bool Load(const Vector<uint8_t>& data) override;

  /**
   * @copydoc Dali::VectorImageRendererPlugin::Render()
   */
  bool Rasterize(Dali::Devel::PixelBuffer& buffer) override;

  /**
   * @copydoc Dali::VectorImageRendererPlugin::GetDefaultSize()
   */
  void GetDefaultSize(uint32_t& width, uint32_t& height) const override;

private:

  std::unique_ptr< tvg::SwCanvas >       mSwCanvas;
  tvg::Picture*                          mPicture;        ///< The pointer to the picture

  uint32_t                               mDefaultWidth;   ///< The width of the surface
  uint32_t                               mDefaultHeight;  ///< The height of the surface
  bool                                   mIsFirstRender;
};

} // namespace Plugin

} // namespace Dali;

#endif // DALI_TIZEN_VECTOR_IMAGE_RENDERER_PLUGIN_H
