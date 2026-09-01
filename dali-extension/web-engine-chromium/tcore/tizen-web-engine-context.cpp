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

#include "tizen-web-engine-context.h"
#include "tizen-web-engine-request-interceptor.h"
#include "tizen-web-engine-security-origin.h"

#include <dali/integration-api/debug.h>

#include <wv_context.h>
#include <wv_context_internal.h>
#include <wv_context_product.h>

namespace Dali
{
namespace Plugin
{
namespace
{
/**
 * @brief Builds the serialized origin string the WV context APIs expect.
 *
 * wv_context_web_database_delete(), wv_context_web_storage_origin_delete() and
 * wv_context_web_storage_usage_for_origin_get() take "protocol://host" rather
 * than a wv_security_origin_h. WV exposes no port getter, so the port is
 * omitted and the engine falls back to the protocol default.
 */
std::string SerializeOrigin(TizenWebEngineSecurityOrigin& origin)
{
  return origin.GetProtocol() + "://" + origin.GetHost();
}
} // namespace

TizenWebEngineContext::TizenWebEngineContext(wv_context_h context, bool isIncognito)
: mWebSecurityOriginAcquiredCallback(nullptr),
  mWebStorageUsageAcquiredCallback(nullptr),
  mWebFormPasswordAcquiredCallback(nullptr),
  mWebDownloadStartedCallback(nullptr),
  mWebMimeOverriddenCallback(nullptr),
  mWebRequestInterceptedCallback(nullptr),
  mWvContext(context),
  mIsIncognito(isIncognito)
{
}

TizenWebEngineContext::~TizenWebEngineContext()
{
  // In non-incognito mode, context callbacks can be reset here.
  if(!mIsIncognito)
  {
    UnregisterContextCallbacks();
  }
}

void TizenWebEngineContext::UnregisterContextCallbacks()
{
  wv_context_intercept_request_callback_set(mWvContext, nullptr, nullptr);
  wv_context_did_start_download_callback_set(mWvContext, nullptr, nullptr);
  wv_context_mime_override_callback_set(mWvContext, nullptr, nullptr);
}

Dali::WebEngineContext::CacheModel TizenWebEngineContext::GetCacheModel() const
{
  return static_cast<Dali::WebEngineContext::CacheModel>(wv_context_cache_model_get(mWvContext));
}

void TizenWebEngineContext::SetCacheModel(Dali::WebEngineContext::CacheModel cacheModel)
{
  wv_context_cache_model_set(mWvContext, static_cast<wv_cache_model_e>(cacheModel));
}

void TizenWebEngineContext::SetProxyUri(const std::string& uri)
{
  wv_context_proxy_uri_set(mWvContext, uri.c_str());
}

std::string TizenWebEngineContext::GetProxyUri() const
{
  const char* uri = wv_context_proxy_uri_get(mWvContext);
  return uri ? std::string(uri) : std::string();
}

void TizenWebEngineContext::SetProxyBypassRule(const std::string& proxy, const std::string& bypass)
{
  // WV GAP (WV_REQUIREMENTS.md A-2): wv_context_proxy_set() takes no bypass
  // rule, so only the proxy URI is applied and every host goes through the
  // proxy. Restore the two-argument call once WV adds the bypass parameter.
  wv_context_proxy_set(mWvContext, proxy.c_str());
}

std::string TizenWebEngineContext::GetProxyBypassRule() const
{
  const char* rule = wv_context_proxy_bypass_rule_get(mWvContext);
  return rule ? std::string(rule) : std::string();
}

void TizenWebEngineContext::SetDefaultProxyAuth(const std::string& username, const std::string& password)
{
  wv_context_proxy_default_auth_set(mWvContext, username.c_str(), password.c_str());
}

void TizenWebEngineContext::SetCertificateFilePath(const std::string& certificatePath)
{
  wv_context_certificate_file_set(mWvContext, certificatePath.c_str());
}

std::string TizenWebEngineContext::GetCertificateFilePath() const
{
  const char* path = wv_context_certificate_file_get(mWvContext);
  return path ? std::string(path) : std::string();
}

void TizenWebEngineContext::DeleteAllWebDatabase()
{
  wv_context_web_database_delete_all(mWvContext);
}

bool TizenWebEngineContext::GetWebDatabaseOrigins(Dali::WebEngineContext::WebEngineSecurityOriginAcquiredCallback callback)
{
  mWebSecurityOriginAcquiredCallback = callback;
  return wv_context_web_database_origins_get(mWvContext, &TizenWebEngineContext::OnSecurityOriginsAcquired, this);
}

bool TizenWebEngineContext::DeleteWebDatabase(WebEngineSecurityOrigin& origin)
{
  TizenWebEngineSecurityOrigin* engineOrigin = static_cast<TizenWebEngineSecurityOrigin*>(&origin);
  const std::string             serialized   = SerializeOrigin(*engineOrigin);
  return wv_context_web_database_delete(mWvContext, serialized.c_str());
}

bool TizenWebEngineContext::GetWebStorageOrigins(Dali::WebEngineContext::WebEngineSecurityOriginAcquiredCallback callback)
{
  mWebSecurityOriginAcquiredCallback = callback;
  return wv_context_web_storage_origins_get(mWvContext, &TizenWebEngineContext::OnSecurityOriginsAcquired, this);
}

bool TizenWebEngineContext::GetWebStorageUsageForOrigin(WebEngineSecurityOrigin& origin, Dali::WebEngineContext::WebEngineStorageUsageAcquiredCallback callback)
{
  mWebStorageUsageAcquiredCallback           = callback;
  TizenWebEngineSecurityOrigin* engineOrigin = static_cast<TizenWebEngineSecurityOrigin*>(&origin);
  const std::string             serialized   = SerializeOrigin(*engineOrigin);
  return wv_context_web_storage_usage_for_origin_get(mWvContext, serialized.c_str(), &TizenWebEngineContext::OnStorageUsageAcquired, this);
}

void TizenWebEngineContext::DeleteAllWebStorage()
{
  wv_context_web_storage_delete_all(mWvContext);
}

bool TizenWebEngineContext::DeleteWebStorage(WebEngineSecurityOrigin& origin)
{
  TizenWebEngineSecurityOrigin* engineOrigin = static_cast<TizenWebEngineSecurityOrigin*>(&origin);
  const std::string             serialized   = SerializeOrigin(*engineOrigin);
  return wv_context_web_storage_origin_delete(mWvContext, serialized.c_str());
}

void TizenWebEngineContext::DeleteLocalFileSystem()
{
  wv_context_local_file_system_all_delete(mWvContext);
}

void TizenWebEngineContext::ClearCache()
{
  wv_context_cache_clear(mWvContext);
}

bool TizenWebEngineContext::DeleteApplicationCache(WebEngineSecurityOrigin& origin)
{
  TizenWebEngineSecurityOrigin* engineOrigin = static_cast<TizenWebEngineSecurityOrigin*>(&origin);
  return wv_context_application_cache_delete(mWvContext, engineOrigin->GetSecurityOrigin());
}

void TizenWebEngineContext::GetFormPasswordList(Dali::WebEngineContext::WebEngineFormPasswordAcquiredCallback callback)
{
  mWebFormPasswordAcquiredCallback = callback;
  wv_context_form_password_data_list_get(mWvContext, &TizenWebEngineContext::OnFormPasswordsAcquired, this);
}

void TizenWebEngineContext::RegisterDownloadStartedCallback(Dali::WebEngineContext::WebEngineDownloadStartedCallback callback)
{
  mWebDownloadStartedCallback = callback;
  if(mWebDownloadStartedCallback)
  {
    wv_context_did_start_download_callback_set(mWvContext, &TizenWebEngineContext::OnDownloadStarted, this);
  }
  else
  {
    wv_context_did_start_download_callback_set(mWvContext, nullptr, nullptr);
  }
}

void TizenWebEngineContext::RegisterMimeOverriddenCallback(Dali::WebEngineContext::WebEngineMimeOverriddenCallback callback)
{
  mWebMimeOverriddenCallback = callback;
  if(mWebMimeOverriddenCallback)
  {
    wv_context_mime_override_callback_set(mWvContext, &TizenWebEngineContext::OnMimeOverridden, this);
  }
  else
  {
    wv_context_mime_override_callback_set(mWvContext, nullptr, nullptr);
  }
}

void TizenWebEngineContext::RegisterRequestInterceptedCallback(Dali::WebEngineContext::WebEngineRequestInterceptedCallback callback)
{
  mWebRequestInterceptedCallback = callback;
  if(mWebRequestInterceptedCallback)
  {
    wv_context_intercept_request_callback_set(mWvContext, &TizenWebEngineContext::OnRequestIntercepted, this);
  }
  else
  {
    wv_context_intercept_request_callback_set(mWvContext, nullptr, nullptr);
  }
}

void TizenWebEngineContext::EnableCache(bool cacheEnabled)
{
  wv_context_cache_disabled_set(mWvContext, !cacheEnabled);
}

bool TizenWebEngineContext::IsCacheEnabled() const
{
  return !wv_context_cache_disabled_get(mWvContext);
}

void TizenWebEngineContext::SetAppId(const std::string& appId)
{
  wv_context_tizen_app_id_set(mWvContext, appId.c_str());
}

bool TizenWebEngineContext::SetAppVersion(const std::string& appVersion)
{
  return wv_context_tizen_app_version_set(mWvContext, appVersion.c_str());
}

void TizenWebEngineContext::SetApplicationType(const ApplicationType applicationType)
{
  wv_context_application_type_set(mWvContext, static_cast<wv_context_application_type_e>(applicationType));
}

void TizenWebEngineContext::SetTimeOffset(float timeOffset)
{
  wv_context_time_offset_set(mWvContext, double(timeOffset));
}

void TizenWebEngineContext::SetTimeZoneOffset(float timeZoneOffset, float daylightSavingTime)
{
  wv_context_timezone_offset_set(mWvContext, double(timeZoneOffset), double(daylightSavingTime));
}

void TizenWebEngineContext::SetDefaultZoomFactor(float zoomFactor)
{
  wv_context_default_zoom_factor_set(mWvContext, double(zoomFactor));
}

float TizenWebEngineContext::GetDefaultZoomFactor() const
{
  return (float)wv_context_default_zoom_factor_get(mWvContext);
}

void TizenWebEngineContext::RegisterUrlSchemesAsCorsEnabled(const std::vector<std::string>& schemes)
{
  GList* list = nullptr;
  for(std::vector<std::string>::const_iterator it = schemes.begin(); it != schemes.end(); ++it)
  {
    // Duplicated: WV does not document whether it copies the strings or keeps
    // the pointers, and the caller's vector is not ours to outlive.
    list = g_list_append(list, g_strdup(it->c_str()));
  }
  wv_context_register_url_schemes_as_cors_enabled(mWvContext, list);
  g_list_free_full(list, g_free);
}

void TizenWebEngineContext::RegisterJsPluginMimeTypes(const std::vector<std::string>& mimeTypes)
{
  GList* list = nullptr;
  for(std::vector<std::string>::const_iterator it = mimeTypes.begin(); it != mimeTypes.end(); ++it)
  {
    list = g_list_append(list, g_strdup(it->c_str()));
  }
  wv_context_register_jsplugin_mime_types(mWvContext, list);
  g_list_free_full(list, g_free);
}

bool TizenWebEngineContext::DeleteAllApplicationCache()
{
  return wv_context_application_cache_delete_all(mWvContext);
}

bool TizenWebEngineContext::DeleteAllWebIndexedDatabase()
{
  return wv_context_web_indexed_database_delete_all(mWvContext);
}

void TizenWebEngineContext::DeleteFormPasswordDataList(const std::vector<std::string>& list)
{
  // WV GAP (WV_REQUIREMENTS.md C): wv_context_form_password_data_list_free() is
  // not declared by the target WV headers, so there is nothing to hand the list
  // to and the engine-side entries stay as they are. Building a GList here
  // would only be thrown away.
  (void)list;
}

void TizenWebEngineContext::DeleteAllFormPasswordData()
{
  wv_context_form_password_data_delete_all(mWvContext);
}

void TizenWebEngineContext::DeleteAllFormCandidateData()
{
  wv_context_form_candidate_data_delete_all(mWvContext);
}

bool TizenWebEngineContext::FreeUnusedMemory()
{
  return wv_context_notify_low_memory(mWvContext);
}

void TizenWebEngineContext::RequestIntercepted(Dali::WebEngineRequestInterceptorPtr interceptor)
{
  if(mWebRequestInterceptedCallback)
  {
    mWebRequestInterceptedCallback(interceptor);
  }
}

void TizenWebEngineContext::OnRequestIntercepted(wv_context_h , wv_intercept_request_h request, void* userData)
{
  TizenWebEngineContext*               pThis          = static_cast<TizenWebEngineContext*>(userData);
  Dali::WebEngineRequestInterceptorPtr webInterceptor = new TizenWebEngineRequestInterceptor(request);
  pThis->RequestIntercepted(webInterceptor);
}

void TizenWebEngineContext::OnSecurityOriginsAcquired(GList* origins, void* userData)
{
  TizenWebEngineContext*                                      pThis = static_cast<TizenWebEngineContext*>(userData);
  std::vector<std::unique_ptr<Dali::WebEngineSecurityOrigin>> originsList;

  for(GList* it = origins; it != nullptr; it = it->next)
  {
    if(it->data)
    {
      wv_security_origin_h securityOrigin = static_cast<wv_security_origin_h>(it->data);
      std::unique_ptr<Dali::WebEngineSecurityOrigin> origin(new TizenWebEngineSecurityOrigin(securityOrigin));
      originsList.push_back(std::move(origin));
    }
  }

  pThis->mWebSecurityOriginAcquiredCallback(originsList);
}

void TizenWebEngineContext::OnStorageUsageAcquired(uint64_t usage, void* userData)
{
  TizenWebEngineContext* pThis = static_cast<TizenWebEngineContext*>(userData);
  pThis->mWebStorageUsageAcquiredCallback(usage);
}

void TizenWebEngineContext::OnFormPasswordsAcquired(GList* list, void* userData)
{
  TizenWebEngineContext*                                             pThis = static_cast<TizenWebEngineContext*>(userData);
  std::vector<std::unique_ptr<Dali::WebEngineContext::PasswordData>> passwordDataList;

  // WV GAP (WV_REQUIREMENTS.md D-1): wv_context_form_password_data_list_get()
  // hands back a bare GList* whose element type WV does not expose, so `url`
  // and `useFingerprint` cannot be read from the entries. Report an empty list
  // until wv_password_data_s exists; restore the walk over `list` then.
  (void)list;

  pThis->mWebFormPasswordAcquiredCallback(passwordDataList);
}

void TizenWebEngineContext::OnDownloadStarted(const char* downloadUrl, void* userData)
{
  TizenWebEngineContext* pThis = static_cast<TizenWebEngineContext*>(userData);
  pThis->mWebDownloadStartedCallback(downloadUrl);
}

bool TizenWebEngineContext::OnMimeOverridden(const char* url, const char* defaultMime, char** newMime, void* userData)
{
  TizenWebEngineContext* pThis = static_cast<TizenWebEngineContext*>(userData);
  std::string            newOverridingMime;
  bool                   result = pThis->mWebMimeOverriddenCallback(url, defaultMime, newOverridingMime);
  if(result)
  {
    // this memory would be freed by chromium-efl if mime-overridden-callback returns true.
    *newMime = strdup(newOverridingMime.c_str());
  }
  return result;
}

} // namespace Plugin
} // namespace Dali
