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
#include <ewk_back_forward_list.h>
#include <dali/devel-api/adaptor-framework/web-engine-back-forward-list.h>
#include <dali/devel-api/adaptor-framework/web-engine-back-forward-list-item.h>

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
  TizenWebEngineBackForwardList(Ewk_Back_Forward_List*);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineBackForwardList();

  /**
   * @brief Assignment operator
   */
  TizenWebEngineBackForwardList& operator=(const TizenWebEngineBackForwardList& list)
  {
    mEwkBackForwardList = list.mEwkBackForwardList;
    return *this;
  };

  /**
   * @copydoc Dali::WebEngineBackForwardList::GetCurrentItem()
   */
  std::unique_ptr<Dali::WebEngineBackForwardListItem> GetCurrentItem() const override;

  /**
   * @copydoc Dali::WebEngineBackForwardList::GetPreviousItem()
   */
  std::unique_ptr<Dali::WebEngineBackForwardListItem> GetPreviousItem() const override;

  /**
   * @copydoc Dali::WebEngineBackForwardList::GetNextItem()
   */
  std::unique_ptr<Dali::WebEngineBackForwardListItem> GetNextItem() const override;

  /**
   * @copydoc Dali::WebEngineBackForwardList::GetItemAtIndex()
   */
  std::unique_ptr<Dali::WebEngineBackForwardListItem> GetItemAtIndex(uint32_t index) const override;

  /**
   * @copydoc Dali::WebEngineBackForwardList::GetItemCount()
   */
  uint32_t GetItemCount() const override;

  /**
   * @copydoc Dali::WebEngineBackForwardList::GetBackwardItems()
   */
  std::vector<std::unique_ptr<Dali::WebEngineBackForwardListItem>> GetBackwardItems(int limit) override;

  /**
   * @copydoc Dali::WebEngineBackForwardList::GetForwardItems()
   */
  std::vector<std::unique_ptr<Dali::WebEngineBackForwardListItem>> GetForwardItems(int limit) override;

private:

  Ewk_Back_Forward_List* mEwkBackForwardList;
};
} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_BACK_FORWARD_LIST_H
