#ifndef DALI_ICU_PLUGIN_H
#define DALI_ICU_PLUGIN_H

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

// EXTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/icu-plugin.h>

namespace Dali
{
namespace Plugin
{

/**
 * @brief Implementation of the ICU class which has Tizen platform dependency.
 */
class ICU : public Dali::TextAbstraction::ICUPlugin
{
public:
  /**
   * @brief Constructor.
   */
  ICU();

  /**
   * @brief Destructor.
   */
  virtual ~ICU();

  /**
   * @copydoc Dali::TextAbstraction::ICUPlugin::UpdateLineBreakInfoByLocale()
   */
  void UpdateLineBreakInfoByLocale(const std::string&              text,
                                   TextAbstraction::Length         numberOfCharacters,
                                   const char*                     locale,
                                   TextAbstraction::LineBreakInfo* breakInfo) override;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_ICU_PLUGIN_H
