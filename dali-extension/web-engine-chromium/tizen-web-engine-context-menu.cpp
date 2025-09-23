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

#include <ewk_context_menu_product.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineContextMenu::TizenWebEngineContextMenu(Ewk_Context_Menu* menu)
: ewkContextMenu(menu)
{
}

TizenWebEngineContextMenu::~TizenWebEngineContextMenu()
{
}

uint32_t TizenWebEngineContextMenu::GetItemCount() const
{
  return ewk_context_menu_item_count(ewkContextMenu);
}

std::unique_ptr<Dali::WebEngineContextMenuItem> TizenWebEngineContextMenu::GetItemAt(uint32_t index) const
{
  Ewk_Context_Menu_Item*                          item = ewk_context_menu_nth_item_get(ewkContextMenu, index);
  std::unique_ptr<Dali::WebEngineContextMenuItem> contextMenuItem(new TizenWebEngineContextMenuItem(item));
  return contextMenuItem;
}

std::vector<std::unique_ptr<Dali::WebEngineContextMenuItem>> TizenWebEngineContextMenu::GetItemList() const
{
  std::vector<std::unique_ptr<Dali::WebEngineContextMenuItem>> contextMenuItemList;
  Eina_List*                                                   itemList = const_cast<Eina_List*>(ewk_context_menu_items_get(ewkContextMenu));
  Eina_List*                                                   list     = nullptr;
  void*                                                        item     = nullptr;
  EINA_LIST_FOREACH(itemList, list, item)
  {
    if(item)
    {
      Ewk_Context_Menu_Item*                          menuItem = static_cast<Ewk_Context_Menu_Item*>(item);
      std::unique_ptr<Dali::WebEngineContextMenuItem> webitem(new TizenWebEngineContextMenuItem(menuItem));
      contextMenuItemList.push_back(std::move(webitem));
    }
  }
  return contextMenuItemList;
}

bool TizenWebEngineContextMenu::RemoveItem(Dali::WebEngineContextMenuItem& item)
{
  TizenWebEngineContextMenuItem* menuItem = static_cast<TizenWebEngineContextMenuItem*>(&item);
  return ewk_context_menu_item_remove(ewkContextMenu, menuItem->GetMenuItem());
}

bool TizenWebEngineContextMenu::AppendItemAsAction(Dali::WebEngineContextMenuItem::ItemTag tag, const std::string& title, bool enabled)
{
  return ewk_context_menu_item_append_as_action(ewkContextMenu, (Ewk_Context_Menu_Item_Tag)tag, title.c_str(), enabled);
}

bool TizenWebEngineContextMenu::AppendItem(Dali::WebEngineContextMenuItem::ItemTag tag, const std::string& title, const std::string& iconFile, bool enabled)
{
  return ewk_context_menu_item_append(ewkContextMenu, (Ewk_Context_Menu_Item_Tag)tag, title.c_str(), iconFile.c_str(), enabled);
}

bool TizenWebEngineContextMenu::SelectItem(Dali::WebEngineContextMenuItem& item)
{
  TizenWebEngineContextMenuItem* menuItem = static_cast<TizenWebEngineContextMenuItem*>(&item);
  return ewk_context_menu_item_select(ewkContextMenu, menuItem->GetMenuItem());
}

bool TizenWebEngineContextMenu::Hide()
{
  return ewk_context_menu_hide(ewkContextMenu);
}

} // namespace Plugin
} // namespace Dali
