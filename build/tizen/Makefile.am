#
# Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = dali-extension.pc

extension_src_dir = ../../dali-extension

include ../../dali-extension/file.list

lib_LTLIBRARIES =

if USE_KEY_EXTENSION
lib_LTLIBRARIES += libdali-key-extension.la
endif

lib_LTLIBRARIES += libdali-video-player-plugin.la

# key extension
libdali_key_extension_la_SOURCES = \
			$(key_extension_src_files)
libdali_key_extension_la_DEPENDENCIES =

libdali_key_extension_la_CXXFLAGS =
			$(DLOG_CFLAGS) \
			$(DALI_CFLAGS) \
			-I../../dali-extension/ \
			-Werror -Wall

libdali_key_extension_la_LIBADD = \
			$(DLOG_LIBS)

libdali_key_extension_la_LDFLAGS = \
			-rdynamic

libdali_key_extensiondir = $(devincludepath)/dali-extension/

libdali_key_extension_HEADERS = $(key_extension_header_files)

# Todo Video player plugin separation
libdali_video_player_plugin_la_SOURCES = \
                     $(video_player_plugin_src_files)

libdali_video_player_plugin_la_DEPENDENCIES =

libdali_video_player_plugin_la_CXXFLAGS = -DDALI_COMPILATION \
                      -DDALI_ADAPTOR_COMPILATION \
                      $(DLOG_CFLAGS) \
                      $(DALI_CFLAGS) \
                      $(ECORE_WAYLAND_CFLAGS) \
                      $(CAPI_MEDIA_PLAYER_CFLAGS) \
                      -I../../dali-extension/video-player \
                      -Werror -Wall

libdali_video_player_plugin_la_LIBADD = \
                      $(DLOG_LIBS) \
                      $(DALI_LIBS) \
                      $(ECORE_WAYLAND_LIBS) \
                      $(CAPI_MEDIA_PLAYER_LIBS)

libdali_video_player_plugin_la_LDFLAGS = \
                      -rdynamic

