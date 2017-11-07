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

AC_ARG_ENABLE([keyextension],
              [AC_HELP_STRING([--enable-keyextension],
              [enables the key extension of all the symbols in the library])],
              [enable_keyextension=yes],
              [enable_keyextension=no])

AM_CONDITIONAL([USE_KEY_EXTENSION], [test x$enable_keyextension = xyes])

PKG_CHECK_MODULES([DALI], [dali-core dali-adaptor dali-toolkit])

# ToDO Videoplayer plugin separation
PKG_CHECK_MODULES(CAPI_MEDIA_PLAYER, capi-media-player)
PKG_CHECK_MODULES(ECORE_WAYLAND, ecore-wayland)
PKG_CHECK_MODULES(CAPI_SYSTEM_INFO, capi-system-info)

devincludepath=${includedir}
AC_SUBST(devincludepath)

AC_CONFIG_FILES([
Makefile
dali-extension.pc
])

AC_OUTPUT
