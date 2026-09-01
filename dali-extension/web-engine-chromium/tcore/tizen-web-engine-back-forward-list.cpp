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

#include "tizen-web-engine-back-forward-list.h"
#include "tizen-web-engine-back-forward-list-item.h"

#include <glib.h>
namespace Dali
{
namespace Plugin
{

TizenWebEngineBackForwardList::TizenWebEngineBackForwardList(wv_back_forward_list_h list)
: mWvBackForwardList(list)
{
}

TizenWebEngineBackForwardList::~TizenWebEngineBackForwardList()
{
}

std::unique_ptr<Dali::WebEngineBackForwardListItem> TizenWebEngineBackForwardList::GetCurrentItem() const
{
  wv_back_forward_list_item_h item    = wv_back_forward_list_current_item_get(mWvBackForwardList);
  Dali::WebEngineBackForwardListItem*                 webitem = new TizenWebEngineBackForwardListItem(item);
  std::unique_ptr<Dali::WebEngineBackForwardListItem> ret(webitem);
  return ret;
}

std::unique_ptr<Dali::WebEngineBackForwardListItem> TizenWebEngineBackForwardList::GetPreviousItem() const
{
  wv_back_forward_list_item_h item    = wv_back_forward_list_previous_item_get(mWvBackForwardList);
  Dali::WebEngineBackForwardListItem*                 webitem = new TizenWebEngineBackForwardListItem(item);
  std::unique_ptr<Dali::WebEngineBackForwardListItem> ret(webitem);
  return ret;
}

std::unique_ptr<Dali::WebEngineBackForwardListItem> TizenWebEngineBackForwardList::GetNextItem() const
{
  wv_back_forward_list_item_h item    = wv_back_forward_list_next_item_get(mWvBackForwardList);
  Dali::WebEngineBackForwardListItem*                 webitem = new TizenWebEngineBackForwardListItem(item);
  std::unique_ptr<Dali::WebEngineBackForwardListItem> ret(webitem);
  return ret;
}

std::unique_ptr<Dali::WebEngineBackForwardListItem> TizenWebEngineBackForwardList::GetItemAtIndex(uint32_t index) const
{
  wv_back_forward_list_item_h item    = wv_back_forward_list_item_at_index_get(mWvBackForwardList, index);
  Dali::WebEngineBackForwardListItem*                 webitem = new TizenWebEngineBackForwardListItem(item);
  std::unique_ptr<Dali::WebEngineBackForwardListItem> ret(webitem);
  return ret;
}

uint32_t TizenWebEngineBackForwardList::GetItemCount() const
{
  return wv_back_forward_list_count(mWvBackForwardList);
}

std::vector<std::unique_ptr<Dali::WebEngineBackForwardListItem>> TizenWebEngineBackForwardList::GetBackwardItems(int limit)
{
  std::vector<std::unique_ptr<Dali::WebEngineBackForwardListItem>> ret;

  GList* list = wv_back_forward_list_n_back_items_copy(mWvBackForwardList, limit);
  for(GList* it = list; it != nullptr; it = it->next)
  {
    Dali::WebEngineBackForwardListItem*                 webitem = new TizenWebEngineBackForwardListItem(static_cast<wv_back_forward_list_item_h>(it->data), true);
    std::unique_ptr<Dali::WebEngineBackForwardListItem> item(webitem);
    ret.push_back(std::move(item));
  }
  g_list_free(list);

  return ret;
}

std::vector<std::unique_ptr<Dali::WebEngineBackForwardListItem>> TizenWebEngineBackForwardList::GetForwardItems(int limit)
{
  std::vector<std::unique_ptr<Dali::WebEngineBackForwardListItem>> ret;

  GList* list = wv_back_forward_list_n_forward_items_copy(mWvBackForwardList, limit);
  for(GList* it = list; it != nullptr; it = it->next)
  {
    Dali::WebEngineBackForwardListItem*                 webitem = new TizenWebEngineBackForwardListItem(static_cast<wv_back_forward_list_item_h>(it->data), true);
    std::unique_ptr<Dali::WebEngineBackForwardListItem> item(webitem);
    ret.push_back(std::move(item));
  }
  g_list_free(list);

  return ret;
}

} // namespace Plugin
} // namespace Dali
