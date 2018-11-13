#
# Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

# Build the Dali image loader plugin

if USE_IMAGELOADER_EXTENSION
extension_src_dir = ../../../dali-extension

include ../../../dali-extension/image-loader/file.list

lib_LTLIBRARIES =

lib_LTLIBRARIES += libdali-image-loader-plugin.la

libdali_image_loader_plugin_la_includes =

libdali_image_loader_plugin_la_SOURCES = \
                     $(image_loader_plugin_src_files)

libdali_image_loader_plugin_la_DEPENDENCIES =

libdali_image_loader_plugin_la_CXXFLAGS = \
                      $(DLOG_CFLAGS) \
                      $(DALI_CFLAGS) \
                      -Werror -Wall

libdali_image_loader_plugin_la_CXXFLAGS+= \
                      -I../../../dali-extension/image-loader

libdali_image_loader_plugin_la_LIBADD = \
                      $(DLOG_LIBS) \
                      $(DALI_LIBS)

libdali_image_loader_plugin_la_LDFLAGS = \
                      -rdynamic
endif
