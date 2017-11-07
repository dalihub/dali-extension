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

// CLASS HEADER
#include <integration-api/key-extension.h>


// The plugin factories
extern "C" DALI_EXPORT_API Dali::KeyExtensionPlugin* CreateKeyExtensionPlugin( void )
{
  return new Dali::Plugin::KeyExtension;
}

extern "C" DALI_EXPORT_API void DestroyKeyExtensionPlugin( Dali::KeyExtensionPlugin* plugin )
{
  if( plugin != NULL )
  {
    delete plugin;
  }
}

namespace Dali
{

KeyExtensionPlugin::KeyLookup mKeyLookupTable[]=
{
  // more than one key name can be assigned to a single key code
};

bool IsExtensionKey( const Dali::KeyEvent& keyEvent, Dali::EXTENSION_KEY daliKey)
{
  int key = -200000;

  for( size_t i = 0 ; i < sizeof(mKeyLookupTable) / sizeof(KeyExtensionPlugin::KeyLookup) ; i++ )
  {
    if( !keyEvent.keyPressedName.compare( mKeyLookupTable[i].keyName ) )
    {
      key = mKeyLookupTable[i].daliKeyCode;
      break;
    }
  }

  return daliKey == key;
}

namespace Plugin
{

KeyExtension::KeyExtension()
{
}

KeyExtension::~KeyExtension()
{
}

Dali::KeyExtensionPlugin::KeyLookup* KeyExtension::GetKeyLookupTable()
{
  return mKeyLookupTable;
}

std::size_t KeyExtension::GetKeyLookupTableCount()
{
  return sizeof(mKeyLookupTable) / sizeof(KeyExtensionPlugin::KeyLookup);
}

}
}
