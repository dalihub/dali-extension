/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

#include "tizen-web-engine-context-menu-item.h"
#include "tizen-web-engine-context-menu.h"

#include <ewk_context_menu_internal.h>
#include <ewk_context_menu_product.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineContextMenuItem::TizenWebEngineContextMenuItem(Ewk_Context_Menu_Item* item)
  : ewkContextMenuItem(item)
{
}

TizenWebEngineContextMenuItem::~TizenWebEngineContextMenuItem()
{
}

Dali::WebEngineContextMenuItem::ItemTag TizenWebEngineContextMenuItem::GetTag() const
{
  return static_cast<Dali::WebEngineContextMenuItem::ItemTag>(ewk_context_menu_item_tag_get(ewkContextMenuItem));
}

Dali::WebEngineContextMenuItem::ItemType TizenWebEngineContextMenuItem::GetType() const
{
  return static_cast<Dali::WebEngineContextMenuItem::ItemType>(ewk_context_menu_item_type_get(ewkContextMenuItem));
}

bool TizenWebEngineContextMenuItem::IsEnabled() const
{
  return ewk_context_menu_item_enabled_get(ewkContextMenuItem);
}

std::string TizenWebEngineContextMenuItem::GetLinkUrl() const
{
  return ewk_context_menu_item_link_url_get(ewkContextMenuItem);
}

std::string TizenWebEngineContextMenuItem::GetImageUrl() const
{
  return ewk_context_menu_item_image_url_get(ewkContextMenuItem);
}

std::string TizenWebEngineContextMenuItem::GetTitle() const
{
  return ewk_context_menu_item_title_get(ewkContextMenuItem);
}

std::unique_ptr<Dali::WebEngineContextMenu> TizenWebEngineContextMenuItem::GetParentMenu() const
{
  Ewk_Context_Menu* menu = ewk_context_menu_item_parent_menu_get(ewkContextMenuItem);
  std::unique_ptr<Dali::WebEngineContextMenu> webEngineContextMenu(new TizenWebEngineContextMenu(menu));
  return webEngineContextMenu;
}

Ewk_Context_Menu_Item* TizenWebEngineContextMenuItem::GetMenuItem() const
{
  return ewkContextMenuItem;
}

} // namespace Plugin
} // namespace Dali
