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

#include "tizen-web-engine-context.h"

#include <ewk_context.h>
#include <ewk_context_internal.h>
#include <ewk_context_product.h>

namespace Dali
{
namespace Plugin
{

TizenWebEngineContext::TizenWebEngineContext( Ewk_Context* context )
  : ewkContext( context )
{
}

TizenWebEngineContext::~TizenWebEngineContext()
{
}

Dali::WebEngineContext::CacheModel TizenWebEngineContext::GetCacheModel() const
{
  return static_cast< Dali::WebEngineContext::CacheModel >( ewk_context_cache_model_get( ewkContext ) );
}

void TizenWebEngineContext::SetCacheModel( Dali::WebEngineContext::CacheModel cacheModel  )
{
  ewk_context_cache_model_set( ewkContext, static_cast< Ewk_Cache_Model >( cacheModel ) );
}

void TizenWebEngineContext::SetProxyUri( const std::string& uri )
{
  ewk_context_proxy_uri_set( ewkContext, uri.c_str() );
}

void TizenWebEngineContext::SetDefaultProxyAuth( const std::string& username, const std::string& password )
{
  ewk_context_proxy_default_auth_set( ewkContext, username.c_str(), password.c_str() );
}

void TizenWebEngineContext::SetCertificateFilePath( const std::string& certificatePath )
{
  ewk_context_certificate_file_set( ewkContext, certificatePath.c_str() );
}

void TizenWebEngineContext::DeleteWebDatabase()
{
  ewk_context_web_database_delete_all( ewkContext );
}

void TizenWebEngineContext::DeleteWebStorage()
{
  ewk_context_web_storage_delete_all( ewkContext );
}

void TizenWebEngineContext::DeleteLocalFileSystem()
{
  ewk_context_local_file_system_all_delete( ewkContext );
}

void TizenWebEngineContext::DisableCache( bool cacheDisabled )
{
  ewk_context_cache_disabled_set( ewkContext, cacheDisabled );
}

void TizenWebEngineContext::ClearCache()
{
  ewk_context_cache_clear( ewkContext );
}

} // namespace Plugin
} // namespace Dali
