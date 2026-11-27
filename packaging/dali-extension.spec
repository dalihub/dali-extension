# This source builds DALi 3 (dali3-extension and its plugins). On Tizen 10.1
# and 11, where applications are still moving from DALi 2.x, it also produces
# the DALi 2.x package names (dali2-extension-*) as alias packages: links to
# the dali3 files plus the Provides that 2.x binaries and specs depend on. The
# Tizen build is autotools, so the links are made at install time in this
# spec rather than by the CMake option the other repositories use. On Tizen 10.0 and
# below the real DALi 2.x is installed from its own source, so the aliases
# must never be built there. --with/--without dali2_aliases overrides the
# default for a local build.
#
# Nested %if, not %elif: Tizen's rpm (4.14) predates %elif and silently
# skips such a branch.
%define legacy_tizen (0%{?tizen_version_major} < 10 || (0%{?tizen_version_major} == 10 && 0%{?tizen_version_minor} == 0))
%if %{legacy_tizen}
%bcond_with    dali2_aliases
%else
%bcond_without dali2_aliases
%endif
%if %{with dali2_aliases} && %{legacy_tizen}
%{error:Tizen <= 10.0 ships the real DALi 2.x; dali2 aliases must not be built there}
%endif

# rpm derives no Provides from a symbolic link, so the alias packages declare
# the ELF ones themselves. 64-bit ELF provides carry a suffix.
%if "%{__isa_bits}" == "64"
%define elf_suffix ()(64bit)
%else
%define elf_suffix %{nil}
%endif

# NOTES
# This spec file is used to build DALi Extensions
#
# gbs will try to download the build.conf for the platform automatically from the repo location when
# performing a gbs build ( use gbs build -v to see it download location) E.g.
# http://download.tizen.org/snapshots/tizen/tv/tizen-tv/repos/arm-wayland/packages/repodata/xxxx-build.conf.gz

Name:       dali3-extension
Summary:    The DALi Tizen Extensions
Version:    3.0.2
Release:    1
Group:      System/Libraries
License:    Apache-2.0 and FSFAP
URL:        https://review.tizen.org/git/?p=platform/core/uifw/dali-extensions.git;a=summary
Source0:    %{name}-%{version}.tar.gz

%if 0%{?tizen_version_major} >= 4
%define tizen_40_or_greater 1
%endif

%if 0%{?tizen_version_major} >= 5
%define tizen_50_or_greater 1
%endif

%if ( 0%{?tizen_version_major} == 5 && 0%{?tizen_version_minor} >= 5 ) || 0%{?tizen_version_major} >= 6
%define tizen_55_or_greater 1
%endif

%if ( 0%{?tizen_version_major} == 6 && 0%{?tizen_version_minor} >= 5 ) || 0%{?tizen_version_major} >= 7
%define tizen_65_or_greater 1
%endif

%if 0%{?tizen_version_major} >= 9
%define tizen_90_or_greater 1
%endif

%if 0%{?tizen_version_major} >= 11
%define tizen_11_or_greater 1
%endif

# BUILD_TCORE_ENABLE: 0 (default) = ECORE, 1 = TCORE.
# The backend is decided by this build config only (regardless of the Tizen version).
# Example: gbs build ... --define "BUILD_TCORE_ENABLE 1"
# Resolve the backend here for both dependencies and build options.
%{!?BUILD_TCORE_ENABLE:%global BUILD_TCORE_ENABLE 0}
%global tizen_wayland_backend ECORE
%if "%{BUILD_TCORE_ENABLE}" != "0" && "%{BUILD_TCORE_ENABLE}" != "1"
%{error:BUILD_TCORE_ENABLE must be 0 or 1}
%endif
%if "%{BUILD_TCORE_ENABLE}" == "1"
%global tizen_wayland_backend TCORE
%endif

%if %{undefined NO_WEB_FRAMEWORK}
%define enable_web_engine_plugin 1
%endif

