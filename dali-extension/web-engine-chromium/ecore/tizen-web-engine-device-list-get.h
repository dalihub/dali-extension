#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_DEVICE_LIST_GET_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_DEVICE_LIST_GET_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-device-list-get.h>
#include <ewk_main_internal.h>
#include <ewk_settings_product.h>
#include <ewk_view_product.h>

#include <string>
#include <vector>

namespace Dali
{
namespace Plugin
{
/**
 * @brief A class TizenWebEngineDeviceListGet for getting device list.
 */
class TizenWebEngineDeviceListGet : public Dali::WebEngineDeviceListGet
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineDeviceListGet(EwkMediaDeviceInfo* device_list, int size);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineDeviceListGet();

  //static TizenWebEngineDeviceListGet New();

  /**
   * @copydoc Dali::WebEngineDeviceListGet::GetSize()
   */
  int GetSize() override;

  /**
   * @copydoc Dali::WebEngineDeviceListGet::Reset()
   */
  void Reset() override;

  /**
   * @copydoc Dali::WebEngineDeviceListGet::GetTypeAndConnect()
   */
  void GetTypeAndConnect(int32_t* type, bool* connect, int index) override;

  /**
   * @copydoc Dali::WebEngineDeviceListGet::GetDeviceId()
   */
  std::string GetDeviceId(int idx) override;

  /**
   * @copydoc Dali::WebEngineDeviceListGet::GetDeviceLabel()
   */
  std::string GetDeviceLabel(int idx) override;

private:
  std::vector<DeviceItem> vc;
};

} // namespace Plugin
} // namespace Dali
#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_DEVICE_LIST_GET_H
