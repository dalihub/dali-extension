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

#include "tizen-web-engine-context.h"
#include "tizen-web-engine-security-origin.h"

#include <ewk_context.h>
#include <ewk_context_internal.h>
#include <ewk_context_product.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineContext::TizenWebEngineContext(Ewk_Context* context)
  : ewkContext(context)
{
}

TizenWebEngineContext::~TizenWebEngineContext()
{
}

Dali::WebEngineContext::CacheModel TizenWebEngineContext::GetCacheModel() const
{
  return static_cast<Dali::WebEngineContext::CacheModel>(ewk_context_cache_model_get(ewkContext));
}

void TizenWebEngineContext::SetCacheModel(Dali::WebEngineContext::CacheModel cacheModel)
{
  ewk_context_cache_model_set(ewkContext, static_cast<Ewk_Cache_Model>(cacheModel));
}

void TizenWebEngineContext::SetProxyUri(const std::string& uri)
{
  ewk_context_proxy_uri_set(ewkContext, uri.c_str());
}

void TizenWebEngineContext::SetDefaultProxyAuth(const std::string& username, const std::string& password)
{
  ewk_context_proxy_default_auth_set(ewkContext, username.c_str(), password.c_str());
}

void TizenWebEngineContext::SetCertificateFilePath(const std::string& certificatePath)
{
  ewk_context_certificate_file_set(ewkContext, certificatePath.c_str());
}

void TizenWebEngineContext::DeleteAllWebDatabase()
{
  ewk_context_web_database_delete_all(ewkContext);
}

bool TizenWebEngineContext::GetWebDatabaseOrigins(Dali::WebEngineContext::WebEngineSecurityOriginAcquiredCallback callback)
{
  webSecurityOriginAcquiredCallback = callback;
  return ewk_context_web_database_origins_get(ewkContext, &TizenWebEngineContext::OnSecurityOriginsAcquired, this);
}

bool TizenWebEngineContext::DeleteWebDatabase(WebEngineSecurityOrigin& origin)
{
  TizenWebEngineSecurityOrigin* engineOrigin = static_cast<TizenWebEngineSecurityOrigin*>(&origin);
  return ewk_context_web_database_delete(ewkContext, engineOrigin->GetSecurityOrigin());
}

bool TizenWebEngineContext::GetWebStorageOrigins(Dali::WebEngineContext::WebEngineSecurityOriginAcquiredCallback callback)
{
  webSecurityOriginAcquiredCallback = callback;
  return ewk_context_web_storage_origins_get(ewkContext, &TizenWebEngineContext::OnSecurityOriginsAcquired, this);
}

bool TizenWebEngineContext::GetWebStorageUsageForOrigin(WebEngineSecurityOrigin& origin, Dali::WebEngineContext::WebEngineStorageUsageAcquiredCallback callback)
{
  webStorageUsageAcquiredCallback = callback;
  TizenWebEngineSecurityOrigin* engineOrigin = static_cast<TizenWebEngineSecurityOrigin*>(&origin);
  return ewk_context_web_storage_usage_for_origin_get(ewkContext, engineOrigin->GetSecurityOrigin(), &TizenWebEngineContext::OnStorageUsageAcquired, this);
}

void TizenWebEngineContext::DeleteAllWebStorage()
{
  ewk_context_web_storage_delete_all(ewkContext);
}

bool TizenWebEngineContext::DeleteWebStorage(WebEngineSecurityOrigin& origin)
{
  TizenWebEngineSecurityOrigin* engineOrigin = static_cast<TizenWebEngineSecurityOrigin*>(&origin);
  return ewk_context_web_storage_origin_delete(ewkContext, engineOrigin->GetSecurityOrigin());
}

void TizenWebEngineContext::DeleteLocalFileSystem()
{
  ewk_context_local_file_system_all_delete(ewkContext);
}

void TizenWebEngineContext::ClearCache()
{
  ewk_context_cache_clear(ewkContext);
}

