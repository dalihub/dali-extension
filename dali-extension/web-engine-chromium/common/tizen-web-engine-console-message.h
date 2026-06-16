#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_CONSOLE_MESSAGE_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_CONSOLE_MESSAGE_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-console-message.h>
#include <ewk_console_message_internal.h>
#include <string>

namespace Dali
{
namespace Plugin
{
/**
 * @brief A class TizenWebEngineConsoleMessage for console message.
 */
class TizenWebEngineConsoleMessage : public Dali::WebEngineConsoleMessage
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineConsoleMessage(Ewk_Console_Message*);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineConsoleMessage();

  /**
   * @copydoc Dali::WebEngineConsoleMessage::GetSource()
   */
  std::string GetSource() const override;

  /**
   * @copydoc Dali::WebEngineConsoleMessage::GetLine()
   */
  uint32_t GetLine() const override;

  /**
   * @copydoc Dali::WebEngineConsoleMessage::GetSeverityLevel()
   */
  SeverityLevel GetSeverityLevel() const override;

  /**
   * @copydoc Dali::WebEngineConsoleMessage::GetText()
   */
  std::string GetText() const override;

private:
  Ewk_Console_Message* ewkConsoleMessage;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_CONSOLE_MESSAGE_H
