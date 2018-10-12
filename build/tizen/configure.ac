4_define([dali_version],[0.1.0])
AC_INIT([dali], [dali_version])
AM_INIT_AUTOMAKE([-Wall foreign])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX
AC_PROG_LIBTOOL

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

LT_INIT

DALI_EXTENSION_VERSION=dali_version
AC_SUBST(DALI_EXTENSION_VERSION)

PKG_CHECK_MODULES([DALI], [dali-core dali-adaptor dali-toolkit])

AC_ARG_ENABLE([web-engine-plugin],
              [AC_HELP_STRING([--enable-web-engine-plugin], [enable web engine plugin features or not])],
              [enable-web-engine-plugin=yes],
              [enable-web-engine-plugin=no])

AM_CONDITIONAL([ENABLE_WEB_ENGINE_PLUGIN], [test x$enable-web-engine-plugin = xyes])

AC_CONFIG_SUBDIRS(key)
AC_CONFIG_SUBDIRS(video-player)
AC_CONFIG_SUBDIRS(web-engine-lite)
AC_CONFIG_SUBDIRS(web-engine-chromium)

AC_CONFIG_FILES([
Makefile
dali-extension.pc
])

AC_OUTPUT
