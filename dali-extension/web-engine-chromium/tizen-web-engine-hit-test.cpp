/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

#include "tizen-web-engine-hit-test.h"

#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineHitTest::TizenWebEngineHitTest(Ewk_Hit_Test* test, Evas* evas, bool needMemoryBeFreed)
  : ewkHitTest(test)
  , canvas(evas)
  , isMemoryFreedByEwk(needMemoryBeFreed)
{
}

TizenWebEngineHitTest::~TizenWebEngineHitTest()
{
  if (isMemoryFreedByEwk)
  {
    ewk_hit_test_free(ewkHitTest);
  }
}

Dali::WebEngineHitTest::ResultContext TizenWebEngineHitTest::GetResultContext() const
{
  return static_cast<Dali::WebEngineHitTest::ResultContext>(ewk_hit_test_result_context_get(ewkHitTest));
}

std::string TizenWebEngineHitTest::GetLinkUri() const
{
  const char* uri = ewk_hit_test_link_uri_get(ewkHitTest);
  return uri ? std::string(uri) : std::string();
}

std::string TizenWebEngineHitTest::GetLinkTitle() const
{
  const char* title = ewk_hit_test_link_title_get(ewkHitTest);
  return title ? std::string(title) : std::string();
}

std::string TizenWebEngineHitTest::GetLinkLabel() const
{
  const char* label = ewk_hit_test_link_label_get(ewkHitTest);
  return label ? std::string(label) : std::string();
}

std::string TizenWebEngineHitTest::GetImageUri() const
{
  const char* uri = ewk_hit_test_image_uri_get(ewkHitTest);
  return uri ? std::string(uri) : std::string();
}

std::string TizenWebEngineHitTest::GetMediaUri() const
{
  const char* uri = ewk_hit_test_media_uri_get(ewkHitTest);
  return uri ? std::string(uri) : std::string();
}

std::string TizenWebEngineHitTest::GetTagName() const
{
  const char* name = ewk_hit_test_tag_name_get(ewkHitTest);
  return name ? std::string(name) : std::string();
}

std::string TizenWebEngineHitTest::GetNodeValue() const
{
  const char* value = ewk_hit_test_node_value_get(ewkHitTest);
  return value ? std::string(value) : std::string();
}

Dali::Property::Map TizenWebEngineHitTest::GetAttributes() const
{
  Eina_Hash* hash = ewk_hit_test_attribute_hash_get(ewkHitTest);
  attributes.Clear();
  eina_hash_foreach(hash, &TizenWebEngineHitTest::IterateAttributes, this);
  return attributes;
}

std::string TizenWebEngineHitTest::GetImageFileNameExtension() const
{
  const char* extension = ewk_hit_test_image_file_name_extension_get(ewkHitTest);
  return extension ? std::string(extension) : std::string();
}

Dali::PixelData TizenWebEngineHitTest::GetImageBuffer()
{
  Evas_Object* image = evas_object_image_add(canvas);
  evas_object_image_colorspace_set(image, EVAS_COLORSPACE_ARGB8888);
  evas_object_image_alpha_set(image, EINA_TRUE);
  evas_object_image_data_copy_set(image, ewk_hit_test_image_buffer_get(ewkHitTest));

  // color-space is argb8888.
  uint8_t* pixelBuffer = (uint8_t*)evas_object_image_data_get(image, false);
  if (!pixelBuffer)
  {
    return Dali::PixelData();
  }

  // get width/height.
  int width = 0, height = 0;
  evas_object_image_size_get(image, &width, &height);

  uint32_t bufferSize = width * height * 4;

  if (bufferSize != ewk_hit_test_image_buffer_length_get(ewkHitTest))
  {
    DALI_LOG_RELEASE_INFO("size of hit test image is not correct.\n");
  }

  uint8_t* convertedBuffer = new uint8_t[bufferSize];

  // convert the color-space to rgba8888.
  for (uint32_t i = 0; i < bufferSize; i += 4)
  {
    convertedBuffer[i] = pixelBuffer[i + 1];
    convertedBuffer[i + 1] = pixelBuffer[i + 2];
    convertedBuffer[i + 2] = pixelBuffer[i + 3];
    convertedBuffer[i + 3] = pixelBuffer[i];
  }

  return Dali::PixelData::New(convertedBuffer, bufferSize, width, height,
                              Dali::Pixel::Format::RGBA8888,
                              Dali::PixelData::ReleaseFunction::DELETE_ARRAY);
}

Eina_Bool TizenWebEngineHitTest::IterateAttributes(const Eina_Hash*, const void* key, void* data, void* fdata)
{
  TizenWebEngineHitTest* pThis = (TizenWebEngineHitTest*)fdata;
  pThis->attributes.Insert((const char*)key, (char*)data);
  return true;
}

} // namespace Plugin
} // namespace Dali
