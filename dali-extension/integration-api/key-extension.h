#ifndef __DALI_KEY_EXTENSION_H__
#define __DALI_KEY_EXTENSION_H__

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/adaptor-framework/key.h>
#include <dali/devel-api/adaptor-framework/key-extension-plugin.h>

namespace Dali
{

/**
 * @brief Enumeration for mapping keyboard and mouse button event keycodes to extension codes.
 * @SINCE_1_2.43
 */
enum EXTENSION_KEY
{
};

/**
 * @brief Checks if a key event is for a specific extension DALI KEY.
 *
 * @SINCE_1_2.43
 * @param keyEvent reference to a keyEvent structure
 * @param daliKey Dali extension key enum
 * @return true if the key is matched, @c false if not
 */
DALI_IMPORT_API bool IsExtensionKey( const Dali::KeyEvent& keyEvent, Dali::EXTENSION_KEY daliKey);

namespace Plugin
{
class KeyExtension : public Dali::KeyExtensionPlugin
{

public:

  /**
   * @brief KeyExtension constructor.
   * @SINCE_1_2.43
   */
  KeyExtension();

  /**
   * @brief KeyExtension destructor.
   * @SINCE_1_2.43
   */
  virtual ~KeyExtension();

  /**
   * @brief Get extension key lookup table.
   *
   * @SINCE_1_2.43
   * @return Pointer of extension Key lookup table.
   */
  virtual Dali::KeyExtensionPlugin::KeyLookup* GetKeyLookupTable();

  /**
   * @brief Get count of extension key lookup table.
   *
   * @SINCE_1_2.43
   * @return count of extension Key lookup table.
   */
  virtual std::size_t GetKeyLookupTableCount();
};

}
}
#endif
