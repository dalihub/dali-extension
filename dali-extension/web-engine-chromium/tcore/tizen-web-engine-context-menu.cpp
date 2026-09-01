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

#include "tizen-web-engine-context-menu.h"
#include "tizen-web-engine-context-menu-item.h"

#include <glib.h>
#include <wv_context_menu_product.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineContextMenu::TizenWebEngineContextMenu(wv_context_menu_h menu)
: wvContextMenu(menu)
{
}

TizenWebEngineContextMenu::~TizenWebEngineContextMenu()
{
}

uint32_t TizenWebEngineContextMenu::GetItemCount() const
{
  return wv_context_menu_item_count(wvContextMenu);
}

std::unique_ptr<Dali::WebEngineContextMenuItem> TizenWebEngineContextMenu::GetItemAt(uint32_t index) const
{
  wv_context_menu_item_h item = wv_context_menu_nth_item_get(wvContextMenu, index);
  std::unique_ptr<Dali::WebEngineContextMenuItem> contextMenuItem(new TizenWebEngineContextMenuItem(item));
  return contextMenuItem;
}

std::vector<std::unique_ptr<Dali::WebEngineContextMenuItem>> TizenWebEngineContextMenu::GetItemList() const
{
  std::vector<std::unique_ptr<Dali::WebEngineContextMenuItem>> contextMenuItemList;
  GList*                                                       itemList = wv_context_menu_items_get(wvContextMenu);
  for(GList* it = itemList; it != nullptr; it = it->next)
  {
    if(it->data)
    {
      wv_context_menu_item_h menuItem = static_cast<wv_context_menu_item_h>(it->data);
      std::unique_ptr<Dali::WebEngineContextMenuItem> webitem(new TizenWebEngineContextMenuItem(menuItem));
      contextMenuItemList.push_back(std::move(webitem));
    }
  }
  return contextMenuItemList;
}

bool TizenWebEngineContextMenu::RemoveItem(Dali::WebEngineContextMenuItem& item)
{
  TizenWebEngineContextMenuItem* menuItem = static_cast<TizenWebEngineContextMenuItem*>(&item);
  return wv_context_menu_item_remove(wvContextMenu, menuItem->GetMenuItem());
}

bool TizenWebEngineContextMenu::AppendItemAsAction(Dali::WebEngineContextMenuItem::ItemTag tag, const std::string& title, bool enabled)
{
  return wv_context_menu_item_append_as_action(wvContextMenu, (wv_context_menu_item_tag_e)tag, title.c_str(), enabled);
}

bool TizenWebEngineContextMenu::AppendItem(Dali::WebEngineContextMenuItem::ItemTag tag, const std::string& title, const std::string& iconFile, bool enabled)
{
  return wv_context_menu_item_append(wvContextMenu, (wv_context_menu_item_tag_e)tag, title.c_str(), iconFile.c_str(), enabled);
}

bool TizenWebEngineContextMenu::SelectItem(Dali::WebEngineContextMenuItem& item)
{
  TizenWebEngineContextMenuItem* menuItem = static_cast<TizenWebEngineContextMenuItem*>(&item);
  return wv_context_menu_item_select(wvContextMenu, menuItem->GetMenuItem());
}

bool TizenWebEngineContextMenu::Hide()
{
  return wv_context_menu_hide(wvContextMenu);
}

} // namespace Plugin
} // namespace Dali
