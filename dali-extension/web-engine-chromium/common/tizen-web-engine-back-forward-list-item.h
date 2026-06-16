#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_BACK_FORWARD_LIST_ITEM_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_BACK_FORWARD_LIST_ITEM_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <ewk_back_forward_list_item.h>

#include <string>

#include <dali/devel-api/adaptor-framework/web-engine/web-engine-back-forward-list-item.h>

namespace Dali
{
namespace Plugin
{
/**
 * @brief A class TizenWebEngineBackForwardListItem for back-forward list item.
 */
class TizenWebEngineBackForwardListItem : public Dali::WebEngineBackForwardListItem
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineBackForwardListItem(Ewk_Back_Forward_List_Item* item, bool willItemBeDeleted = false);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineBackForwardListItem();

  /**
   * @copydoc Dali::WebEngineBackForwardListItem::GetUrl()
   */
  std::string GetUrl() const override;

  /**
   * @copydoc Dali::WebEngineBackForwardListItem::GetTitle()
   */
  std::string GetTitle() const override;

  /**
   * @copydoc Dali::WebEngineBackForwardListItem::GetOriginalUrl()
   */
  std::string GetOriginalUrl() const override;

private:
  Ewk_Back_Forward_List_Item* mEwkBackForwardListItem;

  bool mWillItemBeDeleted; // if true, Ewk api will be called  to unref the item
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_BACK_FORWARD_LIST_ITEM_H
