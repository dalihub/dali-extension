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

#include "web-engine-lwe-back-forward-list.h"

namespace Dali
{
namespace Plugin
{

std::unique_ptr<Dali::WebEngineBackForwardListItem> WebEngineLweBackForwardList::GetCurrentItem() const
{
  return nullptr;
}

std::unique_ptr<Dali::WebEngineBackForwardListItem> WebEngineLweBackForwardList::GetPreviousItem() const
{
  return nullptr;
}

std::unique_ptr<Dali::WebEngineBackForwardListItem> WebEngineLweBackForwardList::GetNextItem() const
{
  return nullptr;
}

std::unique_ptr<Dali::WebEngineBackForwardListItem> WebEngineLweBackForwardList::GetItemAtIndex(uint32_t) const
{
  return nullptr;
}

uint32_t WebEngineLweBackForwardList::GetItemCount() const
{
  return 0u;
}

std::vector<std::unique_ptr<Dali::WebEngineBackForwardListItem>> WebEngineLweBackForwardList::GetBackwardItems(int)
{
  return {};
}

std::vector<std::unique_ptr<Dali::WebEngineBackForwardListItem>> WebEngineLweBackForwardList::GetForwardItems(int)
{
  return {};
}

} // namespace Plugin
} // namespace Dali
