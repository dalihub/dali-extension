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

bool TizenWebEngineContext::DeleteWebStorageOrigin(WebEngineSecurityOrigin& origin)
{
  TizenWebEngineSecurityOrigin* engineOrigin = static_cast<TizenWebEngineSecurityOrigin*>(&origin);
  return ewk_context_web_storage_origin_delete(ewkContext, engineOrigin->GetSecurityOrigin());
}

void TizenWebEngineContext::DeleteLocalFileSystem()
{
  ewk_context_local_file_system_all_delete(ewkContext);
}

void TizenWebEngineContext::DisableCache(bool cacheDisabled)
{
  ewk_context_cache_disabled_set(ewkContext, cacheDisabled);
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

} // namespace Plugin
} // namespace Dali
