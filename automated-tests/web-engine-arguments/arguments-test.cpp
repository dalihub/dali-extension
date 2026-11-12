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

#include "../../dali-extension/web-engine-common/web-engine-arguments.h"
#include <cassert>
#include <cstring>
#include <iostream>

int main()
{
  using Result = Dali::WebEngineArgumentsResult;
  Dali::Plugin::WebEngineArguments arguments;
  {
    char program[] = "browser";
    char option[] = "--incognito";
    char* input[] = {program, option};
    assert(arguments.Store(2, input) == Result::SUCCESS);
    option[2] = 'X';
  }
  assert(!arguments.started);
  assert(arguments.Store(1, nullptr) == Result::INVALID_ARGUMENT);
  const char* invalid[] = {nullptr};
  assert(arguments.Store(1, invalid) == Result::INVALID_ARGUMENT);
  assert(arguments.Store(UINT32_MAX, invalid) == Result::INVALID_ARGUMENT);
  uint32_t count;
  const char* const* values;
  arguments.Read(&count, &values);
  assert(count == 2 && values[2] == nullptr);
  assert(std::strcmp(values[1], "--incognito") == 0);
  arguments.PrepareNative(true);
  assert(arguments.Count() == 4);
  assert(arguments.Data()[4] == nullptr);
  assert(std::strcmp(arguments.Data()[2], "--enable-tcore") == 0);
  assert(std::strcmp(arguments.Data()[3], "--enable-wv-standalone") == 0);
  // Native argv is writable and separate from the original user settings.
  arguments.Data()[0][0] = 'X';
  Dali::Plugin::WebEngineArguments otherBackend;
  arguments.Read(&count, &values);
  assert(otherBackend.Store(count, values) == Result::SUCCESS);
  otherBackend.PrepareNative(false);
  assert(otherBackend.Count() == 2);
  assert(std::strcmp(otherBackend.Data()[0], "browser") == 0);
  arguments.PrepareNative(true);
  assert(arguments.Count() == 4);
  const char* supplied[] = {"browser", "--enable-tcore", "--enable-wv-standalone"};
  assert(arguments.Store(3, supplied) == Result::SUCCESS);
  arguments.PrepareNative(true);
  assert(arguments.Count() == 3);
  assert(arguments.Store(0, nullptr) == Result::SUCCESS);
  arguments.Read(&count, &values);
  assert(count == 0 && values[0] == nullptr);
  arguments.PrepareNative(true);
  assert(arguments.Count() == 3);
  arguments.started = true;
  assert(arguments.Store(0, nullptr) == Result::ALREADY_INITIALIZED);
  assert(arguments.Store(1, nullptr) == Result::ALREADY_INITIALIZED);
  std::cout << "argument ownership, replacement, validation and native argv passed\n";
}
