AC_INIT([dali-media-frame-source], [0.1.0])
AM_INIT_AUTOMAKE([-Wall foreign subdir-objects])
AC_CONFIG_MACRO_DIRS([m4 ../m4])
AC_PROG_CXX
LT_INIT
AX_CXX_COMPILE_STDCXX([17], [noext], [mandatory])
PKG_CHECK_MODULES([DALI], [dali3-core dali3-adaptor dali3-adaptor-integration])
PKG_CHECK_MODULES([MEDIA_TOOL], [capi-media-tool libtbm])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
