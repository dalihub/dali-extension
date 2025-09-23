#ifndef DALI_TIZEN_IMAGE_LOADER_PLUGIN_H
#define DALI_TIZEN_IMAGE_LOADER_PLUGIN_H

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/image-loader-plugin.h>
#include <dali/integration-api/debug.h>

namespace Dali
{

namespace Plugin
{

/**
 * image loading plugin for image loader.
 */
class TizenImageLoader : public Dali::ImageLoaderPlugin
{
public: // Construction & Destruction
  /**
   * Constructor
   */
  TizenImageLoader();

  /**
   * Destructor
   */
  virtual ~TizenImageLoader();

public: // ImageLoaderPlugin overrides
  /**
   * @copydoc Dali::Plugin::TizenImageLoader::BitmapLoaderLookup()
   */
  virtual const Dali::ImageLoader::BitmapLoader* BitmapLoaderLookup(const std::string& filename) const;
};

} // namespace Plugin

} // namespace Dali

#endif // DALI_TIZEN_IMAGE_LOADER_PLUGIN_H
