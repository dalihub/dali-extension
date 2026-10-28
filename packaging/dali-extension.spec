# dali2 is the same code built a second time under the dali2 name, for
# applications that still use it. To drop it, remove the "with dali2" blocks.
%bcond_without dali2

# NOTES
# This spec file is used to build DALi Extensions
#
# gbs will try to download the build.conf for the platform automatically from the repo location when
# performing a gbs build ( use gbs build -v to see it download location) E.g.
# http://download.tizen.org/snapshots/tizen/tv/tizen-tv/repos/arm-wayland/packages/repodata/xxxx-build.conf.gz

Name:       dali3-extension
Summary:    The DALi Tizen Extensions
Version:    2.5.45
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

# BUILD_TCORE_ENABLE: 0 (default) = ECORE, 1 = TCORE, for Tizen 11 only.
# Example: gbs build ... --define "BUILD_TCORE_ENABLE 1"
# Resolve the backend here for both dependencies and build options.
%{!?BUILD_TCORE_ENABLE:%global BUILD_TCORE_ENABLE 0}
%global tizen_wayland_backend ECORE
%if 0%{?tizen_version_major} == 11
%if "%{BUILD_TCORE_ENABLE}" != "0" && "%{BUILD_TCORE_ENABLE}" != "1"
%{error:BUILD_TCORE_ENABLE must be 0 or 1}
%endif
%if "%{BUILD_TCORE_ENABLE}" == "1"
%global tizen_wayland_backend TCORE
%endif
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
BuildRequires:  pkgconfig(dali3-core)
BuildRequires:  pkgconfig(dali3-adaptor)
BuildRequires:  dali3-integration-devel
BuildRequires:  dali3-adaptor-integration-devel
BuildRequires:  pkgconfig(dlog)

BuildRequires:  pkgconfig(dali3-adaptor-integration)
%if %{with dali2}
BuildRequires:  pkgconfig(dali2-core)
BuildRequires:  pkgconfig(dali2-adaptor)
BuildRequires:  dali2-integration-devel
BuildRequires:  dali2-adaptor-integration-devel
BuildRequires:  pkgconfig(dali2-adaptor-integration)
%endif
%if "%{tizen_wayland_backend}" == "TCORE"
BuildRequires:  pkgconfig(tizen-core-wl)
%else
BuildRequires:  pkgconfig(ecore-wl2)
%endif

%if 0%{?tizen_65_or_greater} && 0%{?enable_rive_animation_view} == 1
BuildRequires:  pkgconfig(dali3-toolkit)
BuildRequires:  dali3-toolkit-integration-devel
%if %{with dali2}
BuildRequires:  pkgconfig(dali2-toolkit)
BuildRequires:  dali2-toolkit-integration-devel
%endif
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

%if %{with dali2}
##############################
# dali2
##############################
%package -n dali2-extension
Summary:    The DALi Tizen Extensions
Group:      System/Libraries
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description -n dali2-extension
dali-extension - the same code as %{name}, under the dali2 name

%package -n dali2-extension-devel
Summary:    Development components for the DALi Extension
Group:      Development/Building
Requires:   dali2-extension = %{version}-%{release}

%description -n dali2-extension-devel
Development components for the DALi Tizen Extensions - public headers and package configs

%package -n dali2-extension-key-extension
Summary:    Plugin to support extension keys for Dali
Group:      System/Libraries

%description -n dali2-extension-key-extension
Plugin to support extension keys for Dali

%package -n dali2-extension-video-player-plugin
Summary:    Plugin to play a video file for Dali
Group:      System/Libraries

%description -n dali2-extension-video-player-plugin
VideoPlayer plugin to play a video file for Dali

%if 0%{?tizen_65_or_greater}
%package -n dali2-extension-camera-player-plugin
Summary:    Plugin to play a camera file for Dali
Group:      System/Libraries

%description -n dali2-extension-camera-player-plugin
CameraPlayer plugin to play a camera file for Dali
%endif

%package -n dali2-extension-web-engine-chromium-plugin
Summary:    Plugin to support WebView for Dali
Group:      System/Libraries

%description -n dali2-extension-web-engine-chromium-plugin
Web Engine chromium plugin to support WebView for Dali

