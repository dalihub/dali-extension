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

#include "tizen-web-engine-frame.h"

namespace Dali
{
namespace Plugin
{

TizenWebEngineFrame::TizenWebEngineFrame(Ewk_Frame_Ref frame)
    : ewkFrame(frame)
{
}

TizenWebEngineFrame::~TizenWebEngineFrame()
{
}

bool TizenWebEngineFrame::IsMainFrame() const
{
  return ewk_frame_is_main_frame(ewkFrame);
}

} // namespace Plugin
} // namespace Dali
