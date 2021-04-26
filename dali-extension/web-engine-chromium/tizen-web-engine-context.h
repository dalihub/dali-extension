#ifndef DALI_PLUGIN_WEB_ENGINE_CONTEXT_H
#define DALI_PLUGIN_WEB_ENGINE_CONTEXT_H

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
#include <dali/devel-api/adaptor-framework/web-engine-context.h>
#include <eina_list.h>
#include <string>

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
  TizenWebEngineContext(Ewk_Context*);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineContext();

  /**
   * @copydoc Dali::WebEngineContext::GetCacheModel()
   */
  CacheModel GetCacheModel() const override;

  /**
   * @copydoc Dali::WebEngineContext::SetCacheModel()
   */
  void SetCacheModel(CacheModel cacheModel) override;

  /**
   * @copydoc Dali::WebEngineContext::SetProxyUri()
   */
  void SetProxyUri(const std::string& uri) override;

  /**
   * @copydoc Dali::WebEngineContext::SetDefaultProxyAuth()
   */
  void SetDefaultProxyAuth(const std::string& username, const std::string& password) override;

  /**
   * @copydoc Dali::WebEngineContext::SetCertificateFilePath()
   */
  void SetCertificateFilePath(const std::string& certificatePath) override;

  /**
   * @copydoc Dali::WebEngineContext::DeleteAllWebDatabase()
   */
  void DeleteAllWebDatabase() override;

  /**
   * @copydoc Dali::WebEngineContext::GetWebDatabaseOrigins()
   */
  bool GetWebDatabaseOrigins(WebEngineSecurityOriginAcquiredCallback callback) override;

  /**
   * @copydoc Dali::WebEngineContext::DeleteWebDatabase()
   */
  bool DeleteWebDatabase(WebEngineSecurityOrigin& origin) override;

  /**
   * @copydoc Dali::WebEngineContext::GetWebStorageOrigins()
   */
  bool GetWebStorageOrigins(WebEngineSecurityOriginAcquiredCallback callback) override;

  /**
   * @copydoc Dali::WebEngineContext::GetWebStorageUsageForOrigin()
   */
  bool GetWebStorageUsageForOrigin(WebEngineSecurityOrigin& origin, WebEngineStorageUsageAcquiredCallback callback);

  /**
   * @copydoc Dali::WebEngineContext::DeleteAllWebStorage()
   */
  void DeleteAllWebStorage() override;

  /**
   * @copydoc Dali::WebEngineContext::DeleteWebStorageOrigin()
   */
  bool DeleteWebStorage(WebEngineSecurityOrigin& origin) override;

  /**
   * @copydoc Dali::WebEngineContext::DeleteLocalFileSystem()
   */
  void DeleteLocalFileSystem() override;

  /**
   * @copydoc Dali::WebEngineContext::ClearCache()
   */
  void ClearCache() override;

  /**
   * @copydoc Dali::WebEngineContext::DeleteApplicationCache()
   */
  bool DeleteApplicationCache(WebEngineSecurityOrigin& origin) override;

  /**
   * @copydoc Dali::WebEngineContext::GetFormPasswordList()
   */
  void GetFormPasswordList(WebEngineFormPasswordAcquiredCallback callback) override;

  /**
   * @copydoc Dali::WebEngineContext::RegisterDownloadStartedCallback()
   */
  void RegisterDownloadStartedCallback(WebEngineDownloadStartedCallback callback) override;

  /**
   * @copydoc Dali::WebEngineContext::RegisterMimeOverriddenCallback()
   */
  void RegisterMimeOverriddenCallback(WebEngineMimeOverriddenCallback callback) override;

  /**
   * @copydoc Dali::WebEngineContext::EnableCache()
   */
  void EnableCache(bool cacheEnabled) override;

  /**
   * @copydoc Dali::WebEngineContext::IsCacheEnabled()
   */
  bool IsCacheEnabled() const override;

  /**
   * @copydoc Dali::WebEngineContext::GetContextCertificateFile()
   */
  std::string GetContextCertificateFile() const override;

  /**
   * @copydoc Dali::WebEngineContext::SetContextTizenAppId()
   */
  void SetContextAppId(const std::string& appID) override;

  /**
   * @copydoc Dali::WebEngineContext::SetContextTizenAppVersion()
   */
  bool SetContextAppVersion(const std::string& appVersion) override;

  /**
   * @copydoc Dali::WebEngineContext::SetContextApplicationType()
   */
  void SetContextApplicationType(const ApplicationType applicationType) override;

  /**
   * @copydoc Dali::WebEngineContext::SetContextTimeOffset()
   */
  void SetContextTimeOffset(float timeOffset) override;

  /**
   * @copydoc Dali::WebEngineContext::SetContextTimeZoneOffset()
   */
  void SetContextTimeZoneOffset(float timeZoneOffset, float daylightSavingTime) override;

  /**
   * @copydoc Dali::WebEngineContext::RegisterUrlSchemesAsCorsEnabled()
   */
  void RegisterUrlSchemesAsCorsEnabled(const std::vector<std::string>& schemes) override;

  /**
   * @copydoc Dali::WebEngineContext::RegisterJsPluginMimeTypes()
   */
  void RegisterJsPluginMimeTypes(const std::vector<std::string>& mimeTypes)override;

  /**
   * @copydoc Dali::WebEngineContext::SetDefaultZoomFactor()
   */
  void SetDefaultZoomFactor(float zoomFactor) override;

  /**
   * @copydoc Dali::WebEngineContext::GetContextDefaultZoomFactor()
   */
  float GetContextDefaultZoomFactor() const override;

  /**
   * @copydoc Dali::WebEngineContext::DeleteAllApplicationCache()
   */
  bool DeleteAllApplicationCache() override;

  /**
   * @copydoc Dali::WebEngineContext::DeleteAllWebIndexedDatabase()
   */
  bool DeleteAllWebIndexedDatabase() override;

  /**
   * @copydoc Dali::WebEngineContext::FreeFormPasswordDataList()
   */
  void DeleteFormPasswordDataList(const std::vector<std::string>& list) override;

  /**
   * @copydoc Dali::WebEngineContext::DeleteAllFormPasswordData()
   */

  void DeleteAllFormPasswordData() override;

  /**
   * @copydoc Dali::WebEngineContext::DeleteAllFormCandidateData()
   */

  void DeleteAllFormCandidateData() override;

  /**
   * @copydoc Dali::WebEngineContext::GetContextProxy()
   */
  std::string GetContextProxy() const override;

  /**
   * @copydoc Dali::WebEngineContext::SetContextProxy()
   */
  void SetContextProxy(const std::string& proxy, const std::string& bypass) override;

  /**
   * @copydoc Dali::WebEngineContext::GetProxyBypassRule()
   */
  std::string GetProxyBypassRule() const override;

  /**
   * @copydoc Dali::WebEngineContext::NotifyLowMemory()
   */
  bool FreeUnusedMemory() override;

