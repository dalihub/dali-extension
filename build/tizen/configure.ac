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

AC_ARG_WITH([tizen_55_or_greater],
            [AC_HELP_STRING([--with-tizen-55-or-greater],
                            [Use this conditional when Tizen version is 5.5 or greater])],
            [with_tizen_55_or_greater=yes],
            [with_tizen_55_or_greater=no])

PKG_CHECK_MODULES([DALI], [dali2-core dali2-adaptor dali2-toolkit])

AC_CONFIG_SUBDIRS(key)
AC_CONFIG_SUBDIRS(video-player)
AC_CONFIG_SUBDIRS(image-loader)
AC_CONFIG_SUBDIRS(color-controller)

if test "x$with_tizen_55_or_greater" = "xyes"; then
  AC_CONFIG_SUBDIRS(vector-animation-renderer)
  AC_CONFIG_SUBDIRS(web-engine-chromium)
  AC_CONFIG_SUBDIRS(web-engine-lwe)
fi
AC_CONFIG_SUBDIRS(dali-extension)

devincludepath=${includedir}
AC_SUBST(devincludepath)

AC_CONFIG_FILES([
Makefile
dali2-extension.pc
])

AC_OUTPUT
