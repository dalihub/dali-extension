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

# Build the Dali feedback plugin

extension_src_dir = ../../../dali-extension

include ../../../dali-extension/key/file.list
include ../../../dali-extension/integration-api/file.list

lib_LTLIBRARIES =

lib_LTLIBRARIES += libdali2-key-extension.la

# key extension
libdali2_key_extension_la_SOURCES = \
			$(key_extension_src_files)

libdali2_key_extension_la_DEPENDENCIES =

libdali2_key_extension_la_CXXFLAGS = \
			-I../../../dali-extension/ \
			-I../../../dali-extension/integration-api \
			$(DLOG_CFLAGS) \
			$(DALI_CFLAGS) \
			-Werror -Wall

libdali2_key_extension_la_LIBADD = \
			$(DLOG_LIBS)

libdali2_key_extension_la_LDFLAGS = \
			-rdynamic

libdali2_key_extensiondir = $(devincludepath)/dali-extension/integration-api
libdali2_key_extension_HEADERS = $(key_extension_header_files)
