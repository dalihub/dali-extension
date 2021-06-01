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

# Build the Dali vector animation renderer plugin

extension_src_dir = ../../../dali-extension

include ../../../dali-extension/vector-animation-renderer/file.list

lib_LTLIBRARIES =

lib_LTLIBRARIES += libdali2-vector-animation-renderer-plugin.la

libdali2_vector_animation_renderer_plugin_la_SOURCES = \
                     $(vector_animation_renderer_plugin_src_files)

libdali2_vector_animation_renderer_plugin_la_DEPENDENCIES =

libdali2_vector_animation_renderer_plugin_la_CXXFLAGS = \
                      -I../../../ \
                      $(DALI_CFLAGS) \
                      $(RLOTTIE_CFLAGS) \
                      -Werror -Wall

libdali2_vector_animation_renderer_plugin_la_LIBADD = \
                      $(DALI_LIBS) \
                      $(RLOTTIE_LIBS)

libdali2_vector_animation_renderer_plugin_la_LDFLAGS = \
                      -rdynamic

if RIVE

lib_LTLIBRARIES += libdali2-rive-animation-renderer-plugin.la

libdali2_rive_animation_renderer_plugin_la_SOURCES = \
                     $(rive_animation_renderer_plugin_src_files)

libdali2_rive_animation_renderer_plugin_la_DEPENDENCIES =

libdali2_rive_animation_renderer_plugin_la_CXXFLAGS = \
                      -I../../../ \
                      -I$(includedir)/rive \
                      $(DALI_CFLAGS) \
                      $(THORVG_CFLAGS) \
                      $(RIVE_TIZEN_CFLAGS) \
                      -Werror -Wall

libdali2_rive_animation_renderer_plugin_la_LIBADD = \
                      $(DALI_LIBS) \
                      $(THORVG_LIBS) \
                      $(RIVE_TIZEN_LIBS)

libdali2_rive_animation_renderer_plugin_la_LDFLAGS = \
                      -rdynamic

endif
