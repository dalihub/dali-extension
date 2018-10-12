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

if test x$enable-web-engine-plugin = xyes; then
  PKG_CHECK_MODULES([DALI], [dali-core dali-adaptor dali-toolkit])
  PKG_CHECK_MODULES(WAYLAND, libtbm)
  PKG_CHECK_MODULES(WEB_ENGINE_CHROMIUM, chromium-efl)
  PKG_CHECK_MODULES(ELEMENTARY, elementary)
fi

devincludepath=${includedir}
AC_SUBST(devincludepath)

AC_CONFIG_FILES([
Makefile
])

AC_OUTPUT
