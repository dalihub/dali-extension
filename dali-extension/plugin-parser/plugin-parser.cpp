/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "DALI_PLUGIN_PARSER"

#include <cstring>
#include <vector>
#include <sstream>
#include <glib.h>
#include <pkgmgr-info.h>
#include <pkgmgr_installer_info.h>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include "plugin-parser.h"
#include <fstream>
#include <iostream>

typedef struct Metadata
{
  const char *key;
  const char *value;
} Metadata;

const std::string valueType = "true";
const std::string mdKey = "http://tizen.org/metadata/nui_vulkan_backend";
extern "C" int PKGMGR_MDPARSER_PLUGIN_INSTALL(const char *pkgId, const char *appId, GList *list)
{
  GList *tag = NULL;
  bool mdValue = false;
  Metadata *mdInfo = NULL;
  tag = g_list_first(list);
  while (tag)
  {
    mdInfo = (Metadata*)tag->data;
    if (mdInfo->key == mdKey && mdInfo->value == valueType)
    {
      mdValue = true;
    }
    tag = g_list_next(tag);
  }

  _INFO("mdValue(http://tizen.org/metadata/nui_vulkan_backend)=%d\n", mdValue );
  if (mdValue)
  {
    int ret = makeNuiVulkanBackendSymbolicLink(pkgId, true);
    _INFO("vk symbolic link is added! makeNuiVulkanBackendSymbolicLink()=%d", ret );
  }
  else
  {
    int ret = makeNuiVulkanBackendSymbolicLink(pkgId, false);
    _INFO("gl symbolic link is added! makeNuiVulkanBackendSymbolicLink()=%d", ret );
  }

  return 0;
}

extern "C" int PKGMGR_MDPARSER_PLUGIN_UPGRADE(const char *pkgId, const char *appId, GList *list)
{
  return PKGMGR_MDPARSER_PLUGIN_INSTALL(pkgId, appId, list);
}

extern "C" int getRootPath(std::string pkgId, std::string& rootPath)
{
  int ret = 0;
  char *path = 0;

  uid_t uid = 0;

  if (pkgmgr_installer_info_get_target_uid(&uid) < 0)
  {
    _ERR("Failed to get UID");
    return -1;
  }

  pkgmgrinfo_pkginfo_h handle;
  if (uid == 0)
  {
    ret = pkgmgrinfo_pkginfo_get_pkginfo(pkgId.c_str(), &handle);
    if (ret != PMINFO_R_OK)
      return -1;
  } else
  {
    ret = pkgmgrinfo_pkginfo_get_usr_pkginfo(pkgId.c_str(), uid, &handle);
    if (ret != PMINFO_R_OK)
      return -1;
  }

  ret = pkgmgrinfo_pkginfo_get_root_path(handle, &path);
  if (ret != PMINFO_R_OK)
  {
    pkgmgrinfo_pkginfo_destroy_pkginfo(handle);
    return -1;
  }
  rootPath = path;
  pkgmgrinfo_pkginfo_destroy_pkginfo(handle);

  return 0;
}

namespace bf = boost::filesystem;
extern "C" int makeNuiVulkanBackendSymbolicLink(const std::string& pkgId, bool vkOn)
{
  std::string pkgRoot;
  if (getRootPath(pkgId, pkgRoot) < 0)
  {
    _ERR("makeNuiVulkanBackendSymbolicLink() ERROR : fail to get pkgRoot!");
    return -1;
  }
  _INFO("makeNuiVulkanBackendSymbolicLink() pkgRoot=%s, vkOn=%d", static_cast<const char*>(pkgRoot.c_str()), vkOn);

  const char* nuiVkBinderPath = "/usr/lib/libdali-csharp-binder-vk.so.0.0.0";
  if(!vkOn)
  {
    nuiVkBinderPath = "/usr/lib/libdali-csharp-binder.so.0.0.0";
  }

  bf::path symbol_path = bf::path(pkgRoot) / bf::path("lib/libdali-csharp-binder.so");
  boost::system::error_code error;

  bf::create_symlink(bf::path(nuiVkBinderPath), symbol_path, error);
  if (error)
  {
    _ERR("makeNuiVulkanBackendSymbolicLink() ERROR : create_symlink() error!");
    return -1;
  }

  return 0;
}
