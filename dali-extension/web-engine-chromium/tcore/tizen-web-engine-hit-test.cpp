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

#include "tizen-web-engine-hit-test.h"

#include <dali/devel-api/adaptor-framework/image-loading-devel.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/adaptor-framework/pixel-buffer.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineHitTest::TizenWebEngineHitTest(wv_hit_test_h test, bool needMemoryBeFreed)
: wvHitTest(test),
  isMemoryFreedByWv(needMemoryBeFreed)
{
}

TizenWebEngineHitTest::~TizenWebEngineHitTest()
{
  if(isMemoryFreedByWv)
  {
    wv_hit_test_free(wvHitTest);
  }
}

Dali::WebEngineHitTest::ResultContext TizenWebEngineHitTest::GetResultContext() const
{
  return static_cast<Dali::WebEngineHitTest::ResultContext>(wv_hit_test_result_context_get(wvHitTest));
}

std::string TizenWebEngineHitTest::GetLinkUri() const
{
  const char* uri = wv_hit_test_link_uri_get(wvHitTest);
  return uri ? std::string(uri) : std::string();
}

std::string TizenWebEngineHitTest::GetLinkTitle() const
{
  const char* title = wv_hit_test_link_title_get(wvHitTest);
  return title ? std::string(title) : std::string();
}

std::string TizenWebEngineHitTest::GetLinkLabel() const
{
  const char* label = wv_hit_test_link_label_get(wvHitTest);
  return label ? std::string(label) : std::string();
}

std::string TizenWebEngineHitTest::GetImageUri() const
{
  const char* uri = wv_hit_test_image_uri_get(wvHitTest);
  return uri ? std::string(uri) : std::string();
}

std::string TizenWebEngineHitTest::GetMediaUri() const
{
  // WV GAP (WV_REQUIREMENTS.md B): wv_hit_test_media_uri_get() is not declared
  // by the target WV headers, so the media URI is reported as empty.
  return std::string();
}

std::string TizenWebEngineHitTest::GetTagName() const
{
  const char* name = wv_hit_test_tag_name_get(wvHitTest);
  return name ? std::string(name) : std::string();
}

std::string TizenWebEngineHitTest::GetNodeValue() const
{
  const char* value = wv_hit_test_node_value_get(wvHitTest);
  return value ? std::string(value) : std::string();
}

Dali::Property::Map TizenWebEngineHitTest::GetAttributes() const
{
  GHashTable* hash = wv_hit_test_attribute_hash_get(wvHitTest);
  attributes.Clear();
  if(hash)
  {
    g_hash_table_foreach(hash, &TizenWebEngineHitTest::IterateAttributes, const_cast<TizenWebEngineHitTest*>(this));
  }
  return attributes;
}

std::string TizenWebEngineHitTest::GetImageFileNameExtension() const
{
  const char* extension = wv_hit_test_image_file_name_extension_get(wvHitTest);
  return extension ? std::string(extension) : std::string();
}

Dali::PixelData TizenWebEngineHitTest::GetImageBuffer()
{
  // The buffer holds an encoded image (its container is named by
  // wv_hit_test_image_file_name_extension_get()), so decode it directly rather
  // than pushing it through a canvas-backed image object.
  auto*        buffer = static_cast<uint8_t*>(wv_hit_test_image_buffer_get(wvHitTest));
  unsigned int length = wv_hit_test_image_buffer_length_get(wvHitTest);
  if(!buffer || length == 0)
  {
    return Dali::PixelData();
  }

  Dali::PixelBuffer pixelBuffer = Dali::LoadImageFromBuffer(buffer, length);
  if(!pixelBuffer)
  {
    DALI_LOG_ERROR("failed to decode hit test image (%u bytes)\n", length);
    return Dali::PixelData();
  }

  return Dali::PixelBuffer::Convert(pixelBuffer);
}

void TizenWebEngineHitTest::IterateAttributes(gpointer key, gpointer value, gpointer userData)
{
  TizenWebEngineHitTest* pThis = static_cast<TizenWebEngineHitTest*>(userData);
  pThis->attributes.Insert(static_cast<const char*>(key), static_cast<char*>(value));
}

} // namespace Plugin
} // namespace Dali