%if %{undefined enable_rive_animation_view}
%define enable_rive_animation_view 1
%endif

# Web engine plugins depend on EFL/chromium-efl for ECORE; wv for TCORE.
%if "%{tizen_wayland_backend}" == "TCORE"
%global enable_web_engine_plugin 1
%endif

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(dali3-core)
BuildRequires:  pkgconfig(dali3-adaptor)
BuildRequires:  dali3-integration-devel
BuildRequires:  dali3-adaptor-integration-devel
BuildRequires:  pkgconfig(dali3-adaptor-integration)
%if "%{tizen_wayland_backend}" == "TCORE"
BuildRequires:  pkgconfig(tizen-core-wl)
%else
BuildRequires:  pkgconfig(ecore-wl2)
%endif

%if 0%{?tizen_65_or_greater} && 0%{?enable_rive_animation_view} == 1
BuildRequires:  pkgconfig(dali3-toolkit)
BuildRequires:  dali3-toolkit-integration-devel
BuildRequires:  pkgconfig(thorvg)
BuildRequires:  pkgconfig(rive_tizen)
%endif

# For ASAN test
%if "%{vd_asan}" == "1" || "%{asan}" == "1"
BuildRequires: asan-force-options
BuildRequires: asan-build-env
BuildRequires: libasan
%endif

%description
dali-extension


#############################
# devel
##############################
%package devel
Summary:    Development components for the DALi Extension
Group:      Development/Building
Requires:   %{name} = %{version}-%{release}

%description devel
Development components for the DALi Tizen Extensions - public headers and package configs

##############################
# Dali Key Extension
##############################

%package key-extension
Summary:    Plugin to support extension keys for Dali
Group:      System/Libraries

%description key-extension
Plugin to support extension keys for Dali

##############################
# Dali VideoPlayer Plugin
##############################

%package video-player-plugin
Summary:    Plugin to play a video file for Dali
Group:      System/Libraries
BuildRequires: pkgconfig(capi-media-player)
BuildRequires: pkgconfig(capi-system-info)
BuildRequires: pkgconfig(esplusplayer)
%if "%{tizen_wayland_backend}" == "TCORE"
BuildRequires:  pkgconfig(tizen-core-wl)
%else
# dali-adaptor uses ecore mainloop
BuildRequires:  pkgconfig(ecore-wl2)
%endif

%if 0%{?tizen_90_or_greater}
BuildRequires: pkgconfig(wayland-egl)
%endif

%description video-player-plugin
VideoPlayer plugin to play a video file for Dali

##############################
# Dali MediaFrameSource Plugin
##############################

%if 0%{?tizen_11_or_greater}
%package media-frame-source-plugin
Summary: MMFW frame source bridge for DALi MediaView
Group: System/Libraries
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires: pkgconfig(capi-media-tool)
BuildRequires: pkgconfig(libtbm)
%description media-frame-source-plugin
Optional common media_frame_source bridge for Toolkit and Ui MediaView.
%endif

##############################
# Dali CameraPlayer Plugin
##############################

%if 0%{?tizen_65_or_greater}
%package camera-player-plugin
Summary:    Plugin to play a camera file for Dali
Group:      System/Libraries
BuildRequires: pkgconfig(capi-media-camera)
%if "%{tizen_wayland_backend}" == "TCORE"
BuildRequires:  pkgconfig(tizen-core-wl)
%else
# dali-adaptor uses ecore mainloop
BuildRequires:  pkgconfig(ecore-wl2)
%endif

%description camera-player-plugin
CameraPlayer plugin to play a camera file for Dali
%endif

##############################
# Dali Web Engine chromium Plugin
##############################

%package web-engine-chromium-plugin
Summary:    Plugin to support WebView for Dali
Group:      System/Libraries
%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
BuildRequires: pkgconfig(libtbm)
BuildRequires: pkgconfig(chromium-efl)
%if "%{tizen_wayland_backend}" == "TCORE"
# The tcore backend reaches chromium-efl through the WV interface, which hands
# back GLib containers, and it uses no EFL at all.
BuildRequires: pkgconfig(glib-2.0)
%else
BuildRequires: pkgconfig(elementary)
%endif
%endif

