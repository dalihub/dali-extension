#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_CONTEXT_MENU_ITEM_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_CONTEXT_MENU_ITEM_H

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

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/web-engine-context-menu.h>
#include <dali/devel-api/adaptor-framework/web-engine-context-menu-item.h>
#include <ewk_context_menu.h>

namespace Dali
{
// forward declaration.
class WebEngineContextMenu;

namespace Plugin
{
/**
 * @brief A class TizenWebEngineContextMenuItem for item of context menu.
 */
class TizenWebEngineContextMenuItem : public Dali::WebEngineContextMenuItem
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineContextMenuItem(Ewk_Context_Menu_Item*);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineContextMenuItem();

  /**
   * @copydoc Dali::WebEngineContextMenuItem::GetTag()
   */
  ItemTag GetTag() const override;

  /**
   * @copydoc Dali::WebEngineContextMenuItem::GetType()
   */
  ItemType GetType() const override;

  /**
   * @copydoc Dali::WebEngineContextMenuItem::IsEnabled()
   */
  bool IsEnabled() const override;

  /**
   * @copydoc Dali::WebEngineContextMenuItem::GetLinkUrl()
   */
  std::string GetLinkUrl() const override;

  /**
   * @copydoc Dali::WebEngineContextMenuItem::GetImageUrl()
   */
  std::string GetImageUrl() const override;

  /**
   * @copydoc Dali::WebEngineContextMenuItem::GetTitle()
   */
  std::string GetTitle() const override;

  /**
   * @copydoc Dali::WebEngineContextMenuItem::GetParentMenu()
   */
  std::unique_ptr<Dali::WebEngineContextMenu> GetParentMenu() const override;

  /**
   * @brief Gets menu item of web engine.
   *
   * @return menu item of web engine
   */
  Ewk_Context_Menu_Item* GetMenuItem() const;

private:
  Ewk_Context_Menu_Item* ewkContextMenuItem;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_CONTEXT_MENU_ITEM_H
