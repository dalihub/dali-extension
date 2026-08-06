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

#ifndef DALI_EXTENSION_WEB_ENGINE_LWE_CONTEXT_H
#define DALI_EXTENSION_WEB_ENGINE_LWE_CONTEXT_H

#include <dali/devel-api/adaptor-framework/web-engine/web-engine-context.h>

namespace Dali
{
namespace Plugin
{
/**
 * Shared process-wide LWE context.
 *
 * LWE exposes most relevant state per WebContainer rather than per context.
 * Unsupported context operations deliberately report failure instead of
 * pretending that a value was applied.
 */
class WebEngineLweContext : public Dali::WebEngineContext
{
public:
  WebEngineLweContext()          = default;
  ~WebEngineLweContext() override = default;

  CacheModel GetCacheModel() const override;
  void SetCacheModel(CacheModel cacheModel) override;
  void SetProxyUri(const std::string& uri) override;
  std::string GetProxyUri() const override;
  void SetDefaultProxyAuth(const std::string& username, const std::string& password) override;
  void SetProxyBypassRule(const std::string& proxy, const std::string& bypass) override;
  std::string GetProxyBypassRule() const override;
  void SetCertificateFilePath(const std::string& certificatePath) override;
  std::string GetCertificateFilePath() const override;
  void DeleteAllWebDatabase() override;
  bool GetWebDatabaseOrigins(WebEngineSecurityOriginAcquiredCallback callback) override;
  bool DeleteWebDatabase(WebEngineSecurityOrigin& origin) override;
  bool GetWebStorageOrigins(WebEngineSecurityOriginAcquiredCallback callback) override;
  bool GetWebStorageUsageForOrigin(WebEngineSecurityOrigin& origin, WebEngineStorageUsageAcquiredCallback callback) override;
  void DeleteAllWebStorage() override;
  bool DeleteWebStorage(WebEngineSecurityOrigin& origin) override;
  void DeleteLocalFileSystem() override;
  void ClearCache() override;
  bool DeleteApplicationCache(WebEngineSecurityOrigin& origin) override;
  void GetFormPasswordList(WebEngineFormPasswordAcquiredCallback callback) override;
  void RegisterDownloadStartedCallback(WebEngineDownloadStartedCallback callback) override;
  void RegisterMimeOverriddenCallback(WebEngineMimeOverriddenCallback callback) override;
  void RegisterRequestInterceptedCallback(WebEngineRequestInterceptedCallback callback) override;
  void EnableCache(bool cacheEnabled) override;
  bool IsCacheEnabled() const override;
  void SetAppId(const std::string& appId) override;
  bool SetAppVersion(const std::string& appVersion) override;
  void SetApplicationType(const ApplicationType applicationType) override;
  void SetTimeOffset(float timeOffset) override;
  void SetTimeZoneOffset(float timeZoneOffset, float daylightSavingTime) override;
  void SetDefaultZoomFactor(float zoomFactor) override;
  float GetDefaultZoomFactor() const override;
  void RegisterUrlSchemesAsCorsEnabled(const std::vector<std::string>& schemes) override;
  void RegisterJsPluginMimeTypes(const std::vector<std::string>& mimeTypes) override;
  bool DeleteAllApplicationCache() override;
  bool DeleteAllWebIndexedDatabase() override;
  void DeleteFormPasswordDataList(const std::vector<std::string>& list) override;
  void DeleteAllFormPasswordData() override;
  void DeleteAllFormCandidateData() override;
  bool FreeUnusedMemory() override;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_EXTENSION_WEB_ENGINE_LWE_CONTEXT_H
