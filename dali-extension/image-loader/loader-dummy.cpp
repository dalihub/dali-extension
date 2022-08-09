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

// HEADER
#include "loader-dummy.h"

#include <dali/integration-api/debug.h>
#include <dali/public-api/images/image.h>
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
// INTERNAL INCLUDES

namespace Dali
{
namespace Plugin
{

/**
 * This code is a dummy code. You can implement it here.
 */

bool LoadImageHeader( const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height )
{
  bool success = false;
  /* Loads the header of a image file and fills in the width and height appropriately. */

  return success;
}


bool LoadBitmapFromImage( const Dali::ImageLoader::Input& input, Dali::Devel::PixelBuffer& bitmap )
{
  bool success = false;
  /* Loads the bitmap from an image file.  This function checks the header first */

  return success;
}

}
}
