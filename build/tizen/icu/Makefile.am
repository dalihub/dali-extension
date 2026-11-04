#
# Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

# Build the Dali feedback plugin

extension_src_dir = ../../../dali-extension

include ../../../dali-extension/icu/file.list

lib_LTLIBRARIES =

lib_LTLIBRARIES += libdali3-icu-plugin.la

libdali3_icu_plugin_la_includes =

libdali3_icu_plugin_la_SOURCES = \
                     $(icu_plugin_src_files)

libdali3_icu_plugin_la_DEPENDENCIES =

libdali3_icu_plugin_la_CXXFLAGS = \
                      $(DLOG_CFLAGS) \
                      $(DALI_CFLAGS) \
                      $(ICU_CFLAGS) \
                      -Werror -Wall

libdali3_icu_plugin_la_CXXFLAGS+= \
                      -I../../../dali-extension/icu

libdali3_icu_plugin_la_LIBADD = \
                      $(DLOG_LIBS) \
                      $(DALI_LIBS) \
                      $(ICU_LIBS)

libdali3_icu_plugin_la_LDFLAGS = \
                      -rdynamic

