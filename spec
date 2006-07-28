Name: cvm
Summary: Credential Validation Modules
Version: @VERSION@
Release: 1
License: GPL
Group: Utilities/System
Source: http://untroubled.org/cvm/cvm-@VERSION@.tar.gz
BuildRoot: %{_tmppath}/cvm-buildroot
URL: http://untroubled.org/cvm/
Packager: Bruce Guenter <bruce@untroubled.org>
BuildRequires: bglibs >= 1.100
BuildRequires: mysql-devel
BuildRequires: postgresql-devel
Obsoletes: cvm-vmailmgr

%description
This package implements the CVM interface as a client (cvm-testclient),
and as a module (cvm-unix, cvm-pwfile).

%package devel
Summary: Development libraries for CVM
Group: Development/Libraries

%description devel
This package includes all the development libraries and headers for
building CVM clients or modules.

%package mysql
Group: Utilities/System
Summary: MySQL Credential Validation Modules

%description mysql
Credential Validation Modules that authenticate against a MySQL server.

%package pgsql
Group: Utilities/System
Summary: PostgreSQL Credential Validation Modules

%description pgsql
Credential Validation Modules that authenticate against a PostgreSQL server.

%prep
%setup

%build
echo gcc "%{optflags}" -I%{_includedir}/pgsql >conf-cc
echo gcc -s -L%{_libdir}/mysql >conf-ld
echo %{_bindir} >conf-bin
echo %{_includedir} >conf-include
echo %{_libdir} >conf-lib
make libraries programs mysql pgsql

%install
rm -fr %{buildroot}
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_includedir}
mkdir -p %{buildroot}%{_libdir}

make install_prefix=%{buildroot} install

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc COPYING NEWS README *.html
%{_bindir}/cvm-[^mp]*
%{_bindir}/cvm-pwfile

%files devel
%defattr(-,root,root)
%{_includedir}/*
%{_libdir}/*

%files mysql
%defattr(-,root,root)
%{_bindir}/cvm-mysql*

%files pgsql
%defattr(-,root,root)
%{_bindir}/cvm-pgsql*
