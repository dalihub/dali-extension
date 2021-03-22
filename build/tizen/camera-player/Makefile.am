#
# Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

include ../../../dali-extension/camera-player/file.list

lib_LTLIBRARIES =

lib_LTLIBRARIES += libdali2-camera-player-plugin.la

# Todo Camera player plugin separation
libdali2_camera_player_plugin_la_SOURCES = \
                     $(camera_player_plugin_src_files)

libdali2_camera_player_plugin_la_DEPENDENCIES =

libdali2_camera_player_plugin_la_CXXFLAGS = \
                      $(DLOG_CFLAGS) \
                      $(DALI_CFLAGS) \
                      $(CAPI_MEDIA_CAMERA_CFLAGS) \
                      -Werror -Wall

libdali2_camera_player_plugin_la_CXXFLAGS+= $(ECORE_WL2_CFLAGS) \
                      -I../../../dali-extension/camera-player

libdali2_camera_player_plugin_la_LIBADD = \
                      $(DLOG_LIBS) \
                      $(DALI_LIBS) \
                      $(CAPI_MEDIA_CAMERA_LIBS)

libdali2_camera_player_plugin_la_LIBADD+= $(ECORE_WL2_LIBS)

libdali2_camera_player_plugin_la_LDFLAGS = \
                      -rdynamic