bool TizenWebEngineContext::DeleteApplicationCache(WebEngineSecurityOrigin& origin)
{
  TizenWebEngineSecurityOrigin* engineOrigin = static_cast<TizenWebEngineSecurityOrigin*>(&origin);
  return ewk_context_application_cache_delete(ewkContext, engineOrigin->GetSecurityOrigin());
}

void TizenWebEngineContext::GetFormPasswordList(Dali::WebEngineContext::WebEngineFormPasswordAcquiredCallback callback)
{
  webFormPasswordAcquiredCallback = callback;
  ewk_context_form_password_data_list_get(ewkContext, &TizenWebEngineContext::OnFormPasswordsAcquired, this);
}

void TizenWebEngineContext::RegisterDownloadStartedCallback(Dali::WebEngineContext::WebEngineDownloadStartedCallback callback)
{
  webDownloadStartedCallback = callback;
  ewk_context_did_start_download_callback_set(ewkContext, &TizenWebEngineContext::OnDownloadStarted, this);
}

void TizenWebEngineContext::RegisterMimeOverriddenCallback(Dali::WebEngineContext::WebEngineMimeOverriddenCallback callback)
{
  webMimeOverriddenCallback = callback;
  ewk_context_mime_override_callback_set(ewkContext, &TizenWebEngineContext::OnMimeOverridden, this);
}

void TizenWebEngineContext::OnSecurityOriginsAcquired(Eina_List* origins, void* userData)
{
  TizenWebEngineContext* pThis = static_cast<TizenWebEngineContext*>(userData);
  std::vector<std::unique_ptr<Dali::WebEngineSecurityOrigin>> originsList;

  Eina_List* it = nullptr;
  void* data = nullptr;
  EINA_LIST_FOREACH(origins, it, data)
  {
    if (data)
    {
      Ewk_Security_Origin* securityOrigin = static_cast<Ewk_Security_Origin*>(data);
      std::unique_ptr<Dali::WebEngineSecurityOrigin> origin(new TizenWebEngineSecurityOrigin(securityOrigin));
      originsList.push_back(std::move(origin));
    }
  }

  pThis->webSecurityOriginAcquiredCallback(originsList);
}

void TizenWebEngineContext::OnStorageUsageAcquired(uint64_t usage, void* userData)
{
  TizenWebEngineContext* pThis = static_cast<TizenWebEngineContext*>(userData);
  pThis->webStorageUsageAcquiredCallback(usage);
}

void TizenWebEngineContext::OnFormPasswordsAcquired(Eina_List* list, void* userData)
{
  TizenWebEngineContext* pThis = static_cast<TizenWebEngineContext*>(userData);
  std::vector<std::unique_ptr<Dali::WebEngineContext::PasswordData>> passwordDataList;

  Eina_List* it = nullptr;
  void* data = nullptr;
  EINA_LIST_FOREACH(list, it, data)
  {
    if (data)
    {
      Ewk_Password_Data* ewkPassword = static_cast<Ewk_Password_Data*>(data);
      std::unique_ptr<Dali::WebEngineContext::PasswordData> passwordData(new Dali::WebEngineContext::PasswordData());
      passwordData->url = ewkPassword->url;
      passwordData->useFingerprint = ewkPassword->useFingerprint;
      passwordDataList.push_back(std::move(passwordData));
    }
  }

  pThis->webFormPasswordAcquiredCallback(passwordDataList);
}

void TizenWebEngineContext::OnDownloadStarted(const char* downloadUrl, void* userData)
{
  TizenWebEngineContext* pThis = static_cast<TizenWebEngineContext*>(userData);
  pThis->webDownloadStartedCallback(downloadUrl);
}

Eina_Bool TizenWebEngineContext::OnMimeOverridden(const char* url, const char* defaultMime, char** newMime, void* userData)
{
  TizenWebEngineContext* pThis = static_cast<TizenWebEngineContext*>(userData);
  std::string newOverridingMime;
  bool result = pThis->webMimeOverriddenCallback(url, defaultMime, newOverridingMime);
  if (result)
  {
    // this memory would be freed by chromium-efl if mime-overridden-callback returns true.
    *newMime = strdup(newOverridingMime.c_str());
  }
  return result;
}