%description web-engine-chromium-plugin
Web Engine chromium plugin to support WebView for Dali

##############################
# Dali Image Loader Plugin
##############################

%package image-loader-plugin
Summary:    Plugin to image loading for Dali
Group:      System/Libraries

%description image-loader-plugin
Image Loader plugin to image loading file for Dali

####################################
# Vector Animation Renderer Plugin
####################################
%package vector-animation-renderer-plugin
Summary:    Plugin to render a vector animation
Group:      System/Libraries
%if 0%{?tizen_55_or_greater}
BuildRequires:  pkgconfig(rlottie)
%endif

%description vector-animation-renderer-plugin
Plugin to render a vector animation

##############################
# Dali Web Engine LWE Plugin
##############################

%package web-engine-lwe-plugin
Summary:    Plugin to support WebView for Dali
Group:      System/Libraries
%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
BuildRequires: pkgconfig(lightweight-web-engine)
BuildRequires: pkgconfig(libtbm)
BuildRequires: pkgconfig(egl)
BuildRequires: pkgconfig(vconf)
%endif

%description web-engine-lwe-plugin
Web Engine LWE(Light-weight Web Engine) plugin to support WebView for Dali

%if 0%{?tizen_65_or_greater} && 0%{?enable_rive_animation_view} == 1
####################################
# Rive Animation View Plugin
####################################
%package rive-animation-view
Summary:    Plugin to render a rive animation
Group:      System/Libraries
Requires: dali3-extension-rive-animation-view = %{version}-%{release}

%description rive-animation-view
Plugin to render a rive animation

%package rive-animation-view-devel
Summary: Development components for rive animation view
Group: System/Libraries
Requires: dali3-extension-rive-animation-view = %{version}-%{release}

%description rive-animation-view-devel
Header & package configuration of rive-animation-view
%endif

##############################
# Dali ICU Plugin
##############################

%if 0%{?tizen_65_or_greater}
%package icu-plugin
Summary:    Plugin to use an International Components for Unicode for Dali
Group:      System/Libraries
BuildRequires: pkgconfig(icu-i18n)
BuildRequires: pkgconfig(icu-uc)

%description icu-plugin
ICU plugin to use an International Components for Unicode for Dali
%endif


%if %{with dali2_aliases}
##############################
# DALi 2.x names (aliases)
##############################
# Name-only where the 2.x package only carried files the dali3 package now
# provides under its own name (the login-shell scripts); otherwise links to
# the dali3 plugin.
%package -n dali2-extension
Summary:    DALi 2.x name for the DALi Tizen Extensions
Group:      System/Libraries
Requires:   %{name} = %{version}-%{release}

%description -n dali2-extension
Name-only package for the DALi 2.x extension package name; pulls in dali3-extension.

%package -n dali2-extension-devel
Summary:    DALi 2.x development names for the DALi Extension
Group:      Development/Building
Requires:   dali2-extension = %{version}-%{release}
Requires:   %{name}-devel = %{version}-%{release}

%description -n dali2-extension-devel
include/dali-extension and dali2-extension.pc as links to their dali3 counterparts.

%package -n dali2-extension-key-extension
Summary:    DALi 2.x name for the key extension plugin
Group:      System/Libraries
Requires:   %{name}-key-extension = %{version}-%{release}
Provides:   libdali2-key-extension.so.0%{elf_suffix}

%description -n dali2-extension-key-extension
libdali2-key-extension.so as a link to the dali3 plugin.

%package -n dali2-extension-video-player-plugin
Summary:    DALi 2.x name for the video player plugin
Group:      System/Libraries
Requires:   %{name}-video-player-plugin = %{version}-%{release}
Provides:   libdali2-video-player-plugin.so.0%{elf_suffix}

