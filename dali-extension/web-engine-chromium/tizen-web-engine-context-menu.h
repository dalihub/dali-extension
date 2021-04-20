#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_CONTEXT_MENU_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_CONTEXT_MENU_H

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
#include <dali/public-api/math/vector2.h>
#include <ewk_context_menu.h>
#include <memory>
#include <string>
#include <vector>

namespace Dali
{
namespace Plugin
{

/**
 * @brief A class TizenWebEngineContextMenu for context menu.
 */
class TizenWebEngineContextMenu : public Dali::WebEngineContextMenu
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineContextMenu(Ewk_Context_Menu*);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineContextMenu();

  /**
   * @copydoc Dali::WebEngineContextMenu::GetItemCount()
   */
  uint32_t GetItemCount() const override;

  /**
   * @copydoc Dali::WebEngineContextMenu::GetItemAt()
   */
  std::unique_ptr<Dali::WebEngineContextMenuItem> GetItemAt(uint32_t index) const override;

  /**
   * @copydoc Dali::WebEngineContextMenu::GetItemList()
   */
  std::vector<std::unique_ptr<Dali::WebEngineContextMenuItem>> GetItemList() const override;

  /**
   * @copydoc Dali::WebEngineContextMenu::GetPosition()
   */
  Dali::Vector2 GetPosition() const override;

  /**
   * @copydoc Dali::WebEngineContextMenu::RemoveItem()
   */
  bool RemoveItem(Dali::WebEngineContextMenuItem& item) override;

  /**
   * @copydoc Dali::WebEngineContextMenu::AppendItemAsAction()
   */
  bool AppendItemAsAction(Dali::WebEngineContextMenuItem::ItemTag tag, const std::string& title, bool enabled) override;

  /**
   * @copydoc Dali::WebEngineContextMenu::AppendItem()
   */
  bool AppendItem(Dali::WebEngineContextMenuItem::ItemTag tag, const std::string& title, const std::string& iconFile, bool enabled) override;

  /**
   * @copydoc Dali::WebEngineContextMenu::SelectItem()
   */
  bool SelectItem(Dali::WebEngineContextMenuItem& item) override;

  /**
   * @copydoc Dali::WebEngineContextMenu::Hide()
   */
  bool Hide() override;

private:
  Ewk_Context_Menu* ewkContextMenu;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_CONTEXT_MENU_H
