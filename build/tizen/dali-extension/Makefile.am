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

# Build the Dali extension library

extension_src_dir = ../../../dali-extension

# evas-plugin
include ../../../dali-extension/devel-api/evas-plugin/file.list
include ../../../dali-extension/internal/evas-plugin/file.list

# rive-animation-view
include ../../../dali-extension/devel-api/rive-animation-view/file.list
include ../../../dali-extension/internal/rive-animation-view/file.list

lib_LTLIBRARIES =

lib_LTLIBRARIES += libdali2-extension.la

# Todo Evas plugin separation
libdali2_extension_la_SOURCES = \
            $(evas_plugin_devel_src_files) \
            $(evas_plugin_internal_src_files) \
            $(rive_animation_view_devel_src_files) \
            $(rive_animation_view_internal_src_files)

libdali2_extension_la_DEPENDENCIES =

libdali2_extension_la_CXXFLAGS = \
            $(DLOG_CFLAGS) \
            $(DALI_CFLAGS) \
            $(DALI_EXTENSION_CFLAGS) \
            $(DALI_ADAPTOR_INTEGRATION_CFLAGS) \
            $(ELEMENTARY_CFLAGS) \
            $(EVAS_CFLAGS) \
            $(WAYLAND_CFLAGS) \
            -DEFL_BETA_API_SUPPORT \
            -I../../../ \
            -Werror -Wall

libdali2_extension_la_LIBADD = \
            $(DLOG_LIBS) \
            $(DALI_LIBS) \
            $(DALI_ADAPTOR_INTEGRATION_LIBS) \
            $(ELEMENTARY_LIBS) \
            $(EVAS_LIBS) \
            $(WAYLAND_LIBS)

libdali2_extension_la_LDFLAGS = \
            -rdynamic

#install headers
dali2_extensiondir = $(devincludepath)/dali-extension
dali2_extension_HEADERS = ../../../dali-extension/dali-extension.h

dali2_extension_evasplugindir = $(devincludepath)/dali-extension/devel-api/evas-plugin
dali2_extension_evasplugin_HEADERS = $(evas_plugin_devel_header_files)

dali2_extension_rive_animation_viewdir = $(devincludepath)/dali-extension/devel-api/rive-animation-view
dali2_extension_rive_animation_view_HEADERS = $(rive_animation_view_devel_header_files)