%description -n dali2-extension-video-player-plugin
libdali2-video-player-plugin.so as a link to the dali3 plugin.

%if 0%{?tizen_11_or_greater}
%package -n dali2-extension-media-frame-source-plugin
Summary:    DALi 2.x name for the media frame source plugin
Group:      System/Libraries
Requires:   %{name}-media-frame-source-plugin = %{version}-%{release}
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description -n dali2-extension-media-frame-source-plugin
libdali2-media-frame-source-plugin.so as a link to the dali3 plugin.
%endif

%if 0%{?tizen_65_or_greater}
%package -n dali2-extension-camera-player-plugin
Summary:    DALi 2.x name for the camera player plugin
Group:      System/Libraries
Requires:   %{name}-camera-player-plugin = %{version}-%{release}
Provides:   libdali2-camera-player-plugin.so.0%{elf_suffix}

%description -n dali2-extension-camera-player-plugin
libdali2-camera-player-plugin.so as a link to the dali3 plugin.
%endif

%package -n dali2-extension-web-engine-chromium-plugin
Summary:    DALi 2.x name for the web engine chromium plugin
Group:      System/Libraries
Requires:   %{name}-web-engine-chromium-plugin = %{version}-%{release}
Provides:   libdali2-web-engine-chromium-plugin.so.0%{elf_suffix}

%description -n dali2-extension-web-engine-chromium-plugin
libdali2-web-engine-chromium-plugin.so as a link to the dali3 plugin.

%package -n dali2-extension-image-loader-plugin
Summary:    DALi 2.x name for the image loader plugin
Group:      System/Libraries
Requires:   %{name}-image-loader-plugin = %{version}-%{release}

%description -n dali2-extension-image-loader-plugin
libdali2-image-loader-plugin.so as a link to the dali3 plugin.

%package -n dali2-extension-vector-animation-renderer-plugin
Summary:    DALi 2.x name for the vector animation renderer plugin
Group:      System/Libraries
Requires:   %{name}-vector-animation-renderer-plugin = %{version}-%{release}
Provides:   libdali2-vector-animation-renderer-plugin.so.0%{elf_suffix}

%description -n dali2-extension-vector-animation-renderer-plugin
libdali2-vector-animation-renderer-plugin.so as a link to the dali3 plugin.

%package -n dali2-extension-web-engine-lwe-plugin
Summary:    DALi 2.x name for the web engine LWE plugin
Group:      System/Libraries
Requires:   %{name}-web-engine-lwe-plugin = %{version}-%{release}
Provides:   libdali2-web-engine-lwe-plugin.so.0%{elf_suffix}

%description -n dali2-extension-web-engine-lwe-plugin
libdali2-web-engine-lwe-plugin.so as a link to the dali3 plugin.

%if 0%{?tizen_65_or_greater} && 0%{?enable_rive_animation_view} == 1
%package -n dali2-extension-rive-animation-view
Summary:    DALi 2.x name for the rive animation view
Group:      System/Libraries
Requires:   %{name}-rive-animation-view = %{version}-%{release}
Provides:   libdali2-extension-rive-animation-view.so.0%{elf_suffix}

%description -n dali2-extension-rive-animation-view
libdali2-extension-rive-animation-view.so as a link to the dali3 library.

%package -n dali2-extension-rive-animation-view-devel
Summary:    DALi 2.x development name for the rive animation view
Group:      System/Libraries
Requires:   dali2-extension-rive-animation-view = %{version}-%{release}
Requires:   dali2-extension-devel = %{version}-%{release}
Requires:   %{name}-rive-animation-view-devel = %{version}-%{release}

%description -n dali2-extension-rive-animation-view-devel
dali2-extension-rive-animation-view.pc as a link to its dali3 counterpart; the headers are reached through the include/dali-extension link of dali2-extension-devel.
%endif

