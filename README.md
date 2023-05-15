# O3DE Jam 2305

This repository contains the source code of the prototype that was developed and submitted to the [O3DE Jam - May 5-14, 2023](https://itch.io/jam/o3de-jam). Please see instructions below to setup your environment and start playing it!

**NOTE: At the moment I won't accept any pull request**. You can still help this project providing feedback about the gameplay or reporting any technical error that you have encountered. Please consider [opening an issue](https://github.com/loherangrin/games.o3de.o3de-jam-2305/issues) in the project tracker.

## Game Information

### Description
In a near-future the space exploration has reached most of the planets in the Universe. Technology has evolved to transform desert wastes into clean urban areas instantly. But new planets can be hostile and new settlements have to be always protected from natural disasters. How long can you last?

### Controls
| Keys | Description |
| :---: | :--- |
| W / S | Move forward / backward |
| A / D | Turn left / right |
| Space | Toggle the beam |
| E | Land on a platform (if any exists) |
| P | Pause the game / Complete the mission (only after landing) |

### Goal
Use the beam to transform more tiles as possible without loosing all the energy of the spaceship.

### Rules
- The spaceship consumes energy to move forward and backward, but it can rotate freely.
- When the beam is activated, the spaceship transfers its energy to the selected tiles, transforming them from dust to lawn (or just providing additional energy, if they were already transformed).
- Each transformed tile generates 1 point every 5 seconds, but it will slowly decay returning to its initial state (dust) if no more energy is supplied.
- The decay speed of a tile is affected by the number of already transformed neighbors. While all of them are transformed, the tile doesn't decay anymore.
- When the spaceship energy is lower than 20%, the power-saving mode is activated automatically. Move speed is halved not to consume more energy. Moreover, the beam cannot be activated.
- Energy can be recharged landing on a platform. Spaceship can take off at any time, even if the recharge isn't fully completed.
- Storms can emerge at any place in the map and move in a random direction for a limited time. They reduce the energy of the spaceship or any transformed tiles that they hit along their path.
- Tiles are randomly generated at each sesson. Keep playing to discover new planets!

### Power-ups
After transforming a tile, one of the following collectables might be dropped from it:
| Keys | Description |
| :--- | :--- |
| Coins | Add points to final score |
| Gem | Add extra energy to the spaceship |
| Spikes | Reduce energy of the spaceship |
| Heart | Add extra energy to all transformed tiles |
| Bomb | Reduce energy of all transformed tiles |
| Lever | Speed up the spaceship for a limited time |
| Red flag | Slow down the spaceship for a limited time |
| Lock | Block the decay of all transformed tiles for a limited time |

## Installation
The project uses free pre-made asset packs for models and textures, which are developed and distributed by third-parties under permissive licenses (Creative Commons CC0). Before proceding, please download the following archives from [Kenney.nl](https://www.kenney.nl) site:
   - [Platformer Kit](https://www.kenney.nl/assets/platformer-kit)
   - [Space Kit](https://www.kenney.nl/assets/space-kit)
   - [UI Pack - Space Expansion](https://www.kenney.nl/assets/ui-pack-space-expansion)

Choose one of the following options to install this project:

### 1. Automatic installation
Following steps require to install **O3Tanks**, a version manager for O3DE to streamline the development workflow. You can find additional information at its [main repository](https://github.com/loherangrin/o3tanks/).

1. Download the game and the related engine version (if not installed yet):
```
o3tanks install project https://<this_repository_url>.git
```
2. Extract downloaded archives into the following destinations:
   - Platformer Kit >> `<game_dir>/Assets/3rdParty/Kenney/PlatformerKit`
   - Space Kit >> `<game_dir>/Assets/3rdParty/Kenney/SpaceKit`
   - UI Pack - Space Expansion  >> `<game_dir>/Assets/3rdParty/Kenney/UiPack-SpaceExpansion`

   Please take care to **merge** the extracted directories with the existing ones, in order to keep the asset metadata file that are required for import.

3. Build the game and all required assets:
```
o3tanks build client
o3tanks build assets
```
4. Run the game in a new window:
```
o3tanks run client
```

### 2. Manual installation

1. Install [O3DE (Open 3D Engine)](https://www.o3de.org) 2305.0 (or greater) following the steps that are described in [Get Started](https://www.o3de.org/docs/welcome-guide/setup/) page of the official O3DE documentation.
2. Clone this repository into a directory of your choice:
```
git clone https://<this_repository_url>.git <game_dir>
```
3. Extract downloaded archives into the following destinations:
   - Platformer Kit >> `<game_dir>/Assets/3rdParty/Kenney/PlatformerKit`
   - Space Kit >> `<game_dir>/Assets/3rdParty/Kenney/SpaceKit`
   - UI Pack - Space Expansion  >> `<game_dir>/Assets/3rdParty/Kenney/UiPack-SpaceExpansion`

   Please take care to **merge** the extracted directories with the existing ones, in order to keep the asset metadata file that are required for import.
4. Register the project in O3DE Project Manager and select `Build Now` to start building the project.
5. Open the project and select `MainLevel.prefab`
6. Press `Play Game` at the right-corner of O3DE Editor to start playing in the viewport

## License
The source code of this project is licensed under the Apache License, version 2.0. Please see LICENSE and NOTICE files for further details.
