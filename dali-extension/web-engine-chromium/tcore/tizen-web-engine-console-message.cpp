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

#include "tizen-web-engine-console-message.h"

namespace Dali
{
namespace Plugin
{

TizenWebEngineConsoleMessage::TizenWebEngineConsoleMessage(wv_console_message_h message)
: wvConsoleMessage(message)
{
}

TizenWebEngineConsoleMessage::~TizenWebEngineConsoleMessage()
{
}

std::string TizenWebEngineConsoleMessage::GetSource() const
{
  const char* source = wv_console_message_source_get(wvConsoleMessage);
  return source ? std::string(source) : std::string();
}

uint32_t TizenWebEngineConsoleMessage::GetLine() const
{
  return wv_console_message_line_get(wvConsoleMessage);
}

Dali::WebEngineConsoleMessage::SeverityLevel TizenWebEngineConsoleMessage::GetSeverityLevel() const
{
  return static_cast<Dali::WebEngineConsoleMessage::SeverityLevel>(wv_console_message_level_get(wvConsoleMessage));
}

std::string TizenWebEngineConsoleMessage::GetText() const
{
  const char* text = wv_console_message_text_get(wvConsoleMessage);
  return text ? std::string(text) : std::string();
}

} // namespace Plugin
} // namespace Dali
