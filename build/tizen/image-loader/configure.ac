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

AC_ARG_ENABLE([imageloader-extension],
              [AC_HELP_STRING([--enable-imageloader-extension],
              [enables the image loader extension of all the symbols in the library])],
              [enable_imageloader_extension=yes],
              [enable_imageloader_extension=no])

AM_CONDITIONAL([USE_IMAGELOADER_EXTENSION], [test x$enable_imageloader_extension = xyes])

PKG_CHECK_MODULES([DALI], [dali2-core dali2-adaptor dali2-toolkit])

devincludepath=${includedir}
AC_SUBST(devincludepath)
AC_SUBST(DALI_CFLAGS)

AC_CONFIG_FILES([
Makefile
])

AC_OUTPUT
