Name: @PACKAGE@
Summary: Credential Validation Modules
Version: @VERSION@
Release: 1
Copyright: GPL
Group: Utilities/System
Source: http://em.ca/~bruceg/@PACKAGE@/@VERSION@/@PACKAGE@-@VERSION@.tar.gz
BuildRoot: %{_tmppath}/@PACKAGE@-buildroot
URL: http://em.ca/~bruceg/@PACKAGE@/
Packager: Bruce Guenter <bruceg@em.ca>

%description
This package implements the CVM interface as a client (testcvmclient)
and as a module (cvm_unix).

%prep
%setup

%build
echo gcc "%{optflags}" >conf-cc
echo gcc -s >conf-ld
echo %{_bindir} >conf-bin
make

%install
rm -fr %{buildroot}
rm -f *.o conf_bin.c
echo %{buildroot}%{_bindir} >conf-bin
make installer instcheck

mkdir -p %{buildroot}%{_bindir}
./installer
./instcheck

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc COPYING NEWS README *.html
%{_bindir}/*
