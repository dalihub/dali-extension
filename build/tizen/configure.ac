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

AC_CONFIG_SUBDIRS(key)
AC_CONFIG_SUBDIRS(video-player)
AC_CONFIG_SUBDIRS(web-engine-chromium)
AC_CONFIG_SUBDIRS(image-loader)
AC_CONFIG_SUBDIRS(vector-animation-renderer)
AC_CONFIG_SUBDIRS(color-controller)

AC_CONFIG_FILES([
Makefile
dali-extension.pc
])

AC_OUTPUT
