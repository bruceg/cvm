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
echo gcc "%{optflags}" -I/usr/include/pgsql >conf-cc
echo gcc -s >conf-ld
echo %{_bindir} >conf-bin
make libraries programs mysql pgsql

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
%{_prefix}/bin/cvm-[^mp]*
%{_prefix}/bin/cvm-pwfile

%files devel
%defattr(-,root,root)
%{_prefix}/include/*
%{_prefix}/lib/*

%files mysql
%defattr(-,root,root)
%{_bindir}/cvm-mysql*

%files pgsql
%defattr(-,root,root)
%{_bindir}/cvm-pgsql*
