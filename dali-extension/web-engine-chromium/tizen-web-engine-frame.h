#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_FRAME_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_FRAME_H

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
#include <ewk_frame_internal.h>
#include <string>

#include <dali/devel-api/adaptor-framework/web-engine-frame.h>

namespace Dali
{
namespace Plugin
{

/**
 * @brief A class TizenWebEngineFrame for frame.
 */
class TizenWebEngineFrame : public Dali::WebEngineFrame
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineFrame(Ewk_Frame_Ref);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineFrame();

  /**
   * @copydoc Dali::WebEngineFrame::CanShowMimeType()
   */
  bool IsMainFrame() const override;

private:
  Ewk_Frame_Ref ewkFrame;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_FRAME_H
