#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_BACK_FORWARD_LIST_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_BACK_FORWARD_LIST_H

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
#include <ewk_back_forward_list.h>

#include <dali/devel-api/adaptor-framework/web-engine-back-forward-list.h>

namespace Dali
{
namespace Plugin
{

class WebEngineBackForwardListItem;

/**
 * @brief A class TizenWebEngineBackForwardList for back-forward list of chromium.
 */
class TizenWebEngineBackForwardList : public Dali::WebEngineBackForwardList
{

public:

  /**
   * @brief Constructor.
   */
  TizenWebEngineBackForwardList( Ewk_Back_Forward_List* );

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineBackForwardList();

  /**
   * @brief Returns the current item in the @a list.
   *
   * @param[in] list The back-forward list instance
   *
   * @return The current item in the @a list,\n
   *         otherwise @c NULL in case of an error
   */
  Dali::WebEngineBackForwardListItem& GetCurrentItem() const override;

  /**
   * @brief Returns the item at a given @a index relative to the current item.
   *
   * @param[in] index The index of the item
   *
   * @return The item at a given @a index relative to the current item,\n
   *         otherwise @c NULL in case of an error
   */
  Dali::WebEngineBackForwardListItem& GetItemAtIndex( uint32_t index ) const override;

  /**
   * @brief Returns the length of the back-forward list including the current
   *        item.
   *
   * @return The length of the back-forward list including the current item,\n
   *         otherwise @c 0 in case of an error
   */
  uint32_t GetItemCount() const override;

private:

  Ewk_Back_Forward_List* ewkBackForwardList;

};
} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_BACK_FORWARD_LIST_H
