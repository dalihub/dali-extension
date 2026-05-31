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

PKG_CHECK_MODULES(DALI_ADAPTOR_INTEGRATION, dali2-adaptor-integration)

TIZEN_WAYLAND_BACKEND_INIT

if test "x$use_tcore_backend" != "xyes"; then
  case " $CFLAGS $CXXFLAGS " in
    *USE_TCORE_BACKEND*)
      use_tcore_backend=yes
      tizen_wayland_backend=TCORE
      DALI_CFLAGS="$DALI_CFLAGS -DUSE_TCORE_BACKEND"
      enable_web_engine_plugin=no
      ;;
  esac
fi

if test "x$use_tcore_backend" = "xyes"; then
  PKG_CHECK_MODULES(TCORE_WL, tizen-core-wl)
else
  PKG_CHECK_MODULES(ECORE_WL2, ecore-wl2)
fi

devincludepath=${includedir}
AC_SUBST(devincludepath)
AC_SUBST(DALI_CFLAGS)

AC_CONFIG_FILES([
Makefile
])

AC_OUTPUT