%if 0%{?tizen_65_or_greater}
%package -n dali2-extension-icu-plugin
Summary:    DALi 2.x name for the ICU plugin
Group:      System/Libraries
Requires:   %{name}-icu-plugin = %{version}-%{release}
Provides:   libdali2-icu-plugin.so.0%{elf_suffix}

%description -n dali2-extension-icu-plugin
libdali2-icu-plugin.so as a link to the dali3 plugin.
%endif
%endif

##############################
# Preparation
##############################
%prep
%setup -q

##############################
# Build
##############################
%build
# Keep shadow diagnostics enabled independently of the build server policy.
export CFLAGS="${CFLAGS:-%optflags} -Werror=shadow"
export CXXFLAGS="${CXXFLAGS:-%optflags} -Werror=shadow"

PREFIX="${PREFIX}/usr"
# GBS qemu-user builds use a host liblto_plugin.so (ELFCLASS64) via /emul;
# disable the linker plugin so configure/make link tests succeed on armv7l.
# -fno-use-linker-plugin is GCC-only; clang does not support it and errors
# with [-Werror,-Wignored-optimization-argument].
# Skip the flag when the compiler is clang (detected via $CC at build time).
if echo "${CC:-gcc}" | grep -q "clang"; then
  : # clang: skip -fno-use-linker-plugin
else
  CFLAGS="$CFLAGS -fno-use-linker-plugin"
  CXXFLAGS="$CXXFLAGS -fno-use-linker-plugin"
fi
CXXFLAGS="$CXXFLAGS -Wall -g -Os -fPIC -fvisibility-inlines-hidden -fdata-sections -ffunction-sections -DGL_GLEXT_PROTOTYPES"
LDFLAGS="$LDFLAGS -Wl,--rpath=%{_libdir} -Wl,--as-needed -Wl,--gc-sections -Wl,-Bsymbolic-functions "

%if 0%{?tizen_50_or_greater}
%if "%{tizen_wayland_backend}" == "TCORE"
CFLAGS="$CFLAGS  -DUSE_TCORE_BACKEND"
CXXFLAGS="$CXXFLAGS  -DUSE_TCORE_BACKEND"
%else
CFLAGS="$CFLAGS  -DECORE_WL2 -DEFL_BETA_API_SUPPORT"
CXXFLAGS="$CXXFLAGS  -DECORE_WL2 -DEFL_BETA_API_SUPPORT"
%endif
%endif

%if "%{?profile}" == "tv"
CFLAGS="$CFLAGS  -DOS_TIZEN_TV"
CXXFLAGS="$CXXFLAGS  -DOS_TIZEN_TV"
%endif

%if "%{vd_asan}" == "1" || "%{asan}" == "1"
CFLAGS="$CFLAGS  -fsanitize=address"
CXXFLAGS="$CXXFLAGS  -fsanitize=address"
LDFLAGS="$LDFLAGS  -fsanitize=address"
%endif

libtoolize --force
cd %{_builddir}/%{name}-%{version}/build/tizen
autoreconf --install
_saved_dir=$(pwd)
cd dali-extension
autoreconf --install
cd "$_saved_dir"

%configure --prefix=$PREFIX \
%if 0%{?enable_debug}
           --enable-debug \
%endif
%if 0%{?tizen_50_or_greater}
           --with-tizen-50-or-greater \
%endif
%if 0%{?tizen_55_or_greater}
           --with-tizen-55-or-greater \
%endif
%if 0%{?tizen_65_or_greater}
           --with-tizen-65-or-greater \
%endif
%if 0%{?tizen_90_or_greater}
           --with-tizen-90-or-greater \
%endif
%if 0%{?tizen_11_or_greater}
           --with-tizen-11-or-greater \
%endif
%if 0%{?enable_web_engine_plugin} == 1
           --enable-web-engine-plugin \
%endif
%if 0%{?enable_image_loader}
           --enable-imageloader-extension \
%endif
%if 0%{?enable_rive_animation_view} == 1
           --enable-rive-animation-view \
%else
           --disable-rive-animation-view \
