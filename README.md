# VC.GPS

A GPS route navigation modification for **Grand Theft Auto: Vice City**. It draws a navigation path on the minimap and supports integration with custom map mods.

## Features

- **Automated GPS Routing:** Automatically draws the shortest GPS path directly to your active mission objective on the minimap.
- **GTA SA GPS Redux Style:** Works entirely automatically in the background just like the modern GPS systems.
- **MenuMap Mod Integration:** Fully supports [gennariarmando/menu-map](https://github.com/gennariarmando/menu-map).
  - If you place a custom waypoint on the interactive pause menu map, the GPS will immediately route to your custom waypoint instead of the mission objective.
  - The route to your custom waypoint is drawn in a light pink color to differentiate it from mission blips.
  - The GPS path is drawn directly onto the interactive pause menu map using the mod's native API.
  - When you arrive at your destination, the custom waypoint is automatically deleted and a checkpoint sound is played.

## Installation

1. Install an ASI Loader (such as Ultimate ASI Loader) for GTA Vice City.
2. Place `VC.GPS.asi` in your game's `scripts` folder or the root directory.
3. (Optional but recommended) Install the MenuMap mod to be able to set custom waypoints.

## Building from Source

This project uses Visual Studio 2022 (v143 Platform Toolset / Windows 10.0 SDK).

- Open `VC.GPS.sln` in Visual Studio.
- Select `Release` and `x86`.
- Build the solution. The compiled `.asi` plugin will be located in the `Release` folder.

## Credits
Based on the original III.VC.GPS project.
