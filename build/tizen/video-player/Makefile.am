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

include ../../../dali-extension/video-player/file.list

lib_LTLIBRARIES =

lib_LTLIBRARIES += libdali2-video-player-plugin.la

# Todo Video player plugin separation
if ECOREWL2
libdali2_video_player_plugin_la_SOURCES = \
                     $(video_player_ecore_wl2_plugin_src_files)
else
libdali2_video_player_plugin_la_SOURCES = \
                     $(video_player_ecore_wl_plugin_src_files)
endif

libdali2_video_player_plugin_la_DEPENDENCIES =

libdali2_video_player_plugin_la_CXXFLAGS = \
                      $(DLOG_CFLAGS) \
                      $(DALI_CFLAGS) \
                      $(CAPI_MEDIA_PLAYER_CFLAGS) \
                      $(CAPI_SYSTEM_INFO_CFLAGS) \
                      -Werror -Wall

if ECOREWL2
libdali2_video_player_plugin_la_CXXFLAGS+= $(ECORE_WL2_CFLAGS) \
                      -I../../../dali-extension/video-player/ecore-wl2
else
libdali2_video_player_plugin_la_CXXFLAGS+= $(ECORE_WAYLAND_CFLAGS) \
                      -I../../../dali-extension/video-player/ecore-wl
endif

libdali2_video_player_plugin_la_LIBADD = \
                      $(DLOG_LIBS) \
                      $(DALI_LIBS) \
                      $(CAPI_MEDIA_PLAYER_LIBS) \
                      $(CAPI_SYSTEM_INFO_LIBS)

if ECOREWL2
libdali2_video_player_plugin_la_LIBADD+= $(ECORE_WL2_LIBS)
else
libdali2_video_player_plugin_la_LIBADD+= $(ECORE_WAYLAND_LIBS)
endif

libdali2_video_player_plugin_la_LDFLAGS = \
                      -rdynamic

