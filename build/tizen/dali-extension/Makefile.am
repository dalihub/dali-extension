#
# Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
# capture
include ../../../dali-extension/devel-api/capture/file.list
include ../../../dali-extension/internal/capture/file.list

lib_LTLIBRARIES =

lib_LTLIBRARIES += libdali-extension.la

# Todo Evas plugin separation
libdali_extension_la_SOURCES = \
			$(evas_plugin_devel_src_files) \
			$(evas_plugin_internal_src_files) \
			$(capture_devel_src_files) \
			$(capture_internal_src_files)

libdali_extension_la_DEPENDENCIES =

libdali_extension_la_CXXFLAGS = \
			$(DLOG_CFLAGS) \
			$(DALI_CFLAGS) \
			$(DALI_EXTENSION_CFLAGS) \
			$(DALI_ADAPTOR_INTEGRATION_CFLAGS) \
			$(ELEMENTARY_CFLAGS) \
			$(EVAS_CFLAGS) \
			$(WAYLAND_CFLAGS) \
			$(LIBTBM_CFLAGS) \
			$(LIBPNG_CFLAGS) \
			-DEFL_BETA_API_SUPPORT \
			-I../../../ \
			-Werror -Wall

libdali_extension_la_LIBADD = \
			$(DLOG_LIBS) \
			$(DALI_LIBS) \
			$(DALI_ADAPTOR_INTEGRATION_LIBS) \
			$(ELEMENTARY_LIBS) \
			$(EVAS_LIBS) \
			$(WAYLAND_LIBS) \
			$(LIBTBM_LIBS) \
			$(LIBPNG_LIBS)

libdali_extension_la_LDFLAGS = \
			-rdynamic

#install headers
dali_extensiondir = $(devincludepath)/dali-extension
dali_extension_HEADERS = ../../../dali-extension/dali-extension.h

dali_extension_capturedir = $(devincludepath)/dali-extension/devel-api/capture
dali_extension_capture_HEADERS = $(capture_devel_header_files)

dali_extension_evasplugindir = $(devincludepath)/dali-extension/devel-api/evas-plugin
dali_extension_evasplugin_HEADERS = $(evas_plugin_devel_header_files)