%package -n dali2-extension-image-loader-plugin
Summary:    Plugin to image loading for Dali
Group:      System/Libraries

%description -n dali2-extension-image-loader-plugin
Image Loader plugin to image loading file for Dali

%package -n dali2-extension-vector-animation-renderer-plugin
Summary:    Plugin to render a vector animation
Group:      System/Libraries

%description -n dali2-extension-vector-animation-renderer-plugin
Plugin to render a vector animation

%package -n dali2-extension-web-engine-lwe-plugin
Summary:    Plugin to support WebView for Dali
Group:      System/Libraries

%description -n dali2-extension-web-engine-lwe-plugin
Web Engine LWE(Light-weight Web Engine) plugin to support WebView for Dali

%if 0%{?tizen_65_or_greater} && 0%{?enable_rive_animation_view} == 1
%package -n dali2-extension-rive-animation-view
Summary:    Plugin to render a rive animation
Group:      System/Libraries
Requires: dali2-extension-rive-animation-view = %{version}-%{release}

%description -n dali2-extension-rive-animation-view
Plugin to render a rive animation

%package -n dali2-extension-rive-animation-view-devel
Summary: Development components for rive animation view
Group: System/Libraries
Requires: dali2-extension-rive-animation-view = %{version}-%{release}

%description -n dali2-extension-rive-animation-view-devel
Header & package configuration of rive-animation-view
%endif

%if 0%{?tizen_65_or_greater}
%package -n dali2-extension-icu-plugin
Summary:    Plugin to use an International Components for Unicode for Dali
Group:      System/Libraries

%description -n dali2-extension-icu-plugin
ICU plugin to use an International Components for Unicode for Dali
%endif
%endif

##############################
# Preparation
##############################
%prep
%setup -q

#Use TZ_PATH when tizen version is 3.x or greater

%define dali_data_rw_dir         %TZ_SYS_RO_SHARE/dali/
%define dali_data_ro_dir         %TZ_SYS_RO_SHARE/dali/
%define dev_include_path %{_includedir}

%if %{with dali2}
%define dali2_src_dir %{_builddir}/%{name}-%{version}/_build/dali2
%endif

##############################
# Build
##############################
%build
%if %{with dali2}
# The autotools build runs in the source tree and names its libraries in
# Makefile.am, so dali2 is built in a pristine copy of the tree whose build
# files have the dali3 names switched to dali2.
mkdir -p %{dali2_src_dir}
tar --exclude=./_build -cf - . | tar -xf - -C %{dali2_src_dir}
find %{dali2_src_dir}/build/tizen \( -name configure.ac -o -name Makefile.am -o -name '*.pc.in' \) -exec sed -i 's/dali3/dali2/g' {} +
%endif

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

%if %{with dali2}
# Keep the steps and options the same as dali3 above.
cd %{dali2_src_dir}
libtoolize --force
cd build/tizen
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
%endif

##############################
# Installation
##############################
%install
rm -rf %{buildroot}

# install dali.sh
mkdir -p %{buildroot}%{_sysconfdir}/profile.d

%if "%{?profile}" != "common" && "%{?profile}" != "ivi"
install -m 0644 scripts/dali.sh %{buildroot}%{_sysconfdir}/profile.d
%endif

%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "mobile"
install -m 0644 scripts/other/dali.sh %{buildroot}%{_sysconfdir}/profile.d
%endif

# install dali-shader-conf.sh
install -m 0644 scripts/dali-shader-conf.sh %{buildroot}%{_sysconfdir}/profile.d

cd build/tizen
%make_install DALI_DATA_RW_DIR="%{dali_data_rw_dir}" DALI_DATA_RO_DIR="%{dali_data_ro_dir}"

%if %{with dali2}
cd %{dali2_src_dir}/build/tizen
%make_install DALI_DATA_RW_DIR="%{dali_data_rw_dir}" DALI_DATA_RO_DIR="%{dali_data_ro_dir}"
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

%if %{with dali2}
%post -n dali2-extension
/sbin/ldconfig
exit 0

%post -n dali2-extension-key-extension
/sbin/ldconfig
exit 0

%post -n dali2-extension-video-player-plugin
/sbin/ldconfig
exit 0

