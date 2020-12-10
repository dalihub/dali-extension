/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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

#include "tizen-web-engine-back-forward-list-item.h"
#include "tizen-web-engine-back-forward-list.h"

namespace Dali
{
namespace Plugin
{

TizenWebEngineBackForwardList::TizenWebEngineBackForwardList( Ewk_Back_Forward_List* list )
  : ewkBackForwardList( list )
{
}

TizenWebEngineBackForwardList::~TizenWebEngineBackForwardList()
{
}

Dali::WebEngineBackForwardListItem& TizenWebEngineBackForwardList::GetCurrentItem() const
{
  static TizenWebEngineBackForwardListItem itemCache( 0 );
  Ewk_Back_Forward_List_Item* item = ewk_back_forward_list_current_item_get( ewkBackForwardList );
  itemCache = TizenWebEngineBackForwardListItem( item );
  return itemCache;
}

Dali::WebEngineBackForwardListItem& TizenWebEngineBackForwardList::GetItemAtIndex( uint32_t index ) const
{
  static TizenWebEngineBackForwardListItem itemCache( 0 );
  Ewk_Back_Forward_List_Item* item = ewk_back_forward_list_item_at_index_get( ewkBackForwardList, index );
  itemCache = TizenWebEngineBackForwardListItem( item );
  return itemCache;
}

uint32_t TizenWebEngineBackForwardList::GetItemCount() const
{
  return ewk_back_forward_list_count( ewkBackForwardList );
}

} // namespace Plugin
} // namespace Dali