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

// CLASS HEADER
#include "tizen-web-engine-lwe-context.h"

// INTERNAL INCLUDES
#include "tizen-web-engine-lwe.h"

namespace Dali
{
namespace Plugin
{

TizenWebEngineLweContext::TizenWebEngineLweContext()
{
}

TizenWebEngineLweContext::~TizenWebEngineLweContext()
{
}

Dali::WebEngineContext::CacheModel TizenWebEngineLweContext::GetCacheModel() const
{
  // NOT IMPLEMENTED: LWE::Settings::Get/SetCacheMode exists (int-based, 4
  // values) but doesn't map cleanly onto this 3-value enum; out of scope for
  // now.
  return CacheModel::DOCUMENT_VIEWER;
}

void TizenWebEngineLweContext::SetCacheModel(CacheModel cacheModel)
{
  // NOT IMPLEMENTED: see GetCacheModel() above.
}

void TizenWebEngineLweContext::SetProxyUri(const std::string& uri)
{
  // NOT IMPLEMENTED: LWE::Settings::SetProxyURL is per-WebContainer, not
  // engine-wide; out of scope for now.
}

std::string TizenWebEngineLweContext::GetProxyUri() const
{
  // NOT IMPLEMENTED: see SetProxyUri() above.
  return std::string();
}

void TizenWebEngineLweContext::SetDefaultProxyAuth(const std::string& username, const std::string& password)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

void TizenWebEngineLweContext::SetProxyBypassRule(const std::string& proxy, const std::string& bypass)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

std::string TizenWebEngineLweContext::GetProxyBypassRule() const
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return std::string();
}

void TizenWebEngineLweContext::SetCertificateFilePath(const std::string& certificatePath)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

std::string TizenWebEngineLweContext::GetCertificateFilePath() const
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return std::string();
}

void TizenWebEngineLweContext::DeleteAllWebDatabase()
{
  // NOT IMPLEMENTED: LWE has no WebDatabase (WebSQL) support.
}

bool TizenWebEngineLweContext::GetWebDatabaseOrigins(WebEngineSecurityOriginAcquiredCallback callback)
{
  // NOT IMPLEMENTED: see DeleteAllWebDatabase() above.
  return false;
}

bool TizenWebEngineLweContext::DeleteWebDatabase(WebEngineSecurityOrigin& origin)
{
  // NOT IMPLEMENTED: see DeleteAllWebDatabase() above.
  return false;
}

bool TizenWebEngineLweContext::GetWebStorageOrigins(WebEngineSecurityOriginAcquiredCallback callback)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return false;
}

bool TizenWebEngineLweContext::GetWebStorageUsageForOrigin(WebEngineSecurityOrigin& origin, WebEngineStorageUsageAcquiredCallback callback)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return false;
}

void TizenWebEngineLweContext::DeleteAllWebStorage()
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

bool TizenWebEngineLweContext::DeleteWebStorage(WebEngineSecurityOrigin& origin)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return false;
}

void TizenWebEngineLweContext::DeleteLocalFileSystem()
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

void TizenWebEngineLweContext::ClearCache()
{
  TizenWebEngineLWE::ClearSharedCache();
}

bool TizenWebEngineLweContext::DeleteApplicationCache(WebEngineSecurityOrigin& origin)
{
  // NOT IMPLEMENTED: LWE has no per-origin application cache API; see
  // ClearCache() for the engine-wide cache.
  return false;
}

void TizenWebEngineLweContext::GetFormPasswordList(WebEngineFormPasswordAcquiredCallback callback)
{
  // NOT IMPLEMENTED: LWE has no form-password/autofill engine (also noted in
  // TizenWebEngineLweSettings for IsAutofillPasswordFormEnabled).
}

void TizenWebEngineLweContext::RegisterDownloadStartedCallback(WebEngineDownloadStartedCallback callback)
{
  // NOT IMPLEMENTED: no corresponding LWE concept at the context level
  // (WebContainer::RegisterOnDownloadStartHandler exists per-instance, but
  // isn't wired to any WebEnginePlugin API yet either).
}

void TizenWebEngineLweContext::RegisterMimeOverriddenCallback(WebEngineMimeOverriddenCallback callback)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

void TizenWebEngineLweContext::RegisterRequestInterceptedCallback(WebEngineRequestInterceptedCallback callback)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

void TizenWebEngineLweContext::EnableCache(bool cacheEnabled)
{
  // NOT IMPLEMENTED: see GetCacheModel() above.
}

bool TizenWebEngineLweContext::IsCacheEnabled() const
{
  // NOT IMPLEMENTED: see GetCacheModel() above.
  return false;
}

void TizenWebEngineLweContext::SetAppId(const std::string& appId)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

bool TizenWebEngineLweContext::SetAppVersion(const std::string& appVersion)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return false;
}

void TizenWebEngineLweContext::SetApplicationType(const ApplicationType applicationType)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

void TizenWebEngineLweContext::SetTimeOffset(float timeOffset)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

void TizenWebEngineLweContext::SetTimeZoneOffset(float timeZoneOffset, float daylightSavingTime)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

void TizenWebEngineLweContext::SetDefaultZoomFactor(float zoomFactor)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

float TizenWebEngineLweContext::GetDefaultZoomFactor() const
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
  return -1.0f;
}

void TizenWebEngineLweContext::RegisterUrlSchemesAsCorsEnabled(const std::vector<std::string>& schemes)
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

void TizenWebEngineLweContext::RegisterJsPluginMimeTypes(const std::vector<std::string>& mimeTypes)
{
  // NOT IMPLEMENTED: LWE has no NPAPI/plugin architecture.
}

bool TizenWebEngineLweContext::DeleteAllApplicationCache()
{
  // NOT IMPLEMENTED: see DeleteApplicationCache() above.
  return false;
}

bool TizenWebEngineLweContext::DeleteAllWebIndexedDatabase()
{
  // NOT IMPLEMENTED: LWE has no IndexedDB support exposed at this level.
  return false;
}

void TizenWebEngineLweContext::DeleteFormPasswordDataList(const std::vector<std::string>& list)
{
  // NOT IMPLEMENTED: see GetFormPasswordList() above.
}

void TizenWebEngineLweContext::DeleteAllFormPasswordData()
{
  // NOT IMPLEMENTED: see GetFormPasswordList() above.
}

void TizenWebEngineLweContext::DeleteAllFormCandidateData()
{
  // NOT IMPLEMENTED: no corresponding LWE concept.
}

bool TizenWebEngineLweContext::FreeUnusedMemory()
{
  // NOT IMPLEMENTED: LWE::Settings::SetIdleModeJob(ForceGC) exists but is
  // per-WebContainer; out of scope for now.
  return false;
}

} // namespace Plugin
} // namespace Dali

extern "C" DALI_EXPORT_API Dali::WebEngineContext* GetWebEngineContext(bool isIncognito)
{
  // LWE has no private-browsing/incognito concept (WebEnginePlugin::IsIncognito()
  // always returns false for this plugin too), so the same shared context is
  // returned regardless of isIncognito. Lazily constructed so calling this
  // before any WebView exists is safe -- ClearCache() just no-ops until a
  // WebContainer is alive to clear the shared cache through.
  static Dali::Plugin::TizenWebEngineLweContext context;
  return &context;
}
