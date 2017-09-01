# NOTES
# This spec file is used to build DALi Extensions
#
# gbs will try to download the build.conf for the platform automatically from the repo location when
# performing a gbs build ( use gbs build -v to see it download location) E.g.
# http://download.tizen.org/snapshots/tizen/tv/tizen-tv/repos/arm-wayland/packages/repodata/xxxx-build.conf.gz

Name:       dali-extension
Summary:    The DALi Tizen Extensions
Version:    1.2.54
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

V##############################
# Dali Key Extension
##############################

%package key-extension
Summary:    Plugin to support extension keys for Dali
Group:      System/Libraries

%description key-extension
Plugin to support extension keys for Dali

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

libtoolize --force
cd %{_builddir}/%{name}-%{version}/build/tizen
autoreconf --install

%configure --prefix=$PREFIX \
           --enable-keyextension

make %{?jobs:-j%jobs}

##############################
# Installation
##############################
%install
rm -rf %{buildroot}
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

##############################
# Files in Binary Packages
##############################

%files
%manifest dali-extension.manifest
%defattr(-,root,root,-)
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
