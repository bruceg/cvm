Name: cvm
Summary: Credential Validation Modules
Version: @VERSION@
Release: 1
Copyright: GPL
Group: Utilities/System
Source: http://untroubled.org/cvm/cvm-@VERSION@.tar.gz
BuildRoot: %{_tmppath}/cvm-buildroot
URL: http://untroubled.org/cvm/
Packager: Bruce Guenter <bruceg@em.ca>
BuildRequires: bglibs >= 1.009

%description
This package implements the CVM interface as a client (cvm-testclient),
and as a module (cvm-unix, cvm-pwfile).

%package devel
Summary: Development libraries for CVM
Group: Development/Libraries

%description devel
This package includes all the development libraries and headers for
building CVM clients or modules.

%prep
%setup

%build
echo gcc "%{optflags}" >conf-cc
echo gcc -s >conf-ld
echo %{_bindir} >conf-bin
make programs

%install
rm -fr %{buildroot}
rm -f *.o conf_home.c
echo %{buildroot}%{_prefix} >conf-home
make installer instcheck

mkdir -p %{buildroot}%{_prefix}
./installer
./instcheck

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc COPYING NEWS README *.html
%{_prefix}/bin/*

%files devel
%defattr(-,root,root)
%{_prefix}/include/*
%{_prefix}/lib/*
