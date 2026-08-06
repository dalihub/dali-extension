/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#ifndef DALI_EXTENSION_WEB_ENGINE_LWE_BACK_FORWARD_LIST_H
#define DALI_EXTENSION_WEB_ENGINE_LWE_BACK_FORWARD_LIST_H

#include <dali/devel-api/adaptor-framework/web-engine/web-engine-back-forward-list.h>

namespace Dali
{
namespace Plugin
{
/**
 * LWE can navigate its history but does not expose history entries.
 */
class WebEngineLweBackForwardList : public Dali::WebEngineBackForwardList
{
public:
  std::unique_ptr<Dali::WebEngineBackForwardListItem> GetCurrentItem() const override;
  std::unique_ptr<Dali::WebEngineBackForwardListItem> GetPreviousItem() const override;
  std::unique_ptr<Dali::WebEngineBackForwardListItem> GetNextItem() const override;
  std::unique_ptr<Dali::WebEngineBackForwardListItem> GetItemAtIndex(uint32_t index) const override;
  uint32_t GetItemCount() const override;
  std::vector<std::unique_ptr<Dali::WebEngineBackForwardListItem>> GetBackwardItems(int limit) override;
  std::vector<std::unique_ptr<Dali::WebEngineBackForwardListItem>> GetForwardItems(int limit) override;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_EXTENSION_WEB_ENGINE_LWE_BACK_FORWARD_LIST_H
