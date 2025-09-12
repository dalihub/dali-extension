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
#include <dali/integration-api/bitmap.h>
#include <dali/integration-api/debug.h>
#include <tizen-image-loader.h>
#include "loader-dummy.h"

// The plugin factories
extern "C" DALI_EXPORT_API Dali::ImageLoaderPlugin* CreateImageLoaderPlugin(void)
{
  return new Dali::Plugin::TizenImageLoader;
}

extern "C" DALI_EXPORT_API void DestroyImageLoaderPlugin(Dali::ImageLoaderPlugin* plugin)
{
  if(plugin != NULL)
  {
    delete plugin;
  }
}

namespace Dali
{
namespace Plugin
{
namespace
{
/**
 * Enum for file formats, has to be in sync with BITMAP_LOADER_LOOKUP_TABLE
 */
enum FileFormats
{
  // Unknown file format
  FORMAT_UNKNOWN = -1,

  // formats that use magic bytes
  FORMAT_DUMMY = 0,
  FORMAT_TOTAL_COUNT
};

/**
 * A lookup table containing all the bitmap loaders with the appropriate information.
 * Has to be in sync with enum FileFormats
 */
const Dali::ImageLoader::BitmapLoader BITMAP_LOADER_LOOKUP_TABLE[FORMAT_TOTAL_COUNT] =
  {
    {0x0, 0x0, LoadBitmapFromImage, nullptr, LoadImageHeader, Dali::Integration::Bitmap::BITMAP_2D_PACKED_PIXELS},
};

struct FormatExtension
{
  const std::string extension;
  FileFormats       format;
};

const FormatExtension FORMAT_EXTENSIONS[] =
  {
    {".dummy", FORMAT_DUMMY}};

const unsigned int FORMAT_EXTENSIONS_COUNT = sizeof(FORMAT_EXTENSIONS) / sizeof(FormatExtension);

FileFormats GetFormatHint(const std::string& filename)
{
  FileFormats format = FORMAT_UNKNOWN;

  for(unsigned int i = 0; i < FORMAT_EXTENSIONS_COUNT; ++i)
  {
    unsigned int length = FORMAT_EXTENSIONS[i].extension.size();
    if((filename.size() > length) &&
       (0 == filename.compare(filename.size() - length, length, FORMAT_EXTENSIONS[i].extension)))
    {
      format = FORMAT_EXTENSIONS[i].format;
      break;
    }
  }
  return format;
}

} // namespace

TizenImageLoader::TizenImageLoader()
{
}

TizenImageLoader::~TizenImageLoader()
{
}

const Dali::ImageLoader::BitmapLoader* TizenImageLoader::BitmapLoaderLookup(const std::string& filename) const
{
  const Dali::ImageLoader::BitmapLoader* lookupPtr = BITMAP_LOADER_LOOKUP_TABLE;
  FileFormats                            format    = GetFormatHint(filename);
  if(format != FORMAT_UNKNOWN)
  {
    lookupPtr = BITMAP_LOADER_LOOKUP_TABLE + format;
    return lookupPtr;
  }
  else
  {
    return NULL;
  }
}

} // namespace Plugin

} // namespace Dali
