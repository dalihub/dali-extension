/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#include "web-engine-lwe-input.h"

#include <dali/public-api/events/key-event.h>

#include <cstring>

namespace Dali
{
namespace Plugin
{
namespace
{
constexpr int SHIFT_MODIFIER = 1;

LWE::KeyValue MapNamedKey(const char* keyName)
{
  if(!keyName)
  {
    return LWE::KeyValue::UnidentifiedKey;
  }

  if(std::strcmp(keyName, "Left") == 0 || std::strcmp(keyName, "KP_Left") == 0)
  {
    return LWE::KeyValue::ArrowLeftKey;
  }
  if(std::strcmp(keyName, "Right") == 0 || std::strcmp(keyName, "KP_Right") == 0)
  {
    return LWE::KeyValue::ArrowRightKey;
  }
  if(std::strcmp(keyName, "Up") == 0 || std::strcmp(keyName, "KP_Up") == 0)
  {
    return LWE::KeyValue::ArrowUpKey;
  }
  if(std::strcmp(keyName, "Down") == 0 || std::strcmp(keyName, "KP_Down") == 0)
  {
    return LWE::KeyValue::ArrowDownKey;
  }
  if(std::strcmp(keyName, "Return") == 0 || std::strcmp(keyName, "KP_Enter") == 0)
  {
    return LWE::KeyValue::EnterKey;
  }
  if(std::strcmp(keyName, "BackSpace") == 0)
  {
    return LWE::KeyValue::BackspaceKey;
  }
  if(std::strcmp(keyName, "Tab") == 0)
  {
    return LWE::KeyValue::TabKey;
  }
  if(std::strcmp(keyName, "Escape") == 0)
  {
    return LWE::KeyValue::EscapeKey;
  }
  if(std::strcmp(keyName, "Delete") == 0 || std::strcmp(keyName, "KP_Delete") == 0)
  {
    return LWE::KeyValue::DeleteKey;
  }
  if(std::strcmp(keyName, "Insert") == 0 || std::strcmp(keyName, "KP_Insert") == 0)
  {
    return LWE::KeyValue::InsertKey;
  }
  if(std::strcmp(keyName, "Home") == 0 || std::strcmp(keyName, "KP_Home") == 0)
  {
    return LWE::KeyValue::HomeKey;
  }
  if(std::strcmp(keyName, "End") == 0 || std::strcmp(keyName, "KP_End") == 0)
  {
    return LWE::KeyValue::EndKey;
  }
  if(std::strcmp(keyName, "Prior") == 0 || std::strcmp(keyName, "KP_Prior") == 0)
  {
    return LWE::KeyValue::PageUpKey;
  }
  if(std::strcmp(keyName, "Next") == 0 || std::strcmp(keyName, "KP_Next") == 0)
  {
    return LWE::KeyValue::PageDownKey;
  }
  if(std::strcmp(keyName, "space") == 0 || std::strcmp(keyName, "Space") == 0)
  {
    return LWE::KeyValue::SpaceKey;
  }

  return LWE::KeyValue::UnidentifiedKey;
}

LWE::KeyValue MapCharacterKey(const char* keyName, bool isShiftPressed)
{
  if(!keyName)
  {
    return LWE::KeyValue::UnidentifiedKey;
  }

  if(std::strcmp(keyName, "minus") == 0)
  {
    return isShiftPressed ? LWE::KeyValue::MinusMarkKey : LWE::KeyValue::UnderScoreMarkKey;
  }
  if(std::strcmp(keyName, "equal") == 0)
  {
    return isShiftPressed ? LWE::KeyValue::PlusMarkKey : LWE::KeyValue::EqualitySignKey;
  }
  if(std::strcmp(keyName, "bracketleft") == 0)
  {
    return isShiftPressed ? LWE::KeyValue::LeftCurlyBracketMarkKey : LWE::KeyValue::LeftSquareBracketKey;
  }
  if(std::strcmp(keyName, "bracketright") == 0)
  {
    return isShiftPressed ? LWE::KeyValue::RightCurlyBracketMarkKey : LWE::KeyValue::RightSquareBracketKey;
  }
  if(std::strcmp(keyName, "semicolon") == 0)
  {
    return isShiftPressed ? LWE::KeyValue::ColonMarkKey : LWE::KeyValue::SemiColonMarkKey;
  }
  if(std::strcmp(keyName, "apostrophe") == 0)
  {
    return isShiftPressed ? LWE::KeyValue::DoubleQuoteMarkKey : LWE::KeyValue::SingleQuoteMarkKey;
  }
  if(std::strcmp(keyName, "comma") == 0)
  {
    return isShiftPressed ? LWE::KeyValue::LessThanMarkKey : LWE::KeyValue::CommaMarkKey;
  }
  if(std::strcmp(keyName, "period") == 0)
  {
    return isShiftPressed ? LWE::KeyValue::GreaterThanSignKey : LWE::KeyValue::PeriodKey;
  }
  if(std::strcmp(keyName, "slash") == 0)
  {
    return isShiftPressed ? LWE::KeyValue::QuestionMarkKey : LWE::KeyValue::SlashKey;
  }

  if(std::strlen(keyName) == 1u)
  {
    const char character = keyName[0];
    if(character >= '0' && character <= '9')
    {
      if(isShiftPressed)
      {
        constexpr LWE::KeyValue shiftedDigits[] = {
          LWE::KeyValue::RightParenthesisMarkKey,
          LWE::KeyValue::ExclamationMarkKey,
          LWE::KeyValue::AtMarkKey,
          LWE::KeyValue::SharpMarkKey,
          LWE::KeyValue::DollarMarkKey,
          LWE::KeyValue::PercentMarkKey,
          LWE::KeyValue::CaretMarkKey,
          LWE::KeyValue::AmpersandMarkKey,
          LWE::KeyValue::AsteriskMarkKey,
          LWE::KeyValue::LeftParenthesisMarkKey,
        };
        return shiftedDigits[character - '0'];
      }
      return static_cast<LWE::KeyValue>(static_cast<int>(LWE::KeyValue::Digit0Key) + character - '0');
    }

    if(character >= 'a' && character <= 'z')
    {
      int keyValue = static_cast<int>(LWE::KeyValue::LowerAKey) + character - 'a';
      if(isShiftPressed)
      {
        keyValue -= ('z' - 'a');
        keyValue -= 7;
      }
      return static_cast<LWE::KeyValue>(keyValue);
    }
  }

  return LWE::KeyValue::UnidentifiedKey;
}
} // unnamed namespace

LWE::KeyValue ToLweKeyValue(const Dali::KeyEvent& keyEvent)
{
  // On Windows an arrow key can expose its Win32 virtual-key code as a
  // printable one-byte string, so named control keys must be resolved first.
  const LWE::KeyValue namedKey = MapNamedKey(keyEvent.GetKeyName().CStr());
  if(namedKey != LWE::KeyValue::UnidentifiedKey)
  {
    return namedKey;
  }

  const Dali::String& keyString = keyEvent.GetKeyString();
  if(keyString.Size() == 1u)
  {
    const auto character = static_cast<unsigned char>(keyString.CStr()[0]);
    if(character > 32u && character < 127u)
    {
      return static_cast<LWE::KeyValue>(character);
    }
  }

  // Tizen can report printable keys only by name; retain its legacy mapping
  // for punctuation, digits, and shifted letters as the final fallback.
  return MapCharacterKey(keyEvent.GetKeyName().CStr(), (keyEvent.GetKeyModifier() & SHIFT_MODIFIER) != 0);
}

} // namespace Plugin
} // namespace Dali
