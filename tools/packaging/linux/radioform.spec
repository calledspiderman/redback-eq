%global _prefix /usr

Name:           radioform
Version:        1.0.0
Release:        1%{?dist}
Summary:        System-wide 10-band parametric equalizer

License:        GPLv3
URL:            https://github.com/Torteous44/radioform
Source0:        radioform-%{version}.tar.gz

BuildRequires:  cmake >= 3.20
BuildRequires:  gcc-c++
BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(libpipewire-0.3)
BuildRequires:  pkgconfig(gtk4)
BuildRequires:  pkgconfig(ayatana-appindicator3-0.1)
BuildRequires:  pkgconfig(json-c)

Requires:       pipewire%{?_isa} >= 0.3
Requires:       gtk4%{?_isa} >= 4.0
Requires:       libappindicator-gtk3%{?_isa}
Requires:       json-c%{?_isa} >= 0.15

%description
Radioform is a free, open-source system equalizer that applies
a clean, precise 10-band parametric EQ across everything you play.

It features a PipeWire filter for real-time audio processing and
a GTK4 system tray app for intuitive EQ control.

%prep
%setup -q

%build
cmake -S packages/linux -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=%{_prefix}
cmake --build build -j %{_smp_mflags}

%install
cmake --install build --prefix %{buildroot}%{_prefix}

# Install desktop file
mkdir -p %{buildroot}%{_datadir}/applications
install -m 644 tools/packaging/linux/debian/usr/share/applications/radioform.desktop \
    %{buildroot}%{_datadir}/applications/

# Install icon
mkdir -p %{buildroot}%{_datadir}/icons/hicolor/scalable/apps
install -m 644 tools/packaging/linux/debian/usr/share/icons/hicolor/scalable/apps/radioform.svg \
    %{buildroot}%{_datadir}/icons/hicolor/scalable/apps/

# Install presets
mkdir -p %{buildroot}%{_datadir}/radioform/presets
install -m 644 tools/packaging/linux/debian/usr/share/radioform/presets/*.json \
    %{buildroot}%{_datadir}/radioform/presets/

# Install systemd user service
mkdir -p %{buildroot}%{_userunitdir}
install -m 644 tools/packaging/linux/debian/usr/lib/systemd/user/radioform-filter.service \
    %{buildroot}%{_userunitdir}/

%files
%{_bindir}/radioform-filter
%{_bindir}/radioform-ui
%{_datadir}/applications/radioform.desktop
%{_datadir}/icons/hicolor/scalable/apps/radioform.svg
%{_datadir}/radioform/presets/*.json
%{_userunitdir}/radioform-filter.service

%changelog
* Tue Jul 07 2026 Radioform Developers - 1.0.0-1
- Initial Linux release
