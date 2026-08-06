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

#include "web-engine-lwe-context.h"

#include "web-engine-lwe.h"

namespace Dali
{
namespace Plugin
{

Dali::WebEngineContext::CacheModel WebEngineLweContext::GetCacheModel() const
{
  return CacheModel::DOCUMENT_VIEWER;
}

void WebEngineLweContext::SetCacheModel(CacheModel)
{
}

void WebEngineLweContext::SetProxyUri(const std::string&)
{
}

std::string WebEngineLweContext::GetProxyUri() const
{
  return {};
}

void WebEngineLweContext::SetDefaultProxyAuth(const std::string&, const std::string&)
{
}

void WebEngineLweContext::SetProxyBypassRule(const std::string&, const std::string&)
{
}

std::string WebEngineLweContext::GetProxyBypassRule() const
{
  return {};
}

void WebEngineLweContext::SetCertificateFilePath(const std::string&)
{
}

std::string WebEngineLweContext::GetCertificateFilePath() const
{
  return {};
}

void WebEngineLweContext::DeleteAllWebDatabase()
{
}

bool WebEngineLweContext::GetWebDatabaseOrigins(WebEngineSecurityOriginAcquiredCallback)
{
  return false;
}

bool WebEngineLweContext::DeleteWebDatabase(WebEngineSecurityOrigin&)
{
  return false;
}

bool WebEngineLweContext::GetWebStorageOrigins(WebEngineSecurityOriginAcquiredCallback)
{
  return false;
}

bool WebEngineLweContext::GetWebStorageUsageForOrigin(WebEngineSecurityOrigin&, WebEngineStorageUsageAcquiredCallback)
{
  return false;
}

void WebEngineLweContext::DeleteAllWebStorage()
{
}

bool WebEngineLweContext::DeleteWebStorage(WebEngineSecurityOrigin&)
{
  return false;
}

void WebEngineLweContext::DeleteLocalFileSystem()
{
}

void WebEngineLweContext::ClearCache()
{
  WebEngineLwe::ClearSharedCache();
}

bool WebEngineLweContext::DeleteApplicationCache(WebEngineSecurityOrigin&)
{
  return false;
}

void WebEngineLweContext::GetFormPasswordList(WebEngineFormPasswordAcquiredCallback)
{
}

void WebEngineLweContext::RegisterDownloadStartedCallback(WebEngineDownloadStartedCallback)
{
}

void WebEngineLweContext::RegisterMimeOverriddenCallback(WebEngineMimeOverriddenCallback)
{
}

void WebEngineLweContext::RegisterRequestInterceptedCallback(WebEngineRequestInterceptedCallback)
{
}

void WebEngineLweContext::EnableCache(bool)
{
}

bool WebEngineLweContext::IsCacheEnabled() const
{
  return false;
}

void WebEngineLweContext::SetAppId(const std::string&)
{
}

bool WebEngineLweContext::SetAppVersion(const std::string&)
{
  return false;
}

void WebEngineLweContext::SetApplicationType(const ApplicationType)
{
}

void WebEngineLweContext::SetTimeOffset(float)
{
}

void WebEngineLweContext::SetTimeZoneOffset(float, float)
{
}

void WebEngineLweContext::SetDefaultZoomFactor(float)
{
}

float WebEngineLweContext::GetDefaultZoomFactor() const
{
  return -1.0f;
}

void WebEngineLweContext::RegisterUrlSchemesAsCorsEnabled(const std::vector<std::string>&)
{
}

void WebEngineLweContext::RegisterJsPluginMimeTypes(const std::vector<std::string>&)
{
}

bool WebEngineLweContext::DeleteAllApplicationCache()
{
  return false;
}

bool WebEngineLweContext::DeleteAllWebIndexedDatabase()
{
  return false;
}

void WebEngineLweContext::DeleteFormPasswordDataList(const std::vector<std::string>&)
{
}

void WebEngineLweContext::DeleteAllFormPasswordData()
{
}

void WebEngineLweContext::DeleteAllFormCandidateData()
{
}

bool WebEngineLweContext::FreeUnusedMemory()
{
  return false;
}

} // namespace Plugin
} // namespace Dali
