#ifndef DALI_PLUGIN_WEB_ENGINE_CONTEXT_H
#define DALI_PLUGIN_WEB_ENGINE_CONTEXT_H

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
#include <dali/devel-api/adaptor-framework/web-engine-context.h>

struct Ewk_Context;

namespace Dali
{
namespace Plugin
{

/**
 * @brief A class TizenWebEngineContext for context of chromium.
 */
class TizenWebEngineContext : public Dali::WebEngineContext
{

public:

  /**
   * @brief Constructor.
   */
  TizenWebEngineContext( Ewk_Context* );

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineContext();

  /**
   * @brief Returns the cache model type.
   *
   * @return #Dali::WebEngineContext::CacheModel
   */
  CacheModel GetCacheModel() const override;

  /**
   * @brief Requests to set the cache model.
   *
   * @param[in] cacheModel The cache model
   */
  void SetCacheModel( CacheModel cacheModel ) override;

  /**
   * @brief Sets the given proxy URI to network backend of specific context.
   *
   * @param[in] uri, proxy URI to set
   */
  void SetProxyUri( const std::string& uri ) override;

  /**
   * @brief Sets a proxy auth credential to network backend of specific context.
   *
   * @details Normally, proxy auth credential should be got from the callback
   *          set by ewk_view_authentication_callback_set, once the username in
   *          this API has been set with a non-null value, the authentication
   *          callback will never been invoked. Try to avoid using this API.
   *
   * @param[in] username username to set
   * @param[in] password password to set
   */
  void SetDefaultProxyAuth( const std::string& username, const std::string& password ) override;

  /**
   * Adds CA certificates to persistent NSS certificate database
   *
   * Function accepts a path to a CA certificate file, a path to a directory
   * containing CA certificate files, or a colon-seprarated list of those.
   *
   * Certificate files should have *.crt extension.
   *
   * Directories are traversed recursively.
   *
   * @param[in] certificatePath path to a CA certificate file(s), see above for details
   */
  void SetCertificateFilePath( const std::string& certificatePath ) override;

  /**
   * Requests for deleting all web databases.
   */
  void DeleteWebDatabase() override;

  /**
   * @brief Deletes web storage.
   *
   * @details This function does not ensure that all data will be removed.
   *          Should be used to extend free physical memory.
   *
   * @param[in] context Context object
   */
  void DeleteWebStorage() override;

  /**
   * @brief Requests for deleting all local file systems.
   */
  void DeleteLocalFileSystem() override;

  /**
   * Toggles the cache to be enabled or disabled
   *
   * Function works asynchronously.
   * By default the cache is disabled resulting in not storing network data on disk.
   *
   * @param[in] cacheDisabled enable or disable cache
   */
  void DisableCache( bool cacheDisabled ) override;

  /**
   * @brief Requests to clear cache
   */
  void ClearCache() override;

private:

  Ewk_Context* ewkContext;

};
} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_WEB_ENGINE_CONTEXT_H
