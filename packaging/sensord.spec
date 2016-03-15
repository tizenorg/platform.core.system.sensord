Name:       sensord
Summary:    Sensor daemon
Version:    2.0.3
Release:    0
Group:      System/Sensor Framework
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    sensord.service
Source2:    sensord_command.socket
Source3:    sensord_event.socket


BuildRequires:  cmake
BuildRequires:  libattr-devel
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(libsystemd-daemon)
BuildRequires:  pkgconfig(capi-system-info)
BuildRequires:  pkgconfig(cynara-creds-socket)
BuildRequires:  pkgconfig(cynara-client)
BuildRequires:  pkgconfig(cynara-session)
Requires:   libsensord = %{version}-%{release}

%define auto_rotation_state ON
%define orientation_state OFF
%define gravity_state OFF
%define linear_accel_state OFF
%define rv_state OFF
%define geomagnetic_rv_state OFF
%define gaming_rv_state OFF
%define tilt_state OFF
%define gyroscope_uncal_state OFF
%define build_test_suite ON

%description
Sensor daemon

%package -n libsensord
Summary:    Sensord library
Group:      System/Libraries
Requires:   %{name} = %{version}-%{release}

%description -n libsensord
Sensord library

%package -n libsensord-devel
Summary:    Sensord shared library
Group:      System/Development
Requires:   libsensord = %{version}-%{release}

%description -n libsensord-devel
Sensord shared library

%package -n sensor-hal-devel
Summary:    Sensord HAL interface
Group:      System/Development

%description -n sensor-hal-devel
Sensord HAL interface

%if %{build_test_suite} == "ON"
%package -n sensor-test
Summary:    Sensord library
Group:      System/Testing

%description -n sensor-test
Sensor functional testing

%endif

%prep
%setup -q

MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`

cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DMAJORVER=${MAJORVER} -DFULLVER=%{version} \
	-DORIENTATION=%{orientation_state} -DGRAVITY=%{gravity_state} \
	-DLINEAR_ACCEL=%{linear_accel_state} -DRV=%{rv_state} \
	-DGEOMAGNETIC_RV=%{geomagnetic_rv_state} -DGAMING_RV=%{gaming_rv_state} \
	-DGYROSCOPE_UNCAL=%{gyroscope_uncal_state} -DAUTO_ROTATION=%{auto_rotation_state} \
	-DTILT=%{tilt_state} -DTEST_SUITE=%{build_test_suite}

%build
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}%{_unitdir}

install -m 0644 %SOURCE1 %{buildroot}%{_unitdir}
install -m 0644 %SOURCE2 %{buildroot}%{_unitdir}
install -m 0644 %SOURCE3 %{buildroot}%{_unitdir}

%install_service multi-user.target.wants sensord.service
%install_service sockets.target.wants sensord_event.socket
%install_service sockets.target.wants sensord_command.socket

%post
systemctl daemon-reload

%postun
systemctl daemon-reload

%post -n libsensord
ln -sf %{_libdir}/libsensor.so.%{version} %{_libdir}/libsensor.so.1
/sbin/ldconfig

%postun -n libsensord
/sbin/ldconfig

%files
%attr(0644,root,root)/usr/etc/virtual_sensors.xml
%manifest packaging/sensord.manifest
%{_bindir}/sensord
%{_unitdir}/sensord.service
%{_unitdir}/sensord_command.socket
%{_unitdir}/sensord_event.socket
%{_unitdir}/multi-user.target.wants/sensord.service
%{_unitdir}/sockets.target.wants/sensord_command.socket
%{_unitdir}/sockets.target.wants/sensord_event.socket
%license LICENSE.APLv2

%files -n libsensord
%defattr(-,root,root,-)
%manifest packaging/libsensord.manifest
%{_libdir}/libsensor.so.*
%{_libdir}/libsensord-shared.so
%license LICENSE.APLv2

%files -n libsensord-devel
%defattr(-,root,root,-)
%{_includedir}/sensor/*.h
%{_libdir}/libsensor.so
%{_libdir}/pkgconfig/sensor.pc
%license LICENSE.APLv2

%files -n sensor-hal-devel
%defattr(-,root,root,-)
%{_includedir}/sensor/sensor_hal.h
%license LICENSE.APLv2

%if %{build_test_suite} == "ON"
%files -n sensor-test
%defattr(-,root,root,-)
%{_bindir}/api-test
%{_bindir}/sensor-test
%{_bindir}/multi-thread-performance-test
%{_bindir}/multi-process-performance-test
%{_bindir}/fusion-data-collection
%license LICENSE.APLv2
%endif
