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

# For evas-plugin
PKG_CHECK_MODULES(DALI_ADAPTOR_INTEGRATION, dali-adaptor-integration)
PKG_CHECK_MODULES(ELEMENTARY, elementary)
PKG_CHECK_MODULES(EVAS, evas)
PKG_CHECK_MODULES(WAYLAND, [ecore-wl2])

devincludepath=${includedir}
AC_SUBST(devincludepath)

AC_CONFIG_FILES([
Makefile
])

AC_OUTPUT
