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

PKG_CHECK_MODULES([DALI], [dali2-core dali2-adaptor dali2-toolkit])
PKG_CHECK_MODULES(RLOTTIE, rlottie)

if test "x$with_tizen_65_or_greater" = "xyes"; then
  PKG_CHECK_MODULES(THORVG, thorvg)
  PKG_CHECK_MODULES(RIVE_TIZEN, rive_tizen)
  AM_CONDITIONAL([RIVE], [true])
else
  AM_CONDITIONAL([RIVE], [false])
fi

devincludepath=${includedir}
AC_SUBST(devincludepath)

AC_CONFIG_FILES([
Makefile
])

AC_OUTPUT