%endif
           --with-tizen-wayland-backend=%{tizen_wayland_backend} \
           --enable-keyextension

make %{?jobs:-j%jobs}

##############################
# Installation
##############################
%install
rm -rf %{buildroot}

# Login-shell scripts under versioned names, so that they can sit beside the
# scripts of a DALi 2.x built from different code; the shader-cache script
# points at the dali3 directories (share/dali3, .cache/dali3_common_caches).
mkdir -p %{buildroot}%{_sysconfdir}/profile.d

%if "%{?profile}" != "common" && "%{?profile}" != "ivi"
dali_sh=scripts/dali.sh
%endif
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "mobile"
dali_sh=scripts/other/dali.sh
%endif

install -m 0644 ${dali_sh} %{buildroot}%{_sysconfdir}/profile.d/dali3.sh
sed -e 's|/usr/share/dali/|/usr/share/dali3/|' -e 's|dali_common_caches|dali3_common_caches|' \
    scripts/dali-shader-conf.sh > %{buildroot}%{_sysconfdir}/profile.d/dali3-shader-conf.sh
chmod 0644 %{buildroot}%{_sysconfdir}/profile.d/dali3-shader-conf.sh

cd %{_builddir}/%{name}-%{version}/build/tizen
%make_install

%if %{with dali2_aliases}
# DALi 2.x names as links to the dali3 files just installed. Only what the
# build produced is linked, so the plugin conditions above need no repeating.
legacy_lib() {
  local legacy=$1 current=$2 soversion=$3
  if [ -e %{buildroot}%{_libdir}/lib${current}.so ]; then
    ln -sf lib${current}.so %{buildroot}%{_libdir}/lib${legacy}.so
    if [ -n "${soversion}" ]; then
      ln -sf lib${current}.so.${soversion} %{buildroot}%{_libdir}/lib${legacy}.so.${soversion}
    fi
  fi
}
legacy_lib dali2-key-extension                    dali3-key-extension                    0
legacy_lib dali2-video-player-plugin              dali3-video-player-plugin              0
legacy_lib dali2-media-frame-source-plugin        dali3-media-frame-source-plugin
legacy_lib dali2-camera-player-plugin             dali3-camera-player-plugin             0
legacy_lib dali2-web-engine-chromium-plugin       dali3-web-engine-chromium-plugin       0
legacy_lib dali2-image-loader-plugin              dali3-image-loader-plugin
legacy_lib dali2-vector-animation-renderer-plugin dali3-vector-animation-renderer-plugin 0
legacy_lib dali2-web-engine-lwe-plugin            dali3-web-engine-lwe-plugin            0
legacy_lib dali2-extension-rive-animation-view    dali3-extension-rive-animation-view    0
legacy_lib dali2-icu-plugin                       dali3-icu-plugin                       0
ln -sf dali3/dali-extension %{buildroot}%{_includedir}/dali-extension
# Copies, not links, so that rpm derives pkgconfig(dali2-*) from them; the
# content already names the dali3 include directory.
cp %{buildroot}%{_libdir}/pkgconfig/dali3-extension.pc %{buildroot}%{_libdir}/pkgconfig/dali2-extension.pc
if [ -e %{buildroot}%{_libdir}/pkgconfig/dali3-extension-rive-animation-view.pc ]; then
  cp %{buildroot}%{_libdir}/pkgconfig/dali3-extension-rive-animation-view.pc %{buildroot}%{_libdir}/pkgconfig/dali2-extension-rive-animation-view.pc
fi
%endif

%pre
exit 0

##############################
#  Post Install new package
##############################
%post
/sbin/ldconfig
exit 0

%post key-extension
/sbin/ldconfig
exit 0

%post video-player-plugin
/sbin/ldconfig
exit 0

%if 0%{?tizen_11_or_greater}
%post media-frame-source-plugin -p /sbin/ldconfig
%endif

