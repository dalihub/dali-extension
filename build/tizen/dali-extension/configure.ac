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

# For evas-plugin
PKG_CHECK_MODULES(DALI_ADAPTOR_INTEGRATION, dali2-adaptor-integration)
PKG_CHECK_MODULES(ELEMENTARY, elementary)
PKG_CHECK_MODULES(EVAS, evas)
PKG_CHECK_MODULES(WAYLAND, [ecore-wl2])

# For rive animation
if test "x$with_tizen_65_or_greater" = "xyes"; then
  PKG_CHECK_MODULES(THORVG, thorvg)
  PKG_CHECK_MODULES(RIVE_TIZEN, rive_tizen)
  AM_CONDITIONAL([SUPPORT_RIVE], [true])
else
  AM_CONDITIONAL([SUPPORT_RIVE], [false])
fi

devincludepath=${includedir}
AC_SUBST(devincludepath)
AC_SUBST(DALI_CFLAGS)

AC_CONFIG_FILES([
Makefile
])

AC_OUTPUT