private:
  /**
   * @brief Callback for getting security origins.
   *
   * @param[in] origins security origins list
   * @param[in] data data for callback
   */
  static void OnSecurityOriginsAcquired(Eina_List* origins, void* data);

  /**
   * @brief Callback for getting storage.
   *
   * @param[in] usage usage for storage
   * @param[in] data data for callback
   */
  static void OnStorageUsageAcquired(uint64_t usage, void* data);

  /**
   * @brief Callback for getting form passwords.
   *
   * @param[in] list list for form password
   * @param[in] data data for callback
   */
  static void OnFormPasswordsAcquired(Eina_List* list, void* data);

  /**
   * @brief Callback for download started.
   *
   * @param[in] downloadUrl url to download
   * @param[in] data data for callback
   */
  static void OnDownloadStarted(const char* downloadUrl, void* data);

  /**
   * @brief Callback for overriding default mime type.
   *
   * @param[in] url url for which the mime type can be overridden
   * @param[in] mime current mime type
   * @param[out] newMime a new mime type for url
   * @param[in] data data for callback
   *
   * @return true in case mime should be overridden by the contents of new mime,
   * false otherwise.
   */
  static Eina_Bool OnMimeOverridden(const char* url, const char* mime, char** newMime, void* data);

private:
  WebEngineSecurityOriginAcquiredCallback webSecurityOriginAcquiredCallback;
  WebEngineStorageUsageAcquiredCallback   webStorageUsageAcquiredCallback;
  WebEngineFormPasswordAcquiredCallback   webFormPasswordAcquiredCallback;
  WebEngineDownloadStartedCallback        webDownloadStartedCallback;
  WebEngineMimeOverriddenCallback         webMimeOverriddenCallback;

  Ewk_Context* ewkContext;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_WEB_ENGINE_CONTEXT_H
