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

#include "tizen-web-engine-device-list-get.h"
#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineDeviceListGet::TizenWebEngineDeviceListGet(EwkMediaDeviceInfo* device_list, int size)
{
  for(int i=0; i < size; i++)
  {
    EwkMediaDeviceInfo device = device_list[i];
    
    std::string strId = device.device_id;
    std::string strLabel = device.label;

    DeviceItem item = {strId, strLabel, device.type, device.connected};

    vc.push_back(item);
  }
}

TizenWebEngineDeviceListGet::~TizenWebEngineDeviceListGet()
{
}

int TizenWebEngineDeviceListGet::GetSize()
{
  return vc.size();
}

void TizenWebEngineDeviceListGet::Reset()
{
  vc.clear();
}

void TizenWebEngineDeviceListGet::GetTypeAndConnect(int32_t* type, bool* connect, int index)
{
  size_t idx = index;

  if(idx >= vc.size())
  {
    *type = -1;
    *connect = false;
    return;
  }

  *type = vc[idx].device_type;
  *connect = vc[idx].connected;
}

std::string TizenWebEngineDeviceListGet::GetDeviceId(int index)
{
  size_t idx = index;
  
  if(idx >= vc.size())
  {
    return "error";
  }

  std::string deviceId = vc[idx].device_id;
  return deviceId;
}

std::string TizenWebEngineDeviceListGet::GetDeviceLabel(int index)
{
  size_t idx = index;
  
  if(idx >= vc.size())
  {
    return "error";
  }

  std::string deviceLabel = vc[idx].label;
  return deviceLabel;
}

} // namespace Plugin
} // namespace Dali
