# Copyright (c) 2006 oc2pus
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bugfixes or comments to oc2pus@arcor.de

%define _cvs_build 0
%define _cvs       cvs20060330

Summary:	Danger from the Deep - WW2 german submarine simulation
Name:		dangerdeep
Version:	0.2.0
%if "%{_cvs_build}" == "1"
Release:        1.oc2pus.%{_cvs}
%else
Release:        0.oc2pus.1
%endif
License:	GPL
Group:		Amusements/Games/Strategy/Turn Based
URL:		http://dangerdeep.sourceforge.net
Source0:	%{name}-%{version}.tar.bz2
NoSource:       0
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-buildroot
Packager:       oc2pus <oc2pus@arcor.de>
BuildRequires:	scons
BuildRequires:	ImageMagick
BuildRequires:	fftw3-devel
BuildRequires:  perl
BuildRequires:	SDL-devel
BuildRequires:	SDL_net
BuildRequires:	SDL_image-devel
BuildRequires:	SDL_mixer-devel
BuildRequires:  xorg-x11-Mesa-devel
Requires:	fftw3
Requires:	SDL
Requires:	SDL_net
Requires:	SDL_image
Requires:	SDL_mixer
Requires:	xorg-x11-Mesa

%description
Danger from the deep (aka dangerdeep) is a Free / Open Source World War II
german submarine simulation. It is currently available for Linux/i386 and
Windows, but since it uses SDL/OpenGL it should be portable to other operating
systems or platforms. (If anyone whishes to port it, please contact us.) This
game is planned as tactical simulation and will be as realistic as our time and
knowledge of physics allows. It's current state is ALPHA, but it is playable.

Authors: see CREDITS

%prep
%if "%{_cvs_build}" == "1"
%setup -q -n %{name}
%else
%setup -q -n %{name}-%{version}
%endif

perl -pi -e "s|/usr/local/bin|%{_bindir}|"                          SConstruct
perl -pi -e "s|/usr/local/share/dangerdeep|%{_datadir}/dangerdeep|" SConstruct

%build
scons -k

%install
[ -d %{buildroot} -a "%{buildroot}" != "" ] && rm -rf %{buildroot}

scons \
	installbindir=%{buildroot}%{_bindir} \
	installdatadir=%{buildroot}%{_datadir}/%{name} \
	datadir=%{_datadir}/%{name} \
	install

%__strip %{buildroot}%{_bindir}/%{name}

# man
install -dm 755 %{buildroot}%{_mandir}/man6
install -m 655 doc/man/%{name}.6 %{buildroot}%{_mandir}/man6

# icon
install -dm 755 %{buildroot}%{_datadir}/pixmaps
convert logo.xpm -resize 48x48! %{buildroot}%{_datadir}/pixmaps/%{name}.png
 
# Menu
cat > %{name}.desktop << EOF
[Desktop Entry]
Name=%{name}
Comment=Danger from the Deep - WW2 german submarine simulation
Exec=%{name}
Icon=%{name}.png
Terminal=0
Type=Application
EOF
%suse_update_desktop_file -i %{name} Game StrategyGame

# clean up .sconsign-files
(cd %{buildroot} && find . -name .sconsign | xargs rm -f)

%clean
[ -d %{buildroot} -a "%{buildroot}" != "" ] && rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc ARTWORK* ChangeLog CREDITS README INSTALL LICENSE*
%{_bindir}/*
%dir %{_datadir}/%{name}
%{_datadir}/%{name}/*
%{_mandir}/man6/*
%{_datadir}/pixmaps/%{name}.png
%{_datadir}/applications/%{name}.desktop

%changelog
%if "%{_cvs_build}" == "1"
#* Thu Mar 30 2006 - oc2pus@arcor.de 0.1.0-1.oc2pus.cvs20060330
#- update to cvs20060330
#- using scons to install all files
%else
* Sat Apr 15 2006 - oc2pus@arcor.de 0.1.1-0.oc2pus.1
- update to 0.1.1
* Fri Mar 17 2006 - oc2pus@arcor.de 0.1.0-0.oc2pus.1
- initial rpm 0.1.0
%endif
