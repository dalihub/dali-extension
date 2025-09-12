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

#include "tizen-web-engine-back-forward-list-item.h"

namespace Dali
{
namespace Plugin
{

TizenWebEngineBackForwardListItem::TizenWebEngineBackForwardListItem(Ewk_Back_Forward_List_Item* item, bool willItemBeDeleted)
: mEwkBackForwardListItem(item),
  mWillItemBeDeleted(willItemBeDeleted)
{
}

TizenWebEngineBackForwardListItem::~TizenWebEngineBackForwardListItem()
{
  if(mWillItemBeDeleted)
  {
    ewk_back_forward_list_item_unref(mEwkBackForwardListItem);
  }
}

std::string TizenWebEngineBackForwardListItem::GetUrl() const
{
  const char* url = ewk_back_forward_list_item_url_get(mEwkBackForwardListItem);
  return url ? std::string(url) : std::string();
}

std::string TizenWebEngineBackForwardListItem::GetTitle() const
{
  const char* title = ewk_back_forward_list_item_title_get(mEwkBackForwardListItem);
  return title ? std::string(title) : std::string();
}

std::string TizenWebEngineBackForwardListItem::GetOriginalUrl() const
{
  const char* url = ewk_back_forward_list_item_original_url_get(mEwkBackForwardListItem);
  return url ? std::string(url) : std::string();
}

} // namespace Plugin
} // namespace Dali