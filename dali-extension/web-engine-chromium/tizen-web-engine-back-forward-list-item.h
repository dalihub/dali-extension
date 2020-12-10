#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_BACK_FORWARD_LIST_ITEM_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_BACK_FORWARD_LIST_ITEM_H

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

// EXTERNAL INCLUDES
#include <string>
#include <ewk_back_forward_list_item.h>

#include <dali/devel-api/adaptor-framework/web-engine-back-forward-list-item.h>

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
  TizenWebEngineBackForwardListItem( Ewk_Back_Forward_List_Item* );

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineBackForwardListItem();

  /**
   * @brief Returns the URL of the item.
   *
   * @details The returned URL may differ from the original URL (For example,
   *          if the page is redirected).
   *
   * @return The URL of the @a item,
   *         otherwise @c "" in case of an error
   */
  std::string GetUrl() const override;

  /**
   * @brief Returns the title of the item.
   *
   * @return The title of the @a item,
   *         otherwise @c "" in case of an error
   */
  std::string GetTitle() const override;

  /**
   * @brief Returns the original URL of the item.
   *
   * @return The original URL of the @a item,
   *         otherwise @c "" in case of an error
   */
  std::string GetOriginalUrl() const override;

private:

  Ewk_Back_Forward_List_Item* ewkBackForwardListItem;

};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_BACK_FORWARD_LIST_ITEM_H