%if 0%{?tizen_65_or_greater}
%post camera-player-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_65_or_greater}
%post icu-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%post web-engine-chromium-plugin
_saved_dir=$(pwd)
cd %{_libdir}
ln -sf libdali3-web-engine-chromium-plugin.so libdali3-web-engine-plugin.so
cd "$_saved_dir"
/sbin/ldconfig
exit 0
%endif

%if 0%{?enable_image_loader}
%post image-loader-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_55_or_greater}
%post vector-animation-renderer-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_65_or_greater} && 0%{?enable_rive_animation_view} == 1
%post rive-animation-view
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%post web-engine-lwe-plugin
/sbin/ldconfig
exit 0
%endif

##############################
#   Pre Uninstall old package
##############################
%preun
exit 0

##############################
#   Post Uninstall old package
##############################
%postun
/sbin/ldconfig
exit 0

%postun key-extension
/sbin/ldconfig
exit 0

%postun video-player-plugin
/sbin/ldconfig
exit 0

%if 0%{?tizen_11_or_greater}
%postun media-frame-source-plugin -p /sbin/ldconfig
%endif

%if 0%{?tizen_65_or_greater}
%postun camera-player-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_65_or_greater}
%postun icu-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%postun web-engine-chromium-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?enable_image_loader}
%postun image-loader-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_55_or_greater}
%postun vector-animation-renderer-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_65_or_greater} && 0%{?enable_rive_animation_view} == 1
%postun rive-animation-view
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%postun web-engine-lwe-plugin
/sbin/ldconfig
exit 0
%endif

%if %{with dali2_aliases}
%post -n dali2-extension-key-extension
/sbin/ldconfig
exit 0

%postun -n dali2-extension-key-extension
/sbin/ldconfig
exit 0

%post -n dali2-extension-video-player-plugin
/sbin/ldconfig
exit 0

%postun -n dali2-extension-video-player-plugin
/sbin/ldconfig
exit 0

%if 0%{?tizen_11_or_greater}
%post -n dali2-extension-media-frame-source-plugin -p /sbin/ldconfig
%postun -n dali2-extension-media-frame-source-plugin -p /sbin/ldconfig
%endif

%if 0%{?tizen_65_or_greater}
%post -n dali2-extension-camera-player-plugin
/sbin/ldconfig
exit 0

%postun -n dali2-extension-camera-player-plugin
/sbin/ldconfig
exit 0

%post -n dali2-extension-icu-plugin
/sbin/ldconfig
exit 0

%postun -n dali2-extension-icu-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%post -n dali2-extension-web-engine-chromium-plugin
/sbin/ldconfig
exit 0

%postun -n dali2-extension-web-engine-chromium-plugin
/sbin/ldconfig
exit 0

%post -n dali2-extension-web-engine-lwe-plugin
/sbin/ldconfig
exit 0

%postun -n dali2-extension-web-engine-lwe-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?enable_image_loader}
%post -n dali2-extension-image-loader-plugin
/sbin/ldconfig
exit 0

%postun -n dali2-extension-image-loader-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_55_or_greater}
%post -n dali2-extension-vector-animation-renderer-plugin
/sbin/ldconfig
exit 0

%postun -n dali2-extension-vector-animation-renderer-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_65_or_greater} && 0%{?enable_rive_animation_view} == 1
%post -n dali2-extension-rive-animation-view
/sbin/ldconfig
exit 0

%postun -n dali2-extension-rive-animation-view
/sbin/ldconfig
exit 0
%endif
%endif

##############################
# Files in Binary Packages
##############################
# The Lua scriptlet runs up to the next section, so nothing but %files may
# follow it. Every earlier package installed include/dali-extension as a real
# directory; rpm cannot replace a directory with a link, so the alias package
# clears it before the transaction unpacks.
%if %{with dali2_aliases}
%pretrans -n dali2-extension-devel -p <lua>
local path = "%{_includedir}/dali-extension"
if posix.stat(path, "type") == "directory" then
  os.execute("rm -rf '" .. path .. "'")
