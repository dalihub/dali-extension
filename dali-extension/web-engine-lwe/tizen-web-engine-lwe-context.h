#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_LWE_CONTEXT_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_LWE_CONTEXT_H

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

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-context.h>

namespace Dali
{
namespace Plugin
{
/**
 * @brief A class TizenWebEngineLweContext for the shared engine-wide context of LWE(Starfish).
 *
 * Unlike a WebView/WebContainer, this corresponds to the web engine itself —
 * DALi's WebView loads this plugin via dlopen() and resolves the
 * "GetWebEngineContext" symbol to obtain one shared instance, regardless of
 * how many WebViews exist (see GetWebEngineContext() in the .cpp).
 *
 * LWE has no per-context state of its own (its HTTP cache, cookies, etc. are
 * Starfish-engine-level singletons already), so this class holds no members.
 * Most of the 40 methods have no corresponding LWE concept and stay no-op;
 * only ClearCache() is wired up.
 */
class TizenWebEngineLweContext : public Dali::WebEngineContext
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineLweContext();

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineLweContext() override;

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
   * @copydoc Dali::WebEngineContext::GetProxyUri()
   */
  std::string GetProxyUri() const override;

  /**
   * @copydoc Dali::WebEngineContext::SetDefaultProxyAuth()
   */
  void SetDefaultProxyAuth(const std::string& username, const std::string& password) override;

  /**
   * @copydoc Dali::WebEngineContext::SetProxyBypassRule()
   */
  void SetProxyBypassRule(const std::string& proxy, const std::string& bypass) override;

  /**
   * @copydoc Dali::WebEngineContext::GetProxyBypassRule()
   */
  std::string GetProxyBypassRule() const override;

  /**
   * @copydoc Dali::WebEngineContext::SetCertificateFilePath()
   */
  void SetCertificateFilePath(const std::string& certificatePath) override;

  /**
   * @copydoc Dali::WebEngineContext::GetCertificateFilePath()
   */
  std::string GetCertificateFilePath() const override;

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
  bool GetWebStorageUsageForOrigin(WebEngineSecurityOrigin& origin, WebEngineStorageUsageAcquiredCallback callback) override;

  /**
   * @copydoc Dali::WebEngineContext::DeleteAllWebStorage()
   */
  void DeleteAllWebStorage() override;

  /**
   * @copydoc Dali::WebEngineContext::DeleteWebStorage()
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
   * @copydoc Dali::WebEngineContext::RegisterRequestInterceptedCallback()
   */
  void RegisterRequestInterceptedCallback(WebEngineRequestInterceptedCallback callback) override;

  /**
   * @copydoc Dali::WebEngineContext::EnableCache()
   */
  void EnableCache(bool cacheEnabled) override;

  /**
   * @copydoc Dali::WebEngineContext::IsCacheEnabled()
   */
  bool IsCacheEnabled() const override;

  /**
   * @copydoc Dali::WebEngineContext::SetAppId()
   */
  void SetAppId(const std::string& appId) override;

  /**
   * @copydoc Dali::WebEngineContext::SetAppVersion()
   */
  bool SetAppVersion(const std::string& appVersion) override;

  /**
   * @copydoc Dali::WebEngineContext::SetApplicationType()
   */
  void SetApplicationType(const ApplicationType applicationType) override;

  /**
   * @copydoc Dali::WebEngineContext::SetTimeOffset()
   */
  void SetTimeOffset(float timeOffset) override;

  /**
   * @copydoc Dali::WebEngineContext::SetTimeZoneOffset()
   */
  void SetTimeZoneOffset(float timeZoneOffset, float daylightSavingTime) override;

  /**
   * @copydoc Dali::WebEngineContext::SetDefaultZoomFactor()
   */
  void SetDefaultZoomFactor(float zoomFactor) override;

  /**
   * @copydoc Dali::WebEngineContext::GetDefaultZoomFactor()
   */
  float GetDefaultZoomFactor() const override;

  /**
   * @copydoc Dali::WebEngineContext::RegisterUrlSchemesAsCorsEnabled()
   */
  void RegisterUrlSchemesAsCorsEnabled(const std::vector<std::string>& schemes) override;

  /**
   * @copydoc Dali::WebEngineContext::RegisterJsPluginMimeTypes()
   */
  void RegisterJsPluginMimeTypes(const std::vector<std::string>& mimeTypes) override;

  /**
   * @copydoc Dali::WebEngineContext::DeleteAllApplicationCache()
   */
  bool DeleteAllApplicationCache() override;

  /**
   * @copydoc Dali::WebEngineContext::DeleteAllWebIndexedDatabase()
   */
  bool DeleteAllWebIndexedDatabase() override;

  /**
   * @copydoc Dali::WebEngineContext::DeleteFormPasswordDataList()
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
   * @copydoc Dali::WebEngineContext::FreeUnusedMemory()
   */
  bool FreeUnusedMemory() override;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_LWE_CONTEXT_H
