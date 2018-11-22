# NOTES
# This spec file is used to build DALi Extensions
#
# gbs will try to download the build.conf for the platform automatically from the repo location when
# performing a gbs build ( use gbs build -v to see it download location) E.g.
# http://download.tizen.org/snapshots/tizen/tv/tizen-tv/repos/arm-wayland/packages/repodata/xxxx-build.conf.gz

Name:       dali-extension
Summary:    The DALi Tizen Extensions
Version:    1.3.50
Release:    1
Group:      System/Libraries
License:    Apache-2.0 and BSD-3-Clause and MIT
URL:        https://review.tizen.org/git/?p=platform/core/uifw/dali-extensions.git;a=summary
Source0:    %{name}-%{version}.tar.gz

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(dali-core)
BuildRequires:  pkgconfig(dali-adaptor)
BuildRequires:  pkgconfig(dali-toolkit)
BuildRequires:  pkgconfig(dlog)

%description
dali-extension

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
BuildRequires: pkgconfig(libtbm)
BuildRequires: pkgconfig(chromium-efl)
BuildRequires: pkgconfig(elementary)

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

##############################
# Dali Web Engine Lite Plugin
##############################

%package web-engine-lite-plugin
Summary:    Plugin to support WebView for Dali
Group:      System/Libraries
BuildRequires: pkgconfig(libtbm)
BuildRequires: pkgconfig(libtuv)
BuildRequires: pkgconfig(lightweight-web-engine)

%description web-engine-lite-plugin
Web Engine Lite plugin to support WebView for Dali

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

%post web-engine-chromium-plugin
/sbin/ldconfig
exit 0

%post image-loader-plugin
/sbin/ldconfig
exit 0

%post web-engine-lite-plugin
/sbin/ldconfig
exit 0

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

%postun web-engine-chromium-plugin
/sbin/ldconfig
exit 0

%postun image-loader-plugin
/sbin/ldconfig
exit 0

%postun web-engine-lite-plugin
/sbin/ldconfig
exit 0

##############################
# Files in Binary Packages
##############################

%files
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_sysconfdir}/profile.d/dali.sh
%license LICENSE

%files devel
%defattr(-,root,root,-)
%{_includedir}/%{name}/
%{_libdir}/pkgconfig/*.pc

%files key-extension
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali-key-extension.so*
%license LICENSE

%files video-player-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali-video-player-plugin.so*
%license LICENSE

%files web-engine-chromium-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali-web-engine-chromium-plugin.so*
%license LICENSE

%if 0%{?use_image_loader}
%files image-loader-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali-image-loader-plugin.so*
%license LICENSE
%endif

%files web-engine-lite-plugin
%manifest dali-extension.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali-web-engine-lite-plugin.so*
%license LICENSE
