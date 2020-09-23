# NOTES
# This spec file is used to build DALi Extensions
#
# gbs will try to download the build.conf for the platform automatically from the repo location when
# performing a gbs build ( use gbs build -v to see it download location) E.g.
# http://download.tizen.org/snapshots/tizen/tv/tizen-tv/repos/arm-wayland/packages/repodata/xxxx-build.conf.gz

Name:       dali2-extension
Summary:    The DALi Tizen Extensions
Version:    1.9.30
Release:    1
Group:      System/Libraries
License:    Apache-2.0 and BSD-3-Clause and MIT
URL:        https://review.tizen.org/git/?p=platform/core/uifw/dali-extensions.git;a=summary
Source0:    %{name}-%{version}.tar.gz

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(dali2-core)
BuildRequires:  pkgconfig(dali2-adaptor)
BuildRequires:  pkgconfig(dali2-toolkit)
BuildRequires:  pkgconfig(dlog)

# For evas-plugin
BuildRequires:  pkgconfig(dali2-adaptor-integration)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(ecore-wl2)

%description
dali-extension

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


# # Note
# %if 0%{?tizen_version_major} >= 6
# %define tizen_60_or_greater 1
# %endif

##############################
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
# dali-adaptor uses ecore mainloop
%if 0%{?tizen_version_major} >= 5
BuildRequires:  pkgconfig(ecore-wl2)
%else
BuildRequires:  pkgconfig(ecore-wayland)
%endif

%description video-player-plugin
VideoPlayer plugin to play a video file for Dali

##############################
# Dali Web Engine chromium Plugin
##############################

%package web-engine-chromium-plugin
Summary:    Plugin to support WebView for Dali
Group:      System/Libraries
%if 0%{?tizen_55_or_greater}
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

# tizen_65 is temporary
%if 0%{?tizen_65_or_greater}
####################################
# Vector Image Renderer Plugin
####################################
%package vector-image-renderer-plugin
Summary:    Plugin to render a vector image
Group:      System/Libraries
BuildRequires:  pkgconfig(thorvg)
#define enable_vector_image_renderer_build 1

%description vector-image-renderer-plugin
Plugin to render a vector image

%endif

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
%if 0%{?tizen_55_or_greater}
BuildRequires: pkgconfig(libtbm)
BuildRequires: pkgconfig(lightweight-web-engine)
%endif

%description web-engine-lwe-plugin
Web Engine LWE(Light-weight Web Engine) plugin to support WebView for Dali

##############################
# Preparation
##############################
%prep
%setup -q

#Use TZ_PATH when tizen version is 3.x or greater

%define dali_data_rw_dir         %TZ_SYS_RO_SHARE/dali/
%define dali_data_ro_dir         %TZ_SYS_RO_SHARE/dali/
%define dev_include_path %{_includedir}

# Use Image Loader Plugin
%define use_image_loader 0

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

libtoolize --force
cd %{_builddir}/%{name}-%{version}/build/tizen
autoreconf --install

%configure --prefix=$PREFIX \
%if 0%{?tizen_50_or_greater}
           --with-tizen-50-or-greater \
%endif
%if 0%{?tizen_55_or_greater}
           --with-tizen-55-or-greater \
%endif
           --enable-ecore-wl2 \
           --enable-keyextension
%if 0%{?use_image_loader}
%configure \
           --enable-imageloader-extension
%endif

make %{?jobs:-j%jobs}

##############################
# Installation
##############################
%install
rm -rf %{buildroot}

# install dali.sh
mkdir -p %{buildroot}%{_sysconfdir}/profile.d
install -m 0644 scripts/dali.sh %{buildroot}%{_sysconfdir}/profile.d

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

%if 0%{?tizen_55_or_greater}
%post web-engine-chromium-plugin
pushd %{_libdir}
ln -sf libdali2-web-engine-chromium-plugin.so libdali2-web-engine-plugin.so
popd
/sbin/ldconfig
exit 0
%endif

%post image-loader-plugin
/sbin/ldconfig
exit 0

%if 0%{?tizen_55_or_greater}
%post vector-animation-renderer-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_65_or_greater}
%post vector-image-renderer-plugin
/sbin/ldconfig
exit 0
%endif

%post color-controller-plugin
/sbin/ldconfig
exit 0

%if 0%{?tizen_55_or_greater}
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

%if 0%{?tizen_55_or_greater}
%postun web-engine-chromium-plugin
/sbin/ldconfig
exit 0
%endif

%postun image-loader-plugin
/sbin/ldconfig
exit 0

%if 0%{?tizen_55_or_greater}
%postun vector-animation-renderer-plugin
/sbin/ldconfig
exit 0
%endif

%if 0%{?tizen_65_or_greater}
%postun vector-image-renderer-plugin
/sbin/ldconfig
exit 0
%endif

%postun color-controller-plugin
/sbin/ldconfig
exit 0

%if 0%{?tizen_55_or_greater}
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
%{_libdir}/pkgconfig/*.pc

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

%if 0%{?tizen_55_or_greater}
%files web-engine-chromium-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-web-engine-chromium-plugin.so*
%license LICENSE
%endif

%if 0%{?use_image_loader}
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
%files vector-image-renderer-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-vector-image-renderer-plugin.so*
%license LICENSE
%endif

%files color-controller-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-color-controller-plugin.so*
%license LICENSE

%if 0%{?tizen_55_or_greater}
%files web-engine-lwe-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-web-engine-lwe-plugin.so*
%license LICENSE
%endif
