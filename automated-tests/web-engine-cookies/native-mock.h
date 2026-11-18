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

#pragma once
#include <string>

struct NativeCookies
{
  int registrations = 0;
  int removals = 0;
  void* data = nullptr;
#ifdef BACKEND_ecore
  void (*callback)(void*) = nullptr;
#else
  void (*callback)(void*, void*) = nullptr;
#endif
  int policy = 2;
  int storage = -1;
  std::string path;
};
using Ewk_Cookie_Manager = NativeCookies;
using wv_cookie_manager_h = NativeCookies*;
enum Ewk_Cookie_Accept_Policy
{
  EWK_COOKIE_ACCEPT_POLICY_ALWAYS,
  EWK_COOKIE_ACCEPT_POLICY_NEVER,
  EWK_COOKIE_ACCEPT_POLICY_NO_THIRD_PARTY
};
enum wv_cookie_accept_policy_e
{
  WV_COOKIE_ACCEPT_POLICY_ALWAYS,
  WV_COOKIE_ACCEPT_POLICY_NEVER,
  WV_COOKIE_ACCEPT_POLICY_NO_THIRD_PARTY
};
enum Ewk_Cookie_Persistent_Storage
{
  EWK_COOKIE_PERSISTENT_STORAGE_TEXT,
  EWK_COOKIE_PERSISTENT_STORAGE_SQLITE
};
enum wv_cookie_manager_persistent_storage_e
{
  WV_COOKIE_MANAGER_PERSISTENT_STORAGE_TEXT,
  WV_COOKIE_MANAGER_PERSISTENT_STORAGE_SQLITE
};
inline void ewk_cookie_manager_accept_policy_set(NativeCookies* n, Ewk_Cookie_Accept_Policy p)
{
  n->policy = p;
}
inline void wv_cookie_manager_accept_policy_set(NativeCookies* n, wv_cookie_accept_policy_e p)
{
  n->policy = p;
}
inline void ewk_cookie_manager_persistent_storage_set(NativeCookies* n, const char* p, Ewk_Cookie_Persistent_Storage s)
{
  n->path = p;
  n->storage = s;
}
inline void wv_cookie_manager_persistent_storage_set(NativeCookies* n, const char* p, wv_cookie_manager_persistent_storage_e s)
{
  n->path = p;
  n->storage = s;
}
inline void ewk_cookie_manager_cookies_clear(NativeCookies*) {}
inline void wv_cookie_manager_cookies_clear(NativeCookies*) {}
#ifdef BACKEND_ecore
inline void ewk_cookie_manager_changes_watch(NativeCookies* n, void (*cb)(void*), void* data)
#else
inline void wv_cookie_manager_changes_watch(NativeCookies* n, void (*cb)(void*, void*), void* data)
#endif
{
  cb ? ++n->registrations : ++n->removals;
  n->callback = cb;
  n->data = data;
}
