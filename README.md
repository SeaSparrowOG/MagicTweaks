# Magic Fixes & Tweaks
Magic Fixes and Tweaks is an SKSE plugin for Skyrim Special Edition that fixes a few magic-related bugs and makes some much needed changes to improve the overall quality of life.

## Requirements
* [CMake](https://cmake.org/)
	* Add this to your `PATH`
* [Vcpkg](https://github.com/microsoft/vcpkg)
	* Add the environment variable `VCPKG_ROOT` with the value as the path to the folder containing vcpkg
* [Visual Studio Community 2022](https://visualstudio.microsoft.com/)
	* Desktop development with C++
	* 
## Register Visual Studio as a Generator
* Open `x64 Native Tools Command Prompt`
* Run `cmake`
* Close the cmd window

## Building
```
git clone https://github.com/SeaSparrowOG/MagicTweaks
cd MagicTweaks
cmake --preset vs2022-windows
cmake --build build --config Release
```

## Automation
By defining `SKYRIM_MODS_FOLDER` as an environment variable in Windows, CMake will attempt to copy over necessary files into a new folder named after the project in that directory. It is recommended that you point this to your MO2 mods folder for maximum efficiency.

## Project Configuration Files
Magic Fixes & Tweaks supports configuration files. These are json files stored within `Skyrim Special Edition/Data/SKSE/Plugins/MagicTweaks`. Configuration files should be formatted like this:
```json
{
  "DispelOnSeathe" : [
    "Modname.extension|0xFormID",
	"Modname.extension|0xOtherFormID"
  ]
}
```
For this setting to be of any use, bDispelOnSheathe must be enabled in the MagicTweaks.ini or the custom ini. The idea is that when the player lowers their hands, the specified effects will be dispeled.
For example, `Skyrim.esm|0x3AE9E` is the FlameCloak effect. If that is specified, lowering your hands dispels that effect. If [PowerOfThree's Tweaks](https://www.nexusmods.com/skyrimspecialedition/mods/51073) are installed, you can also use EditorIDs - so "FireCloakFFSelf" is considered valid. If you want to dispel all valid effects from a spell, you can specify that spell itself. `ccbgssse002-exoticarrows.esl|0x80C`, for example, is the Bound Quiver spell from Arcane Archer.

**IMPORTANT** - Not all effects will be dispeled. Instead, only effects that have a duration greater than 0 will be dispeled.