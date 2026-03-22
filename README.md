# FCEUX-Vita

A standalone NES/Famicom emulator for PlayStation Vita, powered by the [FCEUX](https://github.com/TASEmulators/fceux) emulation core.

![FCEUX-Vita](data/vita/romfs/sce_sys/pic0.png)

## Features

- Full-speed NES emulation on PS Vita
- High mapper compatibility (FCEUX supports 300+ mappers)
- RGUI in-game menu (Select + Start)
- Save states (multiple slots)
- ROM browser with ZIP archive support
- Configurable controls with per-game bindings
- Turbo A / Turbo B buttons (FCEUX AutoFire)
- Game Genie cheat support
- FDS (Famicom Disk System) support
- NTSC / PAL / Dendy region support

## Installation

1. Download the latest `fceux-vita.vpk` from Releases
2. Transfer the VPK to your Vita
3. Install using VitaShell
4. Place NES ROMs (`.nes`, `.zip`) in `ux0:/data/fceux-vita/roms/`

For FDS games, place `disksys.rom` in `ux0:/data/fceux-vita/`.

## Controls

| Vita Button | NES Button |
|-------------|------------|
| Cross | B |
| Circle | A |
| Square | Turbo B |
| Triangle | Turbo A |
| D-Pad | D-Pad |
| Start | Start |
| Select | Select |
| Select + Start | Open RGUI Menu |

## Supported Formats

- `.nes`, `.nez` - iNES / NES 2.0 ROMs
- `.unf`, `.unif` - UNIF ROMs
- `.fds` - Famicom Disk System images
- `.nsf` - NES Sound Format
- `.zip` - Archived ROMs (first supported file is extracted)

## Building from Source

### Prerequisites

- [VitaSDK](https://vitasdk.org/)
- CMake 3.10+
- libarchive (with zlib, bz2, zstd)

### Build

```bash
git clone --recurse-submodules https://github.com/user/fceux-vita.git
cd fceux-vita
mkdir build && cd build
cmake -DPLATFORM_VITA=ON -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
make fceux-vita.vpk
```

The output VPK will be at `build/fceux-vita.vpk`.

## Credits

- [FCEUX](https://github.com/TASEmulators/fceux) - NES emulation core
- [libcross2d](https://github.com/Cpasjuste/libcross2d) - Cross-platform 2D library
- [pemu](https://github.com/Cpasjuste/pemu) - Portable emulator framework

## License

GPL-2.0 (same as FCEUX)
