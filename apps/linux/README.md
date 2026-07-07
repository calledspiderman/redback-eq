# Radioform for Linux

System-wide 10-band parametric equalizer for Linux, using **PipeWire** for audio processing and **GTK4** for the system tray UI.

## Architecture

```
┌──────────────────┐     ┌─────────────────┐     ┌──────────────────┐
│  radioform-ui    │────▶│  /tmp/           │◀────│ radioform-filter │
│  (GTK4 tray app) │     │  radioform-      │     │ (PipeWire DSP)   │
│                  │     │  preset.json     │     │                  │
└──────────────────┘     └─────────────────┘     └──────────────────┘
       │                                              │
       │  Unix socket                                 │ PipeWire node
       │  /tmp/radioform-control.sock                  │ (virtual sink)
       ▼                                              ▼
 ┌──────────────────┐                      ┌──────────────────┐
 │ Presets on disk  │                      │ System Audio     │
 │ ~/.local/share/  │                      │ (via PipeWire)   │
 │ radioform/presets│                      │                  │
 └──────────────────┘                      └──────────────────┘
```

- **radioform-filter**: A PipeWire filter node that applies DSP to system audio. Creates a virtual sink you select as your default output.
- **radioform-ui**: GTK4 system tray app with the 10-band EQ interface, preset management, and IPC to the filter.

## Quick Start

```bash
# Start the PipeWire filter
radioform-filter &

# Start the UI (system tray)
radioform-ui &
```

Then select "Radioform EQ" as your default audio sink in system settings.

## Building from Source

### Dependencies

**Debian/Ubuntu:**
```bash
sudo apt install build-essential cmake pkg-config \
    libpipewire-0.3-dev libspa-0.2-dev \
    libgtk-4-dev libayatana-appindicator3-dev \
    libjson-c-dev
```

**Arch Linux:**
```bash
sudo pacman -S base-devel cmake pkg-config \
    pipewire libpipewire \
    gtk4 libappindicator-gtk3 json-c
```

**Fedora:**
```bash
sudo dnf install gcc-c++ cmake pkgconfig \
    pipewire-devel spaudio-devel \
    gtk4-devel libappindicator-gtk3-devel \
    json-c-devel
```

### Build

```bash
# From the repo root:
make linux-build

# Or directly:
./tools/packaging/linux/build.sh
```

### Package

```bash
make linux-deb       # .deb package
make linux-rpm       # .rpm package
make linux-appimage  # AppImage
```

## Presets

Presets are JSON files stored in:
- System: `/usr/share/radioform/presets/`
- User: `~/.local/share/radioform/presets/`

The format matches the macOS version for compatibility.

## Files

| Component | Description |
|-----------|-------------|
| `packages/dsp/` | Portable C++ DSP engine (10-band EQ, limiter, DC blocker) |
| `packages/linux/src/filter/` | PipeWire filter module |
| `apps/linux/` | GTK4 system tray UI app |
| `tools/packaging/linux/` | Debian, RPM, AppImage packaging |

## IPC

The UI communicates with the filter via:
1. **Unix socket** (`/tmp/radioform-control.sock`) — real-time parameter updates
2. **Preset file** (`/tmp/radioform-preset.json`) — file-based preset changes (monitored via inotify)
