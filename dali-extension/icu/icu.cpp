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
#include <icu.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

#include <unicode/brkiter.h>
#include <unicode/unistr.h>
#include <unicode/ustream.h>

// INTERNAL INCLUDES

// The plugin factories
extern "C" DALI_EXPORT_API Dali::TextAbstraction::ICUPlugin *CreateICUPlugin()
{
  return new Dali::Plugin::ICU();
}

extern "C" DALI_EXPORT_API void DestroyICUPlugin(Dali::TextAbstraction::ICUPlugin *plugin)
{
  if (plugin != nullptr)
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
} // unnamed namespace

ICU::ICU()
{

}

ICU::~ICU()
{

}

void ICU::UpdateLineBreakInfoByLocale(const std::string&              text,
                                      TextAbstraction::Length         numberOfCharacters,
                                      const char*                     locale,
                                      TextAbstraction::LineBreakInfo* breakInfo)
{
  icu::UnicodeString unicodeText = icu::UnicodeString::fromUTF8(text);
  if(static_cast<uint32_t>(unicodeText.length()) != numberOfCharacters)
  {
    DALI_LOG_ERROR("UnicodeString len : %lu, numberOfCharacters : %lu\n", static_cast<uint32_t>(unicodeText.length()), numberOfCharacters);
    return;
  }

  icu::Locale icuLocale(locale);

  UErrorCode status = U_ZERO_ERROR;
  std::unique_ptr<icu::BreakIterator> lineIterator(icu::BreakIterator::createLineInstance(icuLocale, status));

  if(U_FAILURE(status))
  {
    DALI_LOG_ERROR("Failed to create BreakIterator: %s\n", u_errorName(status));
    return;
  }

  lineIterator->setText(unicodeText);

  for(int32_t pos = lineIterator->first(); pos != icu::BreakIterator::DONE; pos = lineIterator->next())
  {
    if(pos != 0 && breakInfo[pos - 1] != TextAbstraction::LINE_MUST_BREAK)
    {
      breakInfo[pos - 1] = TextAbstraction::LINE_ALLOW_BREAK;
    }
  }
}

} // namespace Plugin
} // namespace Dali
