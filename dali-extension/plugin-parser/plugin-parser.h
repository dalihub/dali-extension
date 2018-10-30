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

#ifndef __DALI_PLUGIN_PARSER_INTERFACE__
#define __DALI_PLUGIN_PARSER_INTERFACE__

#ifndef NO_TIZEN
#include <dlog.h>
#define LOGX(fmt, arg...) \
    ({ do { \
        dlog_print(DLOG_INFO, LOG_TAG, fmt, ##arg); \
    } while (0); })

#else
#include <stdio.h>
#define LOGE(fmt, args...) printf(fmt, ##args)
#define LOGD(fmt, args...) printf(fmt, ##args)
#define LOGI(fmt, args...) printf(fmt, ##args)
#define LOGX(fmt, args...) printf(fmt, ##args)
#endif

#ifndef _ERR
#define _ERR(fmt, args...) LOGE(fmt "\n", ##args)
#endif

#ifndef _DBG
#define _DBG(fmt, args...) LOGD(fmt "\n", ##args)
#endif

#ifndef _INFO
#define _INFO(fmt, args...) LOGI(fmt "\n", ##args)
#endif

#ifndef _LOGX
#define _LOGX(fmt, args...) LOGX(fmt "\n", ##args)
#endif

extern "C"
{
    typedef struct _xmlDoc xmlDoc;
    typedef xmlDoc* xmlDocPtr;
    int PKGMGR_PARSER_PLUGIN_PRE_INSTALL(const char *pkgId);
    int PKGMGR_PARSER_PLUGIN_PRE_UPGRADE(const char *pkgId);
    int PKGMGR_PARSER_PLUGIN_PRE_UNINSTALL(const char *pkgId);

    int PKGMGR_PARSER_PLUGIN_INSTALL(xmlDocPtr doc, const char* pkgId);
    int PKGMGR_PARSER_PLUGIN_UPGRADE(xmlDocPtr doc, const char* pkgId);
    int PKGMGR_PARSER_PLUGIN_UNINSTALL(xmlDocPtr doc, const char* pkgId);

    int PKGMGR_PARSER_PLUGIN_POST_INSTALL(const char *pkgId);
    int PKGMGR_PARSER_PLUGIN_POST_UPGRADE(const char *pkgId);
    int PKGMGR_PARSER_PLUGIN_POST_UNINSTALL(const char *pkgId);

    int makeNuiVulkanBackendSymbolicLink(const std::string& pkgId, bool vkOn);
    int getRootPath(std::string pkgId, std::string& rootPath);
}

#endif /* __DALI_PLUGIN_PARSER_INTERFACE__ */
