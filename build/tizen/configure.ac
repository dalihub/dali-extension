4_define([dali_version],[0.1.0])
AC_INIT([dali], [dali_version])
AM_INIT_AUTOMAKE([-Wall foreign])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX
AC_PROG_LIBTOOL

AX_CXX_COMPILE_STDCXX([17], [noext], [mandatory])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

LT_INIT

DALI_EXTENSION_VERSION=dali_version
AC_SUBST(DALI_EXTENSION_VERSION)

AC_ARG_WITH([tizen_55_or_greater],
            [AC_HELP_STRING([--with-tizen-55-or-greater],
                            [Use this conditional when Tizen version is 5.5 or greater])],
            [with_tizen_55_or_greater=yes],
            [with_tizen_55_or_greater=no])

AC_ARG_WITH([tizen_65_or_greater],
            [AC_HELP_STRING([--with-tizen-65-or-greater],
                            [Use this conditional when Tizen version is 6.5 or greater])],
            [with_tizen_65_or_greater=yes],
            [with_tizen_65_or_greater=no])

AC_ARG_ENABLE([debug],
              [AC_HELP_STRING([--enable-debug],
                              [Turns on debugging])],
              [enable_debug=$enableval],
              [enable_debug=no])

AC_ARG_ENABLE([web-engine-plugin],
              [AC_HELP_STRING([--enable-web-engine-plugin],
                              [Turns on web engine plugin build])],
              [enable_web_engine_plugin=$enableval],
              [enable_web_engine_plugin=no])

PKG_CHECK_MODULES([DALI], [dali2-core dali2-adaptor dali2-toolkit])

AC_CONFIG_SUBDIRS(key)
AC_CONFIG_SUBDIRS(video-player)
AC_CONFIG_SUBDIRS(image-loader)
AC_CONFIG_SUBDIRS(color-controller)

if test "x$with_tizen_55_or_greater" = "xyes"; then
  AC_CONFIG_SUBDIRS(vector-animation-renderer)
if test "x$enable_web_engine_plugin" = "xyes"; then
  AC_CONFIG_SUBDIRS(web-engine-chromium)
  AC_CONFIG_SUBDIRS(web-engine-lwe)
fi
fi

if test "x$with_tizen_65_or_greater" = "xyes"; then
  AC_CONFIG_SUBDIRS(camera-player)
  AC_CONFIG_SUBDIRS(rive-animation-view)
fi

AC_CONFIG_SUBDIRS(dali-extension)

devincludepath=${includedir}
AC_SUBST(devincludepath)

AC_CONFIG_FILES([
Makefile
dali2-extension.pc
dali2-extension-rive-animation-view.pc
])

AC_OUTPUT
