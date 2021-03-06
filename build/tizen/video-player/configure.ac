4_define([dali_version],[0.1.0])
AC_INIT([dali], [dali_version])
AM_INIT_AUTOMAKE([-Wall foreign])

AC_CONFIG_MACRO_DIRS([m4 ../m4])

AC_PROG_CXX
AC_PROG_LIBTOOL

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AX_CXX_COMPILE_STDCXX([17], [noext], [mandatory])

LT_INIT

DALI_EXTENSION_VERSION=dali_version
AC_SUBST(DALI_EXTENSION_VERSION)

if test "x$enable_debug" = "xyes"; then
  DALI_CFLAGS="$DALI_CFLAGS -DDEBUG_ENABLED"
fi

PKG_CHECK_MODULES([DALI], [dali2-core dali2-adaptor dali2-toolkit])
PKG_CHECK_MODULES(CAPI_MEDIA_PLAYER, capi-media-player)
PKG_CHECK_MODULES(CAPI_SYSTEM_INFO, capi-system-info)

AC_ARG_ENABLE([ecore-wl2],
              [AC_HELP_STRING([--enable-ecore-wl2], [build with ecorewl2 on tizen])],
              [enable_ecore_wl2=yes],
              [enable_ecore_wl2=no])

AM_CONDITIONAL([ECOREWL2], [test x$enable_ecore_wl2 = xyes])

if test x$enable_ecore_wl2 = xyes; then
  PKG_CHECK_MODULES(ECORE_WL2, ecore-wl2)
else
  PKG_CHECK_MODULES(ECORE_WAYLAND, ecore-wayland)
fi

devincludepath=${includedir}
AC_SUBST(devincludepath)
AC_SUBST(DALI_CFLAGS)

AC_CONFIG_FILES([
Makefile
])

AC_OUTPUT
