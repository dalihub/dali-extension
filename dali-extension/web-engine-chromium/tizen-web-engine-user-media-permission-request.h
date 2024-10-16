#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_USER_MEDIA_PERMISSION_REQUEST_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_USER_MEDIA_PERMISSION_REQUEST_H

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
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-user-media-permission-request.h>
#include <ewk_user_media_internal.h>

namespace Dali
{
namespace Plugin
{
/**
 * @brief A class TizenWebEngineUserMediaPermissionRequest for handling user media permission.
 */
class TizenWebEngineUserMediaPermissionRequest : public Dali::WebEngineUserMediaPermissionRequest
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineUserMediaPermissionRequest(Ewk_User_Media_Permission_Request*);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineUserMediaPermissionRequest();

  /**
   * @copydoc Dali::WebEngineUserMediaPermissionRequest::Set()
   */
  void Set(bool allowed) const override;

  /**
   * @copydoc Dali::WebEngineUserMediaPermissionRequest::Suspend()
   */
  bool Suspend() const override;

private:
  Ewk_User_Media_Permission_Request* ewkUserMediaPermissionRequest;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_USER_MEDIA_PERMISSION_REQUEST_H