end
%endif
%files
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_sysconfdir}/profile.d/dali3.sh
%{_sysconfdir}/profile.d/dali3-shader-conf.sh
%license LICENSE

%files devel
%defattr(-,root,root,-)
%{_includedir}/dali3/dali-extension/*
%{_libdir}/pkgconfig/dali3-extension.pc

%files key-extension
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali3-key-extension.so*
%license LICENSE

%files video-player-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali3-video-player-plugin.so*
%license LICENSE

%if 0%{?tizen_65_or_greater}
%files camera-player-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali3-camera-player-plugin.so*
%license LICENSE
%endif

%if 0%{?tizen_65_or_greater}
%files icu-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali3-icu-plugin.so*
%license LICENSE
%endif

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%files web-engine-chromium-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali3-web-engine-chromium-plugin.so*
%license LICENSE
%endif

%if 0%{?enable_image_loader}
%files image-loader-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali3-image-loader-plugin.so*
%license LICENSE
%endif

%if 0%{?tizen_55_or_greater}
%files vector-animation-renderer-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali3-vector-animation-renderer-plugin.so*
%license LICENSE
%endif

%if 0%{?tizen_65_or_greater} && 0%{?enable_rive_animation_view} == 1
%files rive-animation-view
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali3-extension-rive-animation-view.so*
%license LICENSE

%files rive-animation-view-devel
%defattr(-,root,root,-)
%{_includedir}/dali3/dali-extension/devel-api/rive-animation-view/*
%{_libdir}/pkgconfig/dali3-extension-rive-animation-view.pc
%endif

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%files web-engine-lwe-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali3-web-engine-lwe-plugin.so*
%license LICENSE
%endif

%if %{with dali2_aliases}
%files -n dali2-extension
%defattr(-,root,root,-)

%files -n dali2-extension-devel
%defattr(-,root,root,-)
%{_includedir}/dali-extension
%{_libdir}/pkgconfig/dali2-extension.pc

%files -n dali2-extension-key-extension
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-key-extension.so
%{_libdir}/libdali2-key-extension.so.0

%files -n dali2-extension-video-player-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-video-player-plugin.so
%{_libdir}/libdali2-video-player-plugin.so.0

%if 0%{?tizen_65_or_greater}
%files -n dali2-extension-camera-player-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-camera-player-plugin.so
%{_libdir}/libdali2-camera-player-plugin.so.0

%files -n dali2-extension-icu-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-icu-plugin.so
%{_libdir}/libdali2-icu-plugin.so.0
%endif

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%files -n dali2-extension-web-engine-chromium-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-web-engine-chromium-plugin.so
%{_libdir}/libdali2-web-engine-chromium-plugin.so.0

%files -n dali2-extension-web-engine-lwe-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-web-engine-lwe-plugin.so
%{_libdir}/libdali2-web-engine-lwe-plugin.so.0
%endif

%if 0%{?enable_image_loader}
%files -n dali2-extension-image-loader-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-image-loader-plugin.so
%endif

%if 0%{?tizen_55_or_greater}
%files -n dali2-extension-vector-animation-renderer-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-vector-animation-renderer-plugin.so
%{_libdir}/libdali2-vector-animation-renderer-plugin.so.0
%endif

%if 0%{?tizen_65_or_greater} && 0%{?enable_rive_animation_view} == 1
%files -n dali2-extension-rive-animation-view
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-extension-rive-animation-view.so
%{_libdir}/libdali2-extension-rive-animation-view.so.0

%files -n dali2-extension-rive-animation-view-devel
%defattr(-,root,root,-)
%{_libdir}/pkgconfig/dali2-extension-rive-animation-view.pc
%endif
%endif

%if 0%{?tizen_11_or_greater}
%files media-frame-source-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali3-media-frame-source-plugin.so*
%license LICENSE

%if %{with dali2_aliases}
%files -n dali2-extension-media-frame-source-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-media-frame-source-plugin.so
%endif
%endif
