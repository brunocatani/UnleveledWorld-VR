# Changelog

## 1.0.0

First release of the Fallout 4 VR port of Unleveled World by GELUXRUM. It replaces the
original mod's `GLXRM_UnleveledWorld.dll`; the original mod and its INI stay required.

- Sets leveled item-list and character-list entry levels to 1 when game data is ready, and
  script-injected entries as they are added, on Fallout 4 VR 1.2.72.0 with F4SEVR 0.6.21.
- Reads the original mod's `Data\F4SE\Plugins\GLXRM_UnleveledWorld.ini` unchanged and never
  writes it. A missing or unreadable INI is logged and leaves unleveling off.
- Loads only on Fallout4VR.exe 1.2.72.0, checked against the executable itself.
- The script-injection hook patches its call site only after checking that it still calls the
  game's leveled-list add function; otherwise `UnlevelInjections` stays off and the log says so.
