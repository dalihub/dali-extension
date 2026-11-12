#pragma once

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

#include <dali/devel-api/adaptor-framework/web-engine/web-engine-profile.h>
#include <climits>
#include <new>
#include <string>
#include <vector>

namespace DALI_NAMESPACE
{
namespace Plugin
{
/** Plugin-owned user arguments, independent of native backend additions. */
class WebEngineArguments
{
public:
  WebEngineArgumentsResult Store(uint32_t argc, const char* const* argv)
  {
    if(started)
    {
      return WebEngineArgumentsResult::ALREADY_INITIALIZED;
    }
    if(argc > INT_MAX || (argc && !argv))
    {
      return WebEngineArgumentsResult::INVALID_ARGUMENT;
    }
    for(uint32_t i = 0; i < argc; ++i)
    {
      if(!argv[i])
      {
        return WebEngineArgumentsResult::INVALID_ARGUMENT;
      }
    }
    try
    {
      std::vector<std::string> values;
      values.reserve(argc);
      for(uint32_t i = 0; i < argc; ++i)
      {
        values.emplace_back(argv[i]);
      }
      std::vector<const char*> pointers;
      pointers.reserve(argc + 1u);
      for(const auto& value : values)
      {
        pointers.push_back(value.c_str());
      }
      pointers.push_back(nullptr);
      mValues.swap(values);
      mPointers.swap(pointers);
      return WebEngineArgumentsResult::SUCCESS;
    }
    catch(const std::bad_alloc&)
    {
      return WebEngineArgumentsResult::BACKEND_UNAVAILABLE;
    }
  }

  void Read(uint32_t* argc, const char* const** argv) const
  {
    *argc = static_cast<uint32_t>(mValues.size());
    *argv = mPointers.data();
  }

  void PrepareNative(bool tcore)
  {
    mNativeValues.clear();
    mNativePointers.clear();
    mConstNativePointers.clear();
    auto values = mValues;
    if(tcore)
    {
      if(values.empty())
      {
        values.emplace_back("dali-webview");
      }
      for(const char* required : {"--enable-tcore", "--enable-wv-standalone"})
      {
        bool present = false;
        for(const auto& value : values)
        {
          present = present || value == required;
        }
        if(!present)
        {
          values.emplace_back(required);
        }
      }
    }
    for(const auto& value : values)
    {
      mNativeValues.emplace_back(value.begin(), value.end());
      mNativeValues.back().push_back('\0');
    }
    for(auto& value : mNativeValues)
    {
      mNativePointers.push_back(value.data());
      mConstNativePointers.push_back(value.data());
    }
    mNativePointers.push_back(nullptr);
    mConstNativePointers.push_back(nullptr);
  }

  int Count() const
  {
    return static_cast<int>(mNativeValues.size());
  }
  char** Data()
  {
    return mNativePointers.data();
  }
  const char** ConstData()
  {
    return mConstNativePointers.data();
  }
  bool started = false;
  bool succeeded = false;

private:
  std::vector<std::string> mValues;
  std::vector<const char*> mPointers{nullptr};
  std::vector<std::vector<char>> mNativeValues;
  std::vector<char*> mNativePointers;
  std::vector<const char*> mConstNativePointers;
};
} // namespace Plugin
} // namespace DALI_NAMESPACE
