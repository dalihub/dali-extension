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

#include "web-engine-lwe-load-error.h"

#include <utility>

namespace Dali
{
namespace Plugin
{
namespace
{
// LWE exposes Starfish's internal RequestErrorType as an integer. The values
// below follow its public conversion order and are mapped explicitly so they
// are never confused with DALi's unrelated ErrorCode values.
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
} // unnamed namespace

WebEngineLweLoadError::WebEngineLweLoadError(std::string url, std::string description, int errorCode)
: mUrl(std::move(url)),
  mDescription(std::move(description)),
  mErrorCode(errorCode)
{
}

std::string WebEngineLweLoadError::GetUrl() const
{
  return mUrl;
}

Dali::WebEngineLoadError::ErrorCode WebEngineLweLoadError::GetCode() const
{
  switch(static_cast<StarfishRequestErrorType>(mErrorCode))
  {
    case StarfishRequestErrorType::HOST_LOOKUP_ERROR:
      return ErrorCode::CANT_LOOKUP_HOST;
    case StarfishRequestErrorType::UNSUPPORTED_AUTH_SCHEME_ERROR:
    case StarfishRequestErrorType::AUTHENTICATION_ERROR:
    case StarfishRequestErrorType::PROXY_AUTHENTICATION_ERROR:
      return ErrorCode::AUTHENTICATION;
    case StarfishRequestErrorType::CONNECT_ERROR:
      return ErrorCode::CANT_CONNECT;
    case StarfishRequestErrorType::IO_ERROR:
    case StarfishRequestErrorType::FILE_ERROR:
    case StarfishRequestErrorType::FILE_NOT_FOUND_ERROR:
      return ErrorCode::FAILED_FILE_IO;
    case StarfishRequestErrorType::TIMEOUT_ERROR:
      return ErrorCode::REQUEST_TIMEOUT;
    case StarfishRequestErrorType::REDIRECT_LOOP_ERROR:
      return ErrorCode::TOO_MANY_REDIRECTS;
    case StarfishRequestErrorType::UNSUPPORTED_SCHEME_ERROR:
      return ErrorCode::UNSUPPORTED_SCHEME;
    case StarfishRequestErrorType::FAILED_SSL_HANDSHAKE_ERROR:
      return ErrorCode::FAILED_TLS_HANDSHAKE;
    case StarfishRequestErrorType::BAD_URL_ERROR:
      return ErrorCode::BAD_URL;
    case StarfishRequestErrorType::TOO_MANY_REQUEST_ERROR:
      return ErrorCode::TOO_MANY_REQUESTS;
    case StarfishRequestErrorType::NO_ERROR:
    case StarfishRequestErrorType::UNKNOWN_ERROR:
    default:
      return ErrorCode::UNKNOWN;
  }
}

std::string WebEngineLweLoadError::GetDescription() const
{
  return mDescription;
}

Dali::WebEngineLoadError::ErrorType WebEngineLweLoadError::GetType() const
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
      return ErrorType::NETWORK;
    case StarfishRequestErrorType::IO_ERROR:
    case StarfishRequestErrorType::FILE_ERROR:
    case StarfishRequestErrorType::FILE_NOT_FOUND_ERROR:
    case StarfishRequestErrorType::UNSUPPORTED_SCHEME_ERROR:
    case StarfishRequestErrorType::BAD_URL_ERROR:
    case StarfishRequestErrorType::UNKNOWN_ERROR:
      return ErrorType::INTERNAL;
    case StarfishRequestErrorType::NO_ERROR:
    default:
      return ErrorType::NONE;
  }
}

} // namespace Plugin
} // namespace Dali
