# NOTES
# This spec file is used to build DALi Extensions
#
# gbs will try to download the build.conf for the platform automatically from the repo location when
# performing a gbs build ( use gbs build -v to see it download location) E.g.
# http://download.tizen.org/snapshots/tizen/tv/tizen-tv/repos/arm-wayland/packages/repodata/xxxx-build.conf.gz

Name:       dali2-extension
Summary:    The DALi Tizen Extensions
Version:    2.3.41
Release:    1
Group:      System/Libraries
License:    Apache-2.0 and BSD-3-Clause and MIT
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

%if %{undefined NO_WEB_FRAMEWORK}
%define enable_web_engine_plugin 1
%endif

%define enable_color_controller 1

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(dali2-core)
BuildRequires:  pkgconfig(dali2-adaptor)
BuildRequires:  pkgconfig(dali2-toolkit)
BuildRequires:  pkgconfig(dlog)

BuildRequires:  pkgconfig(dali2-adaptor-integration)
BuildRequires:  pkgconfig(ecore-wl2)

# For evas-plugin
%if 0%{?enable_evas_plugin}
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(evas)
%endif

%if 0%{?tizen_65_or_greater}
BuildRequires:  pkgconfig(rive_tizen)
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
BuildRequires: pkgconfig(wayland-egl)
# dali-adaptor uses ecore mainloop
%if 0%{?tizen_version_major} >= 5
BuildRequires:  pkgconfig(ecore-wl2)
%else
BuildRequires:  pkgconfig(ecore-wayland)
%endif

%description video-player-plugin
VideoPlayer plugin to play a video file for Dali

##############################
# Dali CameraPlayer Plugin
##############################

%package camera-player-plugin
Summary:    Plugin to play a camera file for Dali
Group:      System/Libraries
BuildRequires: pkgconfig(capi-media-camera)
# dali-adaptor uses ecore mainloop
BuildRequires:  pkgconfig(ecore-wl2)

%description camera-player-plugin
CameraPlayer plugin to play a camera file for Dali

##############################
# Dali Web Engine chromium Plugin
##############################

%package web-engine-chromium-plugin
Summary:    Plugin to support WebView for Dali
Group:      System/Libraries
%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
BuildRequires: pkgconfig(libtbm)
BuildRequires: pkgconfig(chromium-efl)
BuildRequires: pkgconfig(elementary)
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

####################################
# color controller Plugin
####################################

%package color-controller-plugin
Summary:    Plugin to load color theme
Group:      System/Libraries

%description color-controller-plugin
Plugin to load color theme

##############################
# Dali Web Engine LWE Plugin
##############################

%package web-engine-lwe-plugin
Summary:    Plugin to support WebView for Dali
Group:      System/Libraries
%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
BuildRequires: pkgconfig(libtbm)
BuildRequires: pkgconfig(lightweight-web-engine)
%endif

%description web-engine-lwe-plugin
Web Engine LWE(Light-weight Web Engine) plugin to support WebView for Dali

####################################
# Rive Animation View Plugin
####################################
%package rive-animation-view
Summary:    Plugin to render a rive animation
Group:      System/Libraries
Requires: dali2-extension-rive-animation-view = %{version}-%{release}
%if 0%{?tizen_65_or_greater}
BuildRequires:  pkgconfig(thorvg)
BuildRequires:  pkgconfig(rive_tizen)
%endif

%description rive-animation-view
Plugin to render a rive animation

%package rive-animation-view-devel
Summary: Development components for rive animation view
Group: System/Libraries
Requires: dali2-extension-rive-animation-view = %{version}-%{release}

%description rive-animation-view-devel
Header & package configuration of rive-animation-view

##############################
# Preparation
##############################
%prep
%setup -q

#Use TZ_PATH when tizen version is 3.x or greater

%define dali_data_rw_dir         %TZ_SYS_RO_SHARE/dali/
%define dali_data_ro_dir         %TZ_SYS_RO_SHARE/dali/
%define dev_include_path %{_includedir}

