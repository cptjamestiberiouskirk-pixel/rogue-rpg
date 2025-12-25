# RoguePC Graphics Mode

## Overview

RoguePC now supports an optional graphical tileset rendering mode that overlays SDL2-rendered tiles on top of the traditional terminal-based gameplay. This feature provides a modernized visual experience while maintaining full backward compatibility with the classic ASCII-only text mode.

## Features

- **SDL2-based Rendering:** Uses SDL2 for hardware-accelerated 2D tile rendering
- **Procedural Fallback:** Automatically generates colored placeholder tiles if tilemap image unavailable
- **Non-invasive Integration:** Graphics mode is completely optional and doesn't affect ASCII rendering
- **Terminal Compatible:** Works alongside full terminal compatibility (xterm, GNOME Terminal, etc.)
- **Charset Independence:** No impact on character-set selection (ASCII/CP437/Unicode)

## Building with Graphics Support

### Standard Build (ASCII-only)
```bash
cd src
make clean
make rogue
```

### Graphics-Enabled Build
```bash
cd src
make clean
make rogue ROGUE_GRAPHICS=1
```

The `ROGUE_GRAPHICS=1` flag conditionally compiles the graphics module and links SDL2 libraries.

### Build Requirements for Graphics Mode
- SDL2 development libraries (`libSDL2-dev` or equivalent)
- Standard C compiler with C17/C18 support
- POSIX-compliant system

## Running Graphics Mode

### Enable Graphics at Runtime
```bash
./src/rogue -g
```

The `-g` flag enables graphics rendering if the binary was compiled with `ROGUE_GRAPHICS=1`.

### Disable Graphics (even if compiled)
```bash
./src/rogue      # ASCII-only, no graphics even if ROGUE_GRAPHICS=1
./src/rogue -G   # Explicitly disable graphics
```

## Tile Mapping

The graphics system maps dungeon characters to specific tileset coordinates:

| Character | Tile Type | Coordinates | Dungeon Element |
|-----------|-----------|-------------|-----------------|
| `#` | Wall | (5, 8) | Walls and barriers |
| `.` | Floor | (0, 4) | Empty walkable space |
| `+` | Door | (4, 6) | Doors and transitions |
| `@` | Player | (1, 8) | Player character |
| Other | ASCII | — | Fallback to text rendering |

## Tileset Format

### Supported Formats
- **BMP files:** Native SDL2 support (recommended)
- **PNG files:** Automatic fallback to procedural generation if unavailable
- **Procedural tiles:** Generated if no external tilemap image found

### Tileset Specifications

- **Tile Dimensions:** 16×16 pixels
- **Tilemap Grid:** 12 columns × 11 rows (132 tiles total)
- **Spacing:** 1 pixel between tiles (accounted for in coordinate calculations)

### Creating a Custom Tileset

To create a custom tileset:

1. Design or obtain a 16×16 pixel tile image
2. Arrange tiles in a 12×11 grid with 1-pixel spacing: `(203 × 186 pixels)`
3. Save as `assets/tilemap.bmp` (or convert PNG to BMP)
4. Rebuild: `make clean && make rogue ROGUE_GRAPHICS=1`

## Implementation Details

### Architecture

The graphics implementation is built as a non-invasive overlay:

1. **Header:** [`src/graphics.h`](src/graphics.h) defines tile indices and public API
2. **Implementation:** [`src/graphics.c`](src/graphics.c) handles tileset loading and SDL rendering
3. **Integration:** [`src/curses.c`](src/curses.c) hooks graphics calls into character output
4. **Build System:** [`src/Makefile`](src/Makefile) conditionally compiles graphics module

### Rendering Pipeline

```
Game Logic (rogue.c)
  ↓
Character Output (cur_addch)
  ├→ [Graphics Path] → render_dungeon_tile() → SDL2 tile rendering
  └→ [ASCII Path] → ncurses terminal rendering
```

When graphics are enabled and a supported character is rendered:

