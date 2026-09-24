# Unleveled World VR

A Fallout 4 VR port of **Unleveled World by GELUXRUM**. It can set leveled item-list and character-list entry levels to 1 and apply the same rule to entries injected by scripts.

**The original Unleveled World is required. Install the original mod first, then install this port after it so the VR DLL overwrites the original DLL.**

## Requirements

- Fallout 4 VR **1.2.72.0**.
- **F4SEVR 0.6.21** and **VR Address Library for F4SEVR**.
- **Original Unleveled World by GELUXRUM**, available on Nexus Mods, with its listed requirements.

## Installation

1. Install the original Unleveled World and its requirements.
2. Install this port as a separate mod in Mod Organizer 2.
3. Place the port **below the original in MO2's left pane**, allowing it to overwrite `F4SE\Plugins\GLXRM_UnleveledWorld.dll`.
4. Configure the INI described below, then launch through F4SEVR.

MO2's left pane controls file-overwrite priority. Keep the original mod's other files installed.

For manual installation, install the original mod first and replace its DLL with the VR DLL at:

```text
Data\F4SE\Plugins\GLXRM_UnleveledWorld.dll
```

## Configuration

The port uses the original INI filename and location:

```text
Data\F4SE\Plugins\GLXRM_UnleveledWorld.ini
```

The required original mod supplies this INI. With MO2, edit the file supplied by the mod winning this file conflict. The port reads it unchanged and does not create or rewrite configuration files. If it is missing or unreadable, the plugin logs an error and leaves unleveling disabled.

**All three compiled defaults are `false`.** Existing INI values take precedence. To enable all three features:

```ini
[General]
UnlevelItems=true
UnlevelCharacters=true
UnlevelInjections=true
```

| Option | Effect when enabled |
| --- | --- |
| `UnlevelItems` | Sets existing leveled item-list entry levels to 1. |
| `UnlevelCharacters` | Sets existing leveled character-list entry levels to 1. |
| `UnlevelInjections` | Sets script-injected leveled-list entry levels to 1. |

Restart the game after editing. Missing keys use compiled defaults without changing the file. With all three options set to `false`, the plugin makes no list changes.

## Logs and support

The log is:

```text
Documents\My Games\Fallout4VR\F4SE\GLXRM_UnleveledWorld.log
```

It reports the active INI path and effective settings. Enabled item and character options produce `Unleveling item lists` and `Unleveling character lists`; disabled options produce `Ignoring` messages.

For [VR-port issues](https://github.com/brunocatani/UnleveledWorld-VR/issues), include this log, `f4sevr.log` from the same game session, and the relevant INI settings. Direct VR-port bug reports to this repository.

## Building from source

Install Visual Studio 2022 with Desktop development with C++, a Windows SDK, CMake 3.25 or newer, vcpkg, and CommonLibF4VR. Set `VCPKG_ROOT` to your vcpkg checkout and `COMMON_LIB_F4VR_PATH` to CommonLibF4VR's `CommonLibF4` subdirectory.

```powershell
cmake --preset fast
cmake --build --preset fast
ctest --preset fast
```

The preset builds Release with compiler concurrency capped at two processes. The DLL is written to `build-fast/Release/GLXRM_UnleveledWorld.dll`. Put machine-specific settings and optional deployment paths in an ignored `CMakeUserPresets.json`.

## Credits and permission

- **GELUXRUM** — original creator of Unleveled World. Thank you for creating the mod and granting permission for this VR port. Please support the original mod and its author.
- **brunocatani** — VR adaptation and maintenance.
- The **F4SEVR**, **CommonLibF4VR**, and **SimpleIni** contributors.

The original mod remains a mandatory separate download.

## License

This VR port is distributed under the **GNU General Public License, version 3**. See [LICENSE](LICENSE).

The original code's **Copyright (c) 2023 GELUXRUM** and MIT license notice are preserved in [LICENSE.upstream](LICENSE.upstream). Third-party source files retain their own copyright and license notices.
