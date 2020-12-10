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

namespace Dali
{
namespace Plugin
{

TizenWebEngineBackForwardListItem::TizenWebEngineBackForwardListItem( Ewk_Back_Forward_List_Item* item )
  : ewkBackForwardListItem( item )
{
}

TizenWebEngineBackForwardListItem::~TizenWebEngineBackForwardListItem()
{
}

std::string TizenWebEngineBackForwardListItem::GetUrl() const
{
  return std::string( ewk_back_forward_list_item_url_get( ewkBackForwardListItem ) );
}

std::string TizenWebEngineBackForwardListItem::GetTitle() const
{
  return std::string( ewk_back_forward_list_item_title_get( ewkBackForwardListItem ) );
}

std::string TizenWebEngineBackForwardListItem::GetOriginalUrl() const
{
  return std::string( ewk_back_forward_list_item_original_url_get( ewkBackForwardListItem ) );
}

} // namespace Plugin
} // namespace Dali