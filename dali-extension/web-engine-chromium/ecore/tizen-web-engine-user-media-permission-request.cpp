/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

#include "tizen-web-engine-user-media-permission-request.h"
#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineUserMediaPermissionRequest::TizenWebEngineUserMediaPermissionRequest(Ewk_User_Media_Permission_Request* permission)
: ewkUserMediaPermissionRequest(permission)
{
}

TizenWebEngineUserMediaPermissionRequest::~TizenWebEngineUserMediaPermissionRequest()
{
}

void TizenWebEngineUserMediaPermissionRequest::Set(bool allowed) const
{
  if(ewkUserMediaPermissionRequest)
  {
    ewk_user_media_permission_request_set(ewkUserMediaPermissionRequest, allowed);
  }
}

bool TizenWebEngineUserMediaPermissionRequest::Suspend() const
{
  return ewkUserMediaPermissionRequest ? ewk_user_media_permission_request_suspend(ewkUserMediaPermissionRequest) : false;
}

} // namespace Plugin
} // namespace Dali