##############################
# Build
##############################
%build
PREFIX+="/usr"
CXXFLAGS+=" -Wall -g -Os -fPIC -fvisibility-inlines-hidden -fdata-sections -ffunction-sections -DGL_GLEXT_PROTOTYPES"
LDFLAGS+=" -Wl,--rpath=%{_libdir} -Wl,--as-needed -Wl,--gc-sections -Wl,-Bsymbolic-functions "

%if 0%{?tizen_version_major} >= 5
CFLAGS+=" -DECORE_WL2 -DEFL_BETA_API_SUPPORT"
CXXFLAGS+=" -DECORE_WL2 -DEFL_BETA_API_SUPPORT"
configure_flags="--enable-ecore-wl2"
%endif

%if "%{?profile}" == "tv"
CFLAGS+=" -DOS_TIZEN_TV"
CXXFLAGS+=" -DOS_TIZEN_TV"
%endif

libtoolize --force
cd %{_builddir}/%{name}-%{version}/build/tizen
autoreconf --install

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
%if 0%{?enable_web_engine_plugin} == 1
           --enable-web-engine-plugin \
%endif
%if 0%{?enable_image_loader}
           --enable-imageloader-extension \
%endif
%if 0%{?enable_color_controller}
           --enable-color-controller \
%endif
%if 0%{?enable_evas_plugin}
           --enable-evas-plugin \
%endif
           --enable-ecore-wl2 \
           --enable-keyextension

make %{?jobs:-j%jobs}

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

cd build/tizen
%make_install DALI_DATA_RW_DIR="%{dali_data_rw_dir}" DALI_DATA_RO_DIR="%{dali_data_ro_dir}"

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

%post camera-player-plugin
/sbin/ldconfig
exit 0

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%post web-engine-chromium-plugin
pushd %{_libdir}
ln -sf libdali2-web-engine-chromium-plugin.so libdali2-web-engine-plugin.so
popd
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

%if 0%{?tizen_65_or_greater}
%post rive-animation-view
/sbin/ldconfig
exit 0
%endif

%if 0%{?enable_color_controller}
%post color-controller-plugin
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

%postun camera-player-plugin
/sbin/ldconfig
exit 0

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

%if 0%{?tizen_65_or_greater}
%postun rive-animation-view
/sbin/ldconfig
exit 0
%endif

%if 0%{?enable_color_controller}
%postun color-controller-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%postun web-engine-lwe-plugin
/sbin/ldconfig
exit 0
%endif

##############################
# Files in Binary Packages
##############################

%files
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_sysconfdir}/profile.d/dali.sh
%{_libdir}/libdali2-extension.so*
%license LICENSE

%files devel
%defattr(-,root,root,-)
%{_includedir}/dali-extension/*
%{_libdir}/pkgconfig/dali2-extension.pc

%files key-extension
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-key-extension.so*
%license LICENSE

%files video-player-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-video-player-plugin.so*
%license LICENSE

%if 0%{?tizen_65_or_greater}
%files camera-player-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-camera-player-plugin.so*
%license LICENSE
%endif

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%files web-engine-chromium-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-web-engine-chromium-plugin.so*
%license LICENSE
%endif

%if 0%{?enable_image_loader}
%files image-loader-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-image-loader-plugin.so*
%license LICENSE
%endif

%if 0%{?tizen_55_or_greater}
%files vector-animation-renderer-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-vector-animation-renderer-plugin.so*
%license LICENSE
%endif

%if 0%{?tizen_65_or_greater}
%files rive-animation-view
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-extension-rive-animation-view.so*
%license LICENSE

%files rive-animation-view-devel
%defattr(-,root,root,-)
%{_includedir}/dali-extension/devel-api/rive-animation-view/*
%{_libdir}/pkgconfig/dali2-extension-rive-animation-view.pc
%endif

%if 0%{?enable_color_controller}
%files color-controller-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-color-controller-plugin.so*
%license LICENSE
%endif

%if 0%{?tizen_55_or_greater} && 0%{?enable_web_engine_plugin} == 1
%files web-engine-lwe-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-web-engine-lwe-plugin.so*
%license LICENSE
%endif