%if 0%{?tizen_65_or_greater}
%post -n dali2-extension-camera-player-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_65_or_greater}
%post -n dali2-extension-icu-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%post -n dali2-extension-web-engine-chromium-plugin
_saved_dir=$(pwd)
cd %{_libdir}
ln -sf libdali2-web-engine-chromium-plugin.so libdali2-web-engine-plugin.so
cd "$_saved_dir"
/sbin/ldconfig
exit 0
%endif

%if 0%{?enable_image_loader}
%post -n dali2-extension-image-loader-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_55_or_greater}
%post -n dali2-extension-vector-animation-renderer-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_65_or_greater} && 0%{?enable_rive_animation_view} == 1
%post -n dali2-extension-rive-animation-view
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%post -n dali2-extension-web-engine-lwe-plugin
/sbin/ldconfig
exit 0
%endif

%postun -n dali2-extension
/sbin/ldconfig
exit 0

%postun -n dali2-extension-key-extension
/sbin/ldconfig
exit 0

%postun -n dali2-extension-video-player-plugin
/sbin/ldconfig
exit 0

%if 0%{?tizen_65_or_greater}
%postun -n dali2-extension-camera-player-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_65_or_greater}
%postun -n dali2-extension-icu-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%postun -n dali2-extension-web-engine-chromium-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?enable_image_loader}
%postun -n dali2-extension-image-loader-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_55_or_greater}
%postun -n dali2-extension-vector-animation-renderer-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_65_or_greater} && 0%{?enable_rive_animation_view} == 1
%postun -n dali2-extension-rive-animation-view
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%postun -n dali2-extension-web-engine-lwe-plugin
/sbin/ldconfig
exit 0
%endif
%endif

##############################
# Files in Binary Packages
##############################

%files
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_sysconfdir}/profile.d/dali.sh
%{_sysconfdir}/profile.d/dali-shader-conf.sh
%license LICENSE

%files devel
%defattr(-,root,root,-)
%{_includedir}/dali-extension/*
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
%{_includedir}/dali-extension/devel-api/rive-animation-view/*
%{_libdir}/pkgconfig/dali3-extension-rive-animation-view.pc
%endif

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%files web-engine-lwe-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali3-web-engine-lwe-plugin.so*
%license LICENSE
%endif

%if %{with dali2}
# Both builds install the same headers, and the profile scripts are installed
# once, so dali2 shares them with dali3.
%files -n dali2-extension
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_sysconfdir}/profile.d/dali.sh
%{_sysconfdir}/profile.d/dali-shader-conf.sh
%license LICENSE

%files -n dali2-extension-devel
%defattr(-,root,root,-)
%{_includedir}/dali-extension/*
%{_libdir}/pkgconfig/dali2-extension.pc

%files -n dali2-extension-key-extension
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-key-extension.so*
%license LICENSE

%files -n dali2-extension-video-player-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-video-player-plugin.so*
%license LICENSE

%if 0%{?tizen_65_or_greater}
%files -n dali2-extension-camera-player-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-camera-player-plugin.so*
%license LICENSE
%endif

%if 0%{?tizen_65_or_greater}
%files -n dali2-extension-icu-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-icu-plugin.so*
%license LICENSE
%endif

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%files -n dali2-extension-web-engine-chromium-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-web-engine-chromium-plugin.so*
%license LICENSE
%endif

%if 0%{?enable_image_loader}
%files -n dali2-extension-image-loader-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-image-loader-plugin.so*
%license LICENSE
%endif

%if 0%{?tizen_55_or_greater}
%files -n dali2-extension-vector-animation-renderer-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-vector-animation-renderer-plugin.so*
%license LICENSE
%endif

%if 0%{?tizen_65_or_greater} && 0%{?enable_rive_animation_view} == 1
%files -n dali2-extension-rive-animation-view
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-extension-rive-animation-view.so*
%license LICENSE

%files -n dali2-extension-rive-animation-view-devel
%defattr(-,root,root,-)
%{_includedir}/dali-extension/devel-api/rive-animation-view/*
%{_libdir}/pkgconfig/dali2-extension-rive-animation-view.pc
%endif

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%files -n dali2-extension-web-engine-lwe-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-web-engine-lwe-plugin.so*
%license LICENSE
%endif
%endif
