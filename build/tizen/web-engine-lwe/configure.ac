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

# Before any DALI_CFLAGS assignment: pkg.m4 skips pkg-config when the
# variable is already set, and the headers live under include/dali<major>,
# which only pkg-config knows.
PKG_CHECK_MODULES([DALI], [dali3-core dali3-adaptor])

if test "x$enable_debug" = "xyes"; then
  DALI_CFLAGS="$DALI_CFLAGS -DDEBUG_ENABLED"
fi

if test "x$with_tizen_90_or_greater" = "xyes"; then
  DALI_CFLAGS="$DALI_CFLAGS -DOVER_TIZEN_VERSION_9"
fi

PKG_CHECK_MODULES(WAYLAND, libtbm)
PKG_CHECK_MODULES([WEB_ENGINE_LWE], [lightweight-web-engine])

# The API major is in the package names (dali3-*). The headers install under
# include/dali<major>, beside a DALi 2.x that may be built from different
# code; the spec adds the 2.x name as a link when dali2 aliases are enabled.
dali_api_major=3
devincludepath=${includedir}/dali${dali_api_major}
AC_SUBST(devincludepath)
AC_SUBST(DALI_CFLAGS)

AC_CONFIG_FILES([
Makefile
])

AC_OUTPUT
