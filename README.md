Rogue: Diablo Edition
===============================================================================

## A Total Conversion of the Classic 1985 Rogue into a Modern Action-RPG!

![Title Screen](screenshots/title.png)![Level 1, fully explored](screenshots/level1.png)

<sup>*(Yes, these are actual in-game screenshots using `xterm` with the custom font)*</sup>

A total conversion of the classic 1985 Rogue, grafting modern Action-RPG mechanics onto the original codebase. This version features playable classes, a Diablo-style loot system with prefixes/suffixes, elite boss encounters, and modern quality-of-life improvements.

- Uses original source code from 1985, adapted to compile in modern PCs.
    - All comments from original authors preserved.
    - All changes needed for portability from 16-bit DOS fully marked and explained.
    - Extensive documentation on original code inner mechanics:
      hardware access, BIOS and DOS INT calls, clock and timers, CGA/EGA graphics usage,
      piracy copy protection, splash image, assembly routines, builtin `curses`, etc.
    - Great care taken to preserve as much as possible from original. More than 85% still intact!
- Strictly adheres to C standards to be fully portable to any modern platform.
    - [ISO/IEC 9899:2018 (C17/C18)](https://en.wikipedia.org/wiki/C17_%28C_standard_revision%29), latest ISO C.
    - [POSIX.1-2017](https://pubs.opengroup.org/onlinepubs/9699919799/), latest revision of POSIX 2008.
    - [X/Open Curses, Issue 7](https://publications.opengroup.org/c094), latest Curses specification.
    - [Single UNIX Specification, Version 4 (SUSv4)](https://unix.org/version4/), includes POSIX and Curses.
- Runs in any terminal, text-only, using nothing but `curses`, just like the original.
- Colors and gameplay intact.
- Non-ASCII characters updated from DOS CP437 to suitable Unicode codepoints.

![Game over screen](screenshots/rip.png)
<sup>*(Better get used to, you'll see this... a **lot**)*</sup>

---

### Features

#### Graphical Tileset Rendering
- **Optional Graphics Mode:** Enable tile-based graphics with `-g` flag (requires `ROGUE_GRAPHICS=1` build)
- **Procedural Fallback:** Automatically generates placeholder tiles if tilemap unavailable
- **Terminal Compatible:** Seamless integration with traditional ASCII-based rendering
- **See [GRAPHICS.md](GRAPHICS.md) for full documentation**

#### Class System
Choose your playstyle at the start:
- **Warrior:** Tanky, starts with Ring Mail & Two-Handed Sword.
- **Rogue:** High Crit, starts with Bow, Arrows, & Dagger.
- **Sorcerer:** Magic expert, starts with Wands & Scrolls.

#### Loot 2.0
- **Rarity:** Items drop as Common (White), Magic (Cyan), Rare (Yellow), or Legendary (Magenta).
- **Affixes:** Procedural generation creates items like "Vampiric Long Sword of the Bear" or "Rusty Mace of Weakness".

#### Modern Interface
- **Smart HUD:** Enemy HP bars (e.g., "HP: 5/20") and a unified status line showing all stats.
- **Auto-Equip:** Automatically equips gear that is statistically better than your current loadout.

#### Combat Depth
- **Elite Monsters:** 20% chance for monsters to spawn as Bosses with 3x HP.
- **Monster Abilities:** Bosses have unique affixes: **Vampiric** (Life Steal), **Thorns** (Reflect Dmg), or **Teleporter**.

---

### Installation and Usage

For latest Debian / Ubuntu systems:
```sh
sudo apt install make pkg-config libncurses-dev
cd src
make
./rogue
```

To enable the original splash screen in graphics mode (requires SDL2):
```sh
sudo apt install libsdl2-dev  # plus above requirements (make, ncurses, etc)
cd src
make all
./rogue-sdl
```

For ASCII mode (like UNIX Rogue, but with colors):
```sh
cd src
make ROGUE_CHARSET=1
./rogue
```

To enable graphical tileset rendering (requires SDL2):
```sh
sudo apt install libsdl2-dev
cd src
make rogue ROGUE_GRAPHICS=1
./rogue -g
```

For comprehensive information on graphics mode, see [GRAPHICS.md](GRAPHICS.md).

For Ubuntu 18.04, use `libncursesw5-dev` instead of `libncurses-dev`, or
`libncurses5-dev` for ASCII mode.

Strongly suggested:

- Install the custom `PerfectDOSVGA437Unicode.ttf` font. See [tools](tools/)
- Launch the game using [`./roguepc`](roguepc).
  It will auto-select either `rogue-sdl` or `rogue`, and also choose
  `roguepc-xterm`, `roguepc-gnome-terminal` or none depending on what is
  available in your system.
- If your desktop is 1920x1200 or larger, try `./roguepc --fullscreen -fs 36`
  for the ultimate rogue experience!

![splash image](rogue.png)

<sup>*(Yes, it will display the original splash if you compile with SDL2! \o/)*</sup>

---

### Command-line and enviroment variables

#### Compile/Build time:

- `ROGUE_GRAPHICS`: Enable graphical tileset rendering overlay. Valid values are:
    - Unset (default): ASCII-only rendering via ncurses.
    - `1`: Enable graphics mode with SDL2 tile rendering. Requires `libsdl2-dev`.
    - Build: `make ROGUE_GRAPHICS=1`. Runtime: use `-g` flag or leave unset for ASCII.
    - See [GRAPHICS.md](GRAPHICS.md) for full documentation.
- `ROGUE_CHARSET`: The character set / encoding used by the game. Valid values are:
    - `1`: Plain ASCII. Looks like Rogue UNIX, but with color. Only requires `ncurses`.
    - `2`: Original CP437, as used in DOS. Requires `ncurses`, a terminal capable of handling it,
        and preferably a CP437 font. Good luck!
    - `3`: Unicode/UTF8, the default. Requires `ncursesw` and a Unicode font.
- `ROGUE_NO_X11`: If set, do not use X11/XLib to query the keyboard for NumLock status.
    Unset by default, so `x11` libs are required to compile.
    Only used at run-time if actually running in an X11 terminal emulator.
- `ROGUE_WIDECHAR`: Set if using Unicode charset to invoke the wide char functions in `ncursesw`.
- `ROGUE_SCR_TYPE`: The screen type, or more accurately the mode of the graphics adapter.
    `3` by default, meaning _Text, 80x25, color_. Untested with any other value, but the game
    did adjust itself for monochrome and other modes.
- `ROGUE_DOS_SCREEN`: If set, uses a DOS interrupt at run-time to determine the above,
    as done by the original in 1985. As DOS interrupts were stubbed, will result in an untested value.
- `ROGUE_COLUMNS`: Columns of the video adapter. Should be tied with the above `ROGUE_SCR_TYPE`.
    Defaults to `80` and untested with any other value, but the original game did have some
    special handling and adjustments for 40-columns TVs.
- `ROGUE_DOS_CLOCK`: Use DOS interrupts to access the RTC clock and other time-related events,
    as the original game did. By default uses ISO C functions for time.
- `ROGUE_DOS_DRIVE`: If set, uses a DOS interrupt to find out the current drive letter.
    As all DOS interrupts are dummy stubs in this modern port, it will result in drive `A:`.
    Mostly used for cosmetic purposes, in the _"Fake DOS"_ in-game feature launched with `F10`.
    Does not affect actual paths such as the score file, which always uses the current directory.
- `ROGUE_CURRENT_DRIVE`: The current drive used if the above is unset.
    `0` for `A:`, `1` for `B:`, etc. Defaults to `3` (`C:`).
- `ROGUE_LAST_DRIVE`: The last drive available in the system, `6` by default (`F:`).
    Also purely cosmetic.
- `ROGUE_NOGOOD`: If set, do not circumvent the original anti-piracy, copy-protection routines.
    As this heavily relies on DOS interrupts, assembly code and other direct hardware access
    only available in 1985, this will result in the game acting as if it was illegally copied.
- `ROGUE_RELEASE`: Just sets the additional `CFLAGS` `-s` and `-O2`,
    otherwise it sets `-Og` and `-g3`. No in-game effect.
- `ROGUE_DEBUG`: Enable some in-game debugging messages.
    It disrupts the `curses` display, so the game become somewhat unplayable.
- `ROGUE_DEMO`: Sets `DEMO` for the original Demo mode.

**All** original compile-time environment vars and `-D`efines used as `#ifdef`s in code are preserved!
However, some of them were already non-functional and only partially implemented in the original code.
They are still _untested_ and for now _unsupported_ in this modern port, so many will not work.
Highlights include: _(not a comprehensive list)_
- `DEMO`: Demonstration mode, with gameplay restrictions.
- `DEBUG`: Debug mode, for testing.
- `WIZARD`: "God" mode, with extra commands.
- `INTL`: International release, with different credits.
- `ME`: Be the author!
- `LUXURY`: In 1985, this meant a UNIX machine with an extended C library.
- `MINROG`: Set by default in the original `Makefile`, enables the winning message.

#### Run-time settings:

Files: all preserved from the original! By default all read from and created at the current directory.
- `rogue.opt`: Game options, such as the default player and fruit name, current drive letter, etc.
- `rogue.scr`: High scores, fully working! Path and name can be set in the options above.
- `rogue.sav`: Save state file. Saving and restoring is currently not implemented.
- `rogue.pic`: Splash image. Original game switched to CGA graphics to display,
               this port uses SDL2. See `rogue-sdl`.

Command-line options, all preserved from the original:
- `-s`: Only show the high-scores and exit. Fully working!
- `-r`: Restore a previously-saved game state. Currently a no-op.
- `-l`, `-k`: Only enabled if original `LOGFILE` compile option is set. Unknown effect, if any.
- `-g`: Enable graphics mode (requires binary compiled with `ROGUE_GRAPHICS=1`). See [GRAPHICS.md](GRAPHICS.md).
- `-G`: Explicitly disable graphics mode (even if compiled in).

Note: options are all case-_insensitive_, and can use either `-` or `/`, as the original.
So `-r`, `-R`, `/r` and `/R` are all equivalent.

Environment variables:
- `ROGUE_PIC`: full path to the splash image, by default `rogue.pic` in the _current_ directory.
     Only displayed if using `rogue-sdl` under an SDL2-capable environment such as X11.

---

### Credits

**Original Game (1985):** A.I. Design (Michael Toy, Glenn Wichman, Ken Arnold).

**Modern PC Port:** Roguelike Community.

**Diablo Edition Developer:** James Kirk ([@cptjamestiberiouskirk](https://github.com/cptjamestiberiouskirk))
