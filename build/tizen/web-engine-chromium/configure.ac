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

if test "x$with_tizen_90_or_greater" = "xyes"; then
  DALI_CFLAGS="$DALI_CFLAGS -DOVER_TIZEN_VERSION_9"
fi

TIZEN_WAYLAND_BACKEND_INIT

PKG_CHECK_MODULES([DALI], [dali2-core dali2-adaptor])
PKG_CHECK_MODULES(WAYLAND, libtbm)
PKG_CHECK_MODULES(WEB_ENGINE_CHROMIUM, chromium-efl)

if test "x$use_tcore_backend" = "xyes"; then
  PKG_CHECK_MODULES(TCORE_WL, tizen-core-wl)
  # The tcore backend reaches chromium-efl through the WV interface, which hands
  # back GLib containers (GList, GHashTable). chromium-efl.pc does not pull GLib
  # in, so ask for it here rather than relying on a transitive dependency.
  PKG_CHECK_MODULES(GLIB, glib-2.0)
else
  PKG_CHECK_MODULES(ELEMENTARY, elementary)
  PKG_CHECK_MODULES(ECORE_WL2, ecore-wl2)
fi

devincludepath=${includedir}
AC_SUBST(devincludepath)
AC_SUBST(DALI_CFLAGS)

AC_CONFIG_FILES([
Makefile
])

AC_OUTPUT
