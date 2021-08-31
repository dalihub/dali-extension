#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_HIT_TEST_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_HIT_TEST_H

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

// EXTERNAL INCLUDES
#include <Eina.h>
#include <Evas.h>
#include <dali/devel-api/adaptor-framework/web-engine-hit-test.h>
#include <dali/public-api/object/property-map.h>
#include <ewk_hit_test_internal.h>
#include <string>

namespace Dali
{
namespace Plugin
{

/**
 * @brief A class TizenWebEngineHitTest for hit test.
 */
class TizenWebEngineHitTest : public Dali::WebEngineHitTest
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineHitTest(Ewk_Hit_Test* hitTest, Evas* evas, bool needMemoryBeFreed);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineHitTest();

  /**
   * @copydoc Dali::WebEngineHitTest::GetResultContext()
   */
  ResultContext GetResultContext() const override;

  /**
   * @copydoc Dali::WebEngineHitTest::GetLinkUri()
   */
  std::string GetLinkUri() const override;

  /**
   * @copydoc Dali::WebEngineHitTest::GetLinkTitle()
   */
  std::string GetLinkTitle() const override;

  /**
   * @copydoc Dali::WebEngineHitTest::GetLinkLabel()
   */
  std::string GetLinkLabel() const override;

  /**
   * @copydoc Dali::WebEngineHitTest::GetImageUri()
   */
  std::string GetImageUri() const override;

  /**
   * @copydoc Dali::WebEngineHitTest::GetMediaUri()
   */
  std::string GetMediaUri() const override;

  /**
   * @copydoc Dali::WebEngineHitTest::GetTagName()
   */
  std::string GetTagName() const override;

  /**
   * @copydoc Dali::WebEngineHitTest::GetNodeValue()
   */
  std::string GetNodeValue() const override;

  /**
   * @copydoc Dali::WebEngineHitTest::GetAttributes()
   */
  Dali::Property::Map GetAttributes() const override;

  /**
   * @copydoc Dali::WebEngineHitTest::GetImageFileNameExtension()
   */
  std::string GetImageFileNameExtension() const override;

  /**
   * @copydoc Dali::WebEngineHitTest::GetImageBuffer()
   */
  Dali::PixelData GetImageBuffer() override;

private:
  /**
   * @brief Iterator attributes.
   *
   * @param[in] hash Hash map that need be iterated
   * @param[in] key Key of hash map
   * @param[in] data Value of hash map
   * @param[in] fdata User data for iterating hash map
   *
   * @return true if succeeded, false otherwise
   */
  static Eina_Bool IterateAttributes(const Eina_Hash* hash, const void* key, void* data, void* fdata);

private:
  Ewk_Hit_Test*               ewkHitTest;
  Evas*                       canvas;
  bool                        isMemoryFreedByEwk;
  mutable Dali::Property::Map attributes;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_HIT_TEST_H
