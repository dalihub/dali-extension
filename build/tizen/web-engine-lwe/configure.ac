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

AC_ARG_WITH([da_profile],
            [AC_HELP_STRING([--with-da-profile],
                            [Use this conditional when use da profile])],
            [with_da_profile=yes],
            [with_da_profile=no])

PKG_CHECK_MODULES([DALI], [dali2-core dali2-adaptor dali2-toolkit])

if test "x$with_da_profile" = "xno"; then
PKG_CHECK_MODULES(WAYLAND, libtbm)
PKG_CHECK_MODULES([WEB_ENGINE_LWE], [lightweight-web-engine-dali-plugin])
fi

devincludepath=${includedir}
AC_SUBST(devincludepath)

AC_CONFIG_FILES([
Makefile
])

AC_OUTPUT
