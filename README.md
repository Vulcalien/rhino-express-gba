# Rhino Express for GBA

This is a Game Boy Advance port (or demake) of the game
[Rhino Express](https://github.com/sgaumin/LD53)
by Seb_gamedev, a puzzle game that won first place in the
[Ludum Dare 53](https://ldjam.com/events/ludum-dare/53)
competition.

In this game, the player helps a rhino make deliveries, navigating
levels in order to reach all mailboxes, while also avoiding dangerous
pits and using obstacles to its advantage.

## GBA demake
While making this GBA port, I tried to keep it as similar to the
original game as possible. However, unlike my previous demake
[Minicraft for GBA](https://github.com/Vulcalien/minicraft-gba),
I did not read the source code of the original, choosing instead to
rewrite it based on observation. Images, sounds, music, levels etc...
were taken from the original project and then adapted for use on the
GBA.

I also changed a few things, like replacing mouse input with keypad
controls, and adjusting the location of some things due to the different
aspect ratio.

## Running
Download or build the ROM (`.gba` extension), then open it with a GBA
emulator. If you don't have one installed, I recommend
[mGBA](https://mgba.io/downloads.html).

The game should save automatically after completing a level.

## Building
To build the ROM, you will need the `gcc-arm-none-eabi` compiler and
`Python 3` installed.

Run these commands:
```sh
git submodule update --init
make res
make
```

If all goes well, you should find the ROM `rhino-express.gba` inside the
`bin` directory.

## License
Rhino Express was made by Seb_gamedev for the Ludum Dare 53 competition.
Its code is released under the MIT license.

This demake of the game was made by Vulcalien. Its code is released
under the terms of the GNU General Public License, version 3 or later.
Assets (images, sounds, music, levels) and game mechanics used in this
demake of the game were extracted from the original game and adapted.