void TizenWebEngineContext::EnableCache(bool cacheEnabled)
{
  ewk_context_cache_disabled_set(ewkContext, !cacheEnabled);
}

bool TizenWebEngineContext::IsCacheEnabled() const
{
  return !ewk_context_cache_disabled_get(ewkContext);
}

std::string TizenWebEngineContext::GetContextCertificateFile() const
{
  const std::string ret = ewk_context_certificate_file_get(ewkContext);
  return ret;
}

void TizenWebEngineContext::SetContextAppId(const std::string& appID)
{
  ewk_context_tizen_app_id_set(ewkContext, appID.c_str());
}

bool TizenWebEngineContext::SetContextAppVersion(const std::string& appVersion)
{
  return ewk_context_tizen_app_version_set(ewkContext, appVersion.c_str());
}

void TizenWebEngineContext::SetContextApplicationType(const ApplicationType applicationType)
{
  ewk_context_application_type_set(ewkContext, static_cast<Ewk_Application_Type>(applicationType));
}

void TizenWebEngineContext::SetContextTimeOffset(float timeOffset)
{
  ewk_context_time_offset_set(ewkContext, double(timeOffset));
}

void TizenWebEngineContext::SetContextTimeZoneOffset(float timeZoneOffset, float daylightSavingTime)
{
  ewk_context_timezone_offset_set(ewkContext, double(timeZoneOffset), double(daylightSavingTime));
}

void TizenWebEngineContext::RegisterUrlSchemesAsCorsEnabled(const std::vector<std::string>& schemes)
{
  Eina_List* list = NULL;
  for (std::vector<std::string>::const_iterator it = schemes.begin(); it != schemes.end(); ++it)
  {
    list = eina_list_append(list, (*it).c_str());
  }

  ewk_context_register_url_schemes_as_cors_enabled(ewkContext, list);
}

void TizenWebEngineContext::RegisterJsPluginMimeTypes(const std::vector<std::string>& mimeTypes)
{
  Eina_List* list = NULL;
  for (std::vector<std::string>::const_iterator it = mimeTypes.begin(); it != mimeTypes.end(); ++it)
  {
    list = eina_list_append(list, (*it).c_str());
  }

  ewk_context_register_jsplugin_mime_types(ewkContext, list);
}

void TizenWebEngineContext::SetDefaultZoomFactor(float zoomFactor)
{
  ewk_context_default_zoom_factor_set(ewkContext, double(zoomFactor));
}

float TizenWebEngineContext::GetContextDefaultZoomFactor() const
{
  return float(ewk_context_default_zoom_factor_get(ewkContext));
}

bool TizenWebEngineContext::DeleteAllApplicationCache()
{
  return ewk_context_application_cache_delete_all(ewkContext);
}

bool TizenWebEngineContext::DeleteAllWebIndexedDatabase()
{
  return ewk_context_web_indexed_database_delete_all(ewkContext);
}

void TizenWebEngineContext::DeleteFormPasswordDataList(const std::vector<std::string>& list)
{
  Eina_List* eList = NULL;
  for (std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it)
  {
    eList = eina_list_append(eList, (*it).c_str());
  }

  ewk_context_form_password_data_list_free(ewkContext, eList);
}

void TizenWebEngineContext::DeleteAllFormPasswordData()
{
  ewk_context_form_password_data_delete_all(ewkContext);
}

void TizenWebEngineContext::DeleteAllFormCandidateData()
{
  ewk_context_form_candidate_data_delete_all(ewkContext);
}

std::string TizenWebEngineContext::GetContextProxy() const
{
  return ewk_context_proxy_uri_get(ewkContext);
}

void TizenWebEngineContext::SetContextProxy(const std::string& proxy, const std::string& bypass)
{
  ewk_context_proxy_set(ewkContext, proxy.c_str(), bypass.c_str());
}

std::string TizenWebEngineContext::GetProxyBypassRule() const
{
  return ewk_context_proxy_bypass_rule_get(ewkContext);
}

bool TizenWebEngineContext::FreeUnusedMemory()
{
  return ewk_context_notify_low_memory(ewkContext);
}

} // namespace Plugin
} // namespace Dali
