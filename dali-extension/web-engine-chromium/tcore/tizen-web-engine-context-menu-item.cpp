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

#include "tizen-web-engine-context-menu-item.h"
#include "tizen-web-engine-context-menu.h"

#include <dali/integration-api/debug.h>

#include <wv_context_menu_internal.h>
#include <wv_context_menu_product.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineContextMenuItem::TizenWebEngineContextMenuItem(wv_context_menu_item_h item)
: wvContextMenuItem(item)
{
}

TizenWebEngineContextMenuItem::~TizenWebEngineContextMenuItem()
{
}

Dali::WebEngineContextMenuItem::ItemTag TizenWebEngineContextMenuItem::GetTag() const
{
  return static_cast<Dali::WebEngineContextMenuItem::ItemTag>(wv_context_menu_item_tag_get(wvContextMenuItem));
}

Dali::WebEngineContextMenuItem::ItemType TizenWebEngineContextMenuItem::GetType() const
{
  return static_cast<Dali::WebEngineContextMenuItem::ItemType>(wv_context_menu_item_type_get(wvContextMenuItem));
}

bool TizenWebEngineContextMenuItem::IsEnabled() const
{
  return wv_context_menu_item_enabled_get(wvContextMenuItem);
}

std::string TizenWebEngineContextMenuItem::GetLinkUrl() const
{
  const char* url = wv_context_menu_item_link_url_get(wvContextMenuItem);
  return url ? std::string(url) : std::string();
}

std::string TizenWebEngineContextMenuItem::GetImageUrl() const
{
  const char* url = wv_context_menu_item_image_url_get(wvContextMenuItem);
  return url ? std::string(url) : std::string();
}

std::string TizenWebEngineContextMenuItem::GetTitle() const
{
  const char* title = wv_context_menu_item_title_get(wvContextMenuItem);
  return title ? std::string(title) : std::string();
}

std::unique_ptr<Dali::WebEngineContextMenu> TizenWebEngineContextMenuItem::GetParentMenu() const
{
  // WV GAP (WV_REQUIREMENTS.md B): wv_context_menu_item_parent_menu_get() is
  // not declared by the target WV headers, so the parent menu is unreachable.
  return nullptr;
}

wv_context_menu_item_h TizenWebEngineContextMenuItem::GetMenuItem() const
{
  return wvContextMenuItem;
}

} // namespace Plugin
} // namespace Dali
