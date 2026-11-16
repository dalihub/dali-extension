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

# The headers live under the versioned include directory (include/dali3),
# which only pkg-config knows; the default include path no longer finds them.
PKG_CHECK_MODULES([DALI], [dali3-core dali3-adaptor])

if test "x$enable_debug" = "xyes"; then
  DALI_CFLAGS="$DALI_CFLAGS -DDEBUG_ENABLED"
fi

AC_ARG_ENABLE([keyextension],
              [AC_HELP_STRING([--enable-keyextension],
              [enables the key extension of all the symbols in the library])],
              [enable_keyextension=yes],
              [enable_keyextension=no])

AM_CONDITIONAL([USE_KEY_EXTENSION], [test x$enable_keyextension = xyes])

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
