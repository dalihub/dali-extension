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
#include "tizen-web-engine-lwe-load-error.h"

namespace Dali
{
namespace Plugin
{

namespace
{
// LWE::ResourceError::GetErrorCode() is a plain int cast of the engine's
// internal Starfish::RequestErrorType enum (src/public/delegate/
// LWEWebContainerDelegate.cpp: convertErrorCode() returns
// static_cast<int>(errortype)). That enum is not part of LWE's public
// headers, so its values are reproduced here in declaration order
// (src/core/modules/resource_request/ResourceRequest.h:77-94) rather than
// referenced by name.
enum class StarfishRequestErrorType
{
  NO_ERROR                      = 0,
  UNKNOWN_ERROR                 = 1,
  HOST_LOOKUP_ERROR             = 2,
  UNSUPPORTED_AUTH_SCHEME_ERROR = 3,
  AUTHENTICATION_ERROR          = 4,
  PROXY_AUTHENTICATION_ERROR    = 5,
  CONNECT_ERROR                 = 6,
  IO_ERROR                      = 7,
  TIMEOUT_ERROR                 = 8,
  REDIRECT_LOOP_ERROR           = 9,
  UNSUPPORTED_SCHEME_ERROR      = 10,
  FAILED_SSL_HANDSHAKE_ERROR    = 11,
  BAD_URL_ERROR                 = 12,
  FILE_ERROR                    = 13,
  FILE_NOT_FOUND_ERROR          = 14,
  TOO_MANY_REQUEST_ERROR        = 15,
};

} // Anonymous namespace

TizenWebEngineLweLoadError::TizenWebEngineLweLoadError(const LWE::ResourceError& error)
: mUrl(error.GetUrl()),
  mDescription(error.GetDescription()),
  mErrorCode(error.GetErrorCode())
{
}

TizenWebEngineLweLoadError::~TizenWebEngineLweLoadError()
{
}

std::string TizenWebEngineLweLoadError::GetUrl() const
{
  return mUrl;
}

Dali::WebEngineLoadError::ErrorCode TizenWebEngineLweLoadError::GetCode() const
{
  switch(static_cast<StarfishRequestErrorType>(mErrorCode))
  {
    case StarfishRequestErrorType::HOST_LOOKUP_ERROR:
    {
      return Dali::WebEngineLoadError::ErrorCode::CANT_LOOKUP_HOST;
    }
    case StarfishRequestErrorType::UNSUPPORTED_AUTH_SCHEME_ERROR:
    case StarfishRequestErrorType::AUTHENTICATION_ERROR:
    case StarfishRequestErrorType::PROXY_AUTHENTICATION_ERROR:
    {
      return Dali::WebEngineLoadError::ErrorCode::AUTHENTICATION;
    }
    case StarfishRequestErrorType::CONNECT_ERROR:
    {
      return Dali::WebEngineLoadError::ErrorCode::CANT_CONNECT;
    }
    case StarfishRequestErrorType::IO_ERROR:
    case StarfishRequestErrorType::FILE_ERROR:
    case StarfishRequestErrorType::FILE_NOT_FOUND_ERROR:
    {
      return Dali::WebEngineLoadError::ErrorCode::FAILED_FILE_IO;
    }
    case StarfishRequestErrorType::TIMEOUT_ERROR:
    {
      return Dali::WebEngineLoadError::ErrorCode::REQUEST_TIMEOUT;
    }
    case StarfishRequestErrorType::REDIRECT_LOOP_ERROR:
    {
      return Dali::WebEngineLoadError::ErrorCode::TOO_MANY_REDIRECTS;
    }
    case StarfishRequestErrorType::UNSUPPORTED_SCHEME_ERROR:
    {
      return Dali::WebEngineLoadError::ErrorCode::UNSUPPORTED_SCHEME;
    }
    case StarfishRequestErrorType::FAILED_SSL_HANDSHAKE_ERROR:
    {
      return Dali::WebEngineLoadError::ErrorCode::FAILED_TLS_HANDSHAKE;
    }
    case StarfishRequestErrorType::BAD_URL_ERROR:
    {
      return Dali::WebEngineLoadError::ErrorCode::BAD_URL;
    }
    case StarfishRequestErrorType::TOO_MANY_REQUEST_ERROR:
    {
      return Dali::WebEngineLoadError::ErrorCode::TOO_MANY_REQUESTS;
    }
    case StarfishRequestErrorType::NO_ERROR:
    case StarfishRequestErrorType::UNKNOWN_ERROR:
    default:
    {
      return Dali::WebEngineLoadError::ErrorCode::UNKNOWN;
    }
  }
}

std::string TizenWebEngineLweLoadError::GetDescription() const
{
  return mDescription;
}

Dali::WebEngineLoadError::ErrorType TizenWebEngineLweLoadError::GetType() const
{
  switch(static_cast<StarfishRequestErrorType>(mErrorCode))
  {
    case StarfishRequestErrorType::HOST_LOOKUP_ERROR:
    case StarfishRequestErrorType::UNSUPPORTED_AUTH_SCHEME_ERROR:
    case StarfishRequestErrorType::AUTHENTICATION_ERROR:
    case StarfishRequestErrorType::PROXY_AUTHENTICATION_ERROR:
    case StarfishRequestErrorType::CONNECT_ERROR:
    case StarfishRequestErrorType::TIMEOUT_ERROR:
    case StarfishRequestErrorType::REDIRECT_LOOP_ERROR:
    case StarfishRequestErrorType::FAILED_SSL_HANDSHAKE_ERROR:
    case StarfishRequestErrorType::TOO_MANY_REQUEST_ERROR:
    {
      return Dali::WebEngineLoadError::ErrorType::NETWORK;
    }
    case StarfishRequestErrorType::IO_ERROR:
    case StarfishRequestErrorType::FILE_ERROR:
    case StarfishRequestErrorType::FILE_NOT_FOUND_ERROR:
    case StarfishRequestErrorType::UNSUPPORTED_SCHEME_ERROR:
    case StarfishRequestErrorType::BAD_URL_ERROR:
    case StarfishRequestErrorType::UNKNOWN_ERROR:
    {
      return Dali::WebEngineLoadError::ErrorType::INTERNAL;
    }
    case StarfishRequestErrorType::NO_ERROR:
    default:
    {
      return Dali::WebEngineLoadError::ErrorType::NONE;
    }
  }
}

} // namespace Plugin
} // namespace Dali
