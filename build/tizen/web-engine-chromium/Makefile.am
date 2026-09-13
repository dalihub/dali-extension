#
# Copyright (c) 2020 Samsung Electronics Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

AUTOMAKE_OPTIONS = subdir-objects

extension_src_dir = ../../../dali-extension

include ../../../dali-extension/web-engine-chromium/file.list

lib_LTLIBRARIES =

lib_LTLIBRARIES += libdali2-web-engine-chromium-plugin.la

if USE_TCORE_BACKEND
libdali2_web_engine_chromium_plugin_la_SOURCES = \
                     $(web_engine_chromium_tcore_plugin_src_files)
else
libdali2_web_engine_chromium_plugin_la_SOURCES = \
                     $(web_engine_chromium_ecore_plugin_src_files)
endif

libdali2_web_engine_chromium_plugin_la_DEPENDENCIES =

libdali2_web_engine_chromium_plugin_la_CXXFLAGS = \
                      $(DALI_CFLAGS) \
                      $(WEB_ENGINE_CHROMIUM_CFLAGS) \
                      -Werror -Wall

if USE_TCORE_BACKEND
libdali2_web_engine_chromium_plugin_la_CXXFLAGS += \
                      $(TCORE_WL_CFLAGS) \
                      $(GLIB_CFLAGS) \
                      -I../../../dali-extension/web-engine-chromium/tcore
else
libdali2_web_engine_chromium_plugin_la_CXXFLAGS += \
                      $(ELEMENTARY_CFLAGS) \
                      $(ECORE_WL2_CFLAGS) \
                      -I../../../dali-extension/web-engine-chromium/ecore
endif

libdali2_web_engine_chromium_plugin_la_LIBADD = \
                      $(DLOG_LIBS) \
                      $(DALI_LIBS) \
                      $(WEB_ENGINE_CHROMIUM_LIBS)

if USE_TCORE_BACKEND
libdali2_web_engine_chromium_plugin_la_LIBADD += \
                      $(TCORE_WL_LIBS) \
                      $(GLIB_LIBS)
else
libdali2_web_engine_chromium_plugin_la_LIBADD += \
                      $(ELEMENTARY_LIBS) \
                      $(ECORE_WL2_LIBS)
endif

libdali2_web_engine_chromium_plugin_la_LDFLAGS = \
                      -rdynamic

if !USE_TCORE_BACKEND
libdali2_web_engine_chromium_plugin_la_LDFLAGS += \
                      -lelementary
endif