1. `cur_addch()` calls `render_dungeon_tile()` before terminal output
2. `render_dungeon_tile()` maps the character to a tile coordinate
3. SDL2 renders the tile sprite to the screen
4. Terminal rendering continues for characters without graphic tiles

### Cursor Tracking

The graphics subsystem maintains independent cursor position tracking:

```c
static int graphics_cursor_row = 0;
static int graphics_cursor_col = 0;
```

These track the current terminal cell position for proper tile placement.

## Feature Limitations and Future Work

### Current Limitations
- **Monster Sprites:** Creatures render as ASCII; procedural sprite support not yet implemented
- **Lighting Effects:** No dynamic lighting or shadow calculations
- **Animations:** Static tile rendering; animation support future enhancement
- **Terminal Integration:** Graphics render to SDL window only, not in terminal

### Future Enhancements
- Dynamic tileset switching based on dungeon level
- Procedural creature sprite generation
- Lighting system with light radius calculations
- Simple sprite animations (torches flickering, water flowing)
- Custom color palette support
- Save/load visual settings

## Troubleshooting

### Graphics don't appear with `-g` flag

**Possible Causes:**
1. Binary not compiled with `ROGUE_GRAPHICS=1`
   - Solution: Rebuild with `make ROGUE_GRAPHICS=1`

2. SDL2 not initialized properly
   - Check terminal output for "Graphics renderer not initialized"
   - Verify SDL2 libraries installed: `pkg-config --exists SDL2`

3. Tileset file not found
   - Game will automatically use procedural tiles (colored rectangles)
   - For custom tileset: ensure `tilemap.bmp` in correct path

### Build fails with ROGUE_GRAPHICS=1

**Common Issues:**
- SDL2 headers missing: `pkg-config --cflags SDL2`
- SDL2 libraries not found: `pkg-config --libs SDL2`
- Solution: Install SDL2 dev libraries (`libsdl2-dev` on Debian/Ubuntu)

### Game crashes when graphics enabled

1. Run with debugging: `./src/rogue -g 2>&1 | tee debug.log`
2. Check terminal output for SDL initialization errors
3. Verify SDL2 version: `sdl2-config --version`

## Code Examples

### Enabling Graphics Programmatically

In custom game code (e.g., after initializing display):

```c
#ifdef ROGUE_GRAPHICS
if (load_tileset(renderer, "assets/tilemap.bmp") == 0) {
    extern int graphics_enabled;
    graphics_enabled = 1;
}
#endif
```

### Adding New Tile Mappings

Edit `get_tile_index()` in [`src/graphics.c`](src/graphics.c):

```c
case 'M':  /* Custom tile for treasure chests */
    *col = TREASURE_CHEST_COL;
    *row = TREASURE_CHEST_ROW;
    return 0;
```

## Performance Considerations

- Graphics overlay adds minimal CPU overhead (single SDL_Rect blit per character)
- Terminal rendering unchanged (parallel, not sequential)
- Tile caching via SDL2 texture (loaded once at startup)
- Procedural tiles generated once during initialization

## Platform Support

| Platform | ASCII Mode | Graphics Mode |
|----------|-----------|---------------|
| Linux (x86_64) | ✓ | ✓ |
| Linux (ARM) | ✓ | ✓* |
| macOS | ✓ | ✓* |
| Windows (MSYS2/WSL) | ✓ | ✓* |

<sup>*Requires SDL2 development libraries</sup>

## License and Attribution

The graphics subsystem is original code created for the RoguePC project, maintaining strict adherence to ISO C17/C18 and POSIX standards.

Original 1985 Rogue game © Epyx, Inc.  
Graphics implementation © RoguePC Contributors

## See Also

- [README.md](README.md) - Main project documentation
- [src/graphics.h](src/graphics.h) - Graphics API documentation
- [src/graphics.c](src/graphics.c) - Rendering implementation
- [src/Makefile](src/Makefile) - Build configuration
