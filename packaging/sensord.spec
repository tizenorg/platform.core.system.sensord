Name:       sensord
Summary:    Sensor daemon
Version:    1.0.0
Release:    0
Group:     	System/Sensor Framework
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    sensord.service
Source2:    sensord.socket

%define accel_state ON
%define gyro_state ON
%define proxi_state OFF
%define light_state OFF
%define geo_state ON
%define gravity_state OFF
%define linear_accel_state OFF
%define motion_state OFF

%define build_test_suite OFF

BuildRequires:  cmake
BuildRequires:  vconf-keys-devel
BuildRequires:  libattr-devel
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(libsystemd-daemon)
BuildRequires:  pkgconfig(capi-system-info)

%description
Sensor daemon

%package sensord
Summary:    Sensor daemon
Group:      System/Sensor Framework
Requires:   %{name} = %{version}-%{release}

%description sensord
Sensor daemon

%package -n libsensord
Summary:    Sensord library
Group:      System/Sensor Framework
Requires:   %{name} = %{version}-%{release}

%description -n libsensord
Sensord library

%package -n libsensord-devel
Summary:    Sensord library (devel)
Group:      System/Sensor Framework
Requires:   %{name} = %{version}-%{release}

%description -n libsensord-devel
Sensord library (devel)

%if %{build_test_suite} == "ON"
%package -n sensor-tc
Summary:    Sensord library
Group:      System/Sensor Framework
Requires:   %{name} = %{version}-%{release}

%description -n sensor-tc
Sensor functional testing

%endif

%prep
%setup -q

%build
#CFLAGS+=" -fvisibility=hidden "; export CFLAGS
#CXXFLAGS+=" -fvisibility=hidden -fvisibility-inlines-hidden ";export CXXFLAGS
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DACCEL=%{accel_state} \
	-DGYRO=%{gyro_state} -DPROXI=%{proxi_state} -DLIGHT=%{light_state} \
	-DGEO=%{geo_state} -DGRAVITY=%{gravity_state} \
	-DLINEAR_ACCEL=%{linear_accel_state} -DMOTION=%{motion_state} \
	-DTEST_SUITE=%{build_test_suite}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}/usr/share/license
mkdir -p %{buildroot}%{_unitdir}/sockets.target.wants
mkdir -p %{buildroot}%{_unitdir}/multi-user.target.wants
install -m 0644 %SOURCE1 %{buildroot}%{_unitdir}
install -m 0644 %SOURCE2 %{buildroot}%{_unitdir}
ln -s ../sensord.socket  %{buildroot}%{_unitdir}/sockets.target.wants/sensord.socket
ln -s ../sensord.service  %{buildroot}%{_unitdir}/multi-user.target.wants/sensord.service

%post -p /sbin/ldconfig
systemctl daemon-reload

%postun -p /sbin/ldconfig
systemctl daemon-reload

%files -n sensord
%manifest sensord.manifest
%{_bindir}/sensord
%attr(0644,root,root)/usr/etc/sensor_plugins.xml
%attr(0644,root,root)/usr/etc/sensors.xml
%{_unitdir}/sensord.service
%{_unitdir}/sensord.socket
%{_unitdir}/multi-user.target.wants/sensord.service
%{_unitdir}/sockets.target.wants/sensord.socket
%license LICENSE.APLv2
%{_datadir}/license/sensord

%files -n libsensord
%manifest libsensord.manifest
%defattr(-,root,root,-)
%{_libdir}/libsensor.so.*
%{_libdir}/sensord/*.so*
%{_libdir}/libsensord-share.so
%{_libdir}/libsensord-server.so
%license LICENSE.APLv2
%{_datadir}/license/libsensord

%files -n libsensord-devel
%defattr(-,root,root,-)
%{_includedir}/sensor/*.h
%{_includedir}/sf_common/*.h
%{_libdir}/libsensor.so
%{_libdir}/pkgconfig/sensor.pc
%{_libdir}/pkgconfig/sf_common.pc
%{_libdir}/pkgconfig/sensord-server.pc

%if %{build_test_suite} == "ON"
%files -n sensor-tc
%defattr(-,root,root,-)
/usr/bin/accelerometer
/usr/bin/geomagnetic
/usr/bin/orientation
/usr/bin/gravity
%license LICENSE.APLv2
%{_datadir}/license/test
%endif

