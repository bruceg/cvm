Name: @PACKAGE@
Summary: Credential Validation Modules
Version: @VERSION@
Release: 1
Copyright: GPL
Group: Utilities/System
Source: http://em.ca/~bruceg/@PACKAGE@/@VERSION@/@PACKAGE@-@VERSION@.tar.gz
BuildRoot: /tmp/@PACKAGE@-buildroot
URL: http://em.ca/~bruceg/@PACKAGE@/
Packager: Bruce Guenter <bruceg@em.ca>

%description
This package implements the CVM interface as a client (testcvmclient)
and as a module (cvm_unix).

%prep
%setup

%build
echo gcc "$RPM_OPT_FLAGS" >conf-cc
echo gcc -s >conf-ld
echo %{_bindir} >conf-bin
make

%install
rm -fr $RPM_BUILD_ROOT
rm -f *.o conf_bin.c
echo $RPM_BUILD_ROOT%{_bindir} >conf-bin
make installer instcheck

mkdir -p $RPM_BUILD_ROOT%{_bindir}
./installer
./instcheck

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc COPYING NEWS README *.html
%{_bindir}/*
