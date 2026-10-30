# TDPAHook

Hook for the four games of The Dark Pictures Anthology's first season that changes the address they download the list of the anthology's games from.

Table of Contents
-----------------
- [Supported games](#supported-games)
- [Legal notes](#legal-notes)
- [Features](#features)
- [Build Requirements](#build-requirements)
- [Compiling](#compiling)
- [Installing](#installing)
- [Configuration](#configuration)
- [Usage](#usage)
- [Credits](#credits)

Supported games
---------------

| Game                                                           | Folder of the hook (in the game folder) | Executable                        |
|:---------------------------------------------------------------|:----------------------------------------|:----------------------------------|
| [Man of Medan](https://store.steampowered.com/app/939850/)     | `SMG019\Binaries\Win64`                 | `ManOfMedan-Win64-Shipping.exe`   |
| [Little Hope](https://store.steampowered.com/app/1194630/)     | `SMG024\Binaries\Win64`                 | `LittleHope-Win64-Shipping.exe`   |
| [House of Ashes](https://store.steampowered.com/app/1281590/)  | `SMG025\Binaries\Win64`                 | `HouseOfAshes-Win64-Shipping.exe` |
| [The Devil in Me](https://store.steampowered.com/app/1567020/) | `SMG028\Binaries\Win64`                 | `TheDevilInMe-Win64-Shipping.exe` |

Legal notes
-----------

- The project contains no original assets from the games.
- To use this project you need to have an original copy of the game (bought from Steam, see [Supported games](#supported-games)), the project doesn't make piracy easier and doesn't break any of the DRM included in-game.
- The project is not affiliated with or endorsed by Supermassive Games or Bandai Namco Entertainment.

Features
--------

- Changes the address the games download the list of the anthology's games from, `https://media.prod.tdp-tech.eu/DP_Anthology_Games.JSON`, to `https://tdpa.grzyb.app/DP_Anthology_Games.json` or to an address set in `TDPAHook.ini`
- One DLL for all four games: it finds the address in whichever game loads it
- Leaves the game's files unchanged: the address is rewritten in memory when the game starts, before any of its code runs
- Optional log file

Build Requirements
------------------

- Visual Studio 2026

### Libraries (installed by vcpkg):
- GoogleTest (unit tests)

Compiling
---------

See the [shared build workflows](https://github.com/GrzybDev/actions/tree/v1/.github/workflows) `cpp-checks.yml` and `cpp-dll.yml`, which CI and the release build run, for more detailed steps

1. [Integrate vcpkg with Visual Studio](https://learn.microsoft.com/vcpkg/commands/integrate)
2. Build the solution `TDPAHook.slnx` in Visual Studio (x64 Release for the hook)
3. Run `x64\Release\TDPAHook.Tests.exe` to run the unit tests; set `TDPAHOOK_GAME_EXES` to the paths of the games' executables, separated by semicolons, to also check that the hook finds and rewrites the address in your copies (the files stay unchanged)

The hook is `x64\Release\XAPOFX1_5.dll`.

Installing
----------

1. Copy `XAPOFX1_5.dll`, either from the latest build in [GitHub Releases](https://github.com/GrzybDev/TDPAHook/releases) or from `x64\Release` if you compiled it yourself, to the folder of the game's executable (see [Supported games](#supported-games)); the game loads it instead of the system's `XAPOFX1_5.dll`, to which the hook passes the game's calls
2. Optionally put `TDPAHook.ini` next to it (see [Configuration](#configuration))

To uninstall, delete `XAPOFX1_5.dll` and, if you added them, `TDPAHook.ini` and `TDPAHook.log`.

Configuration
-------------

`TDPAHook.ini` is optional and is read from the folder of `XAPOFX1_5.dll`.

| Key             | Description                                           | Allowed values                                        | Default                                          |
|:---------------:|:------------------------------------------------------|:-----------------------------------------------------:|:------------------------------------------------:|
| `Games.ListUrl` | The address the game downloads the list of games from | An `http` or `https` address of 54 characters at most | `https://tdpa.grzyb.app/DP_Anthology_Games.json` |
| `Log.File`      | Save what the hook did to `TDPAHook.log`              | 0 or 1                                                | 0                                                |

```ini
[Games]
ListUrl=https://example.com/DP_Anthology_Games.json

[Log]
File=1
```

The game keeps the address in a fixed place in its executable that only its code refers to, so the new address is written over the original and can be no longer than it (54 characters). With a longer address the game keeps its own, and the log says so. The address has to serve a list in the format of the original.

Usage
-----

1. Install the hook as described in [Installing](#installing)
2. Start the game from Steam as usual

When the game starts, before any of its code runs, the hook:
1. Reads `TDPAHook.ini`, if there is one
2. Writes the new address over the original in the game's memory
3. Leaves everything else to the game, which downloads the list from the new address while it starts; its calls to `XAPOFX1_5.dll` go on to the system's copy

Credits
-------

- [GrzybDev](https://grzyb.dev)

Special thanks to:
- Supermassive Games (for making the games)
- Bandai Namco Entertainment (for publishing them)
