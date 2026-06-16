# tizen_wayland_backend.m4 - Tizen Wayland backend selection (ECORE or TCORE)
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

AC_DEFUN([TIZEN_WAYLAND_BACKEND_INIT], [
AC_ARG_WITH([tizen-wayland-backend],
            [AC_HELP_STRING([--with-tizen-wayland-backend=BACKEND],
                            [Select Tizen Wayland backend: ECORE (default) or TCORE])],
            [],
            [with_tizen_wayland_backend=ECORE])

tizen_wayland_backend=$with_tizen_wayland_backend
use_tcore_backend=no

if test "x$tizen_wayland_backend" = "xTCORE"; then
  use_tcore_backend=yes
elif test "x$tizen_wayland_backend" != "xECORE"; then
  AC_MSG_ERROR([Invalid tizen-wayland-backend: $tizen_wayland_backend (must be ECORE or TCORE)])
fi

if test "x$use_tcore_backend" != "xyes"; then
  case " $CFLAGS $CXXFLAGS " in
    *USE_TCORE_BACKEND*)
      use_tcore_backend=yes
      tizen_wayland_backend=TCORE
      ;;
  esac
fi

if test "x$use_tcore_backend" = "xyes"; then
  if test "x$enable_web_engine_plugin" = "xyes"; then
    enable_web_engine_plugin=no
  fi
fi

AM_CONDITIONAL([USE_TCORE_BACKEND], [test x$use_tcore_backend = xyes])

AC_SUBST([use_tcore_backend])
AC_SUBST([tizen_wayland_backend])
])
