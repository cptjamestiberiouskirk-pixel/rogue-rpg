/*
 * Graphics rendering support for RoguePC
 * Uses SDL2 to render tileset-based graphics instead of ASCII
 *
 * graphics.h
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#ifdef ROGUE_GRAPHICS

#include <SDL2/SDL.h>

/*
 * Tile indices in tilemap.bmp (16px × 16px tiles)
 * Layout: 192x176 sprite sheet with 12 columns × 11 rows
 * Mapping: character → {col, row} in tileset
 */

/* Item Row (Row 0) */
#define TILE_POTION_COL  1
#define TILE_POTION_ROW  0
#define TILE_SCROLL_COL  2
#define TILE_SCROLL_ROW  0
#define TILE_WEAPON_COL  4
#define TILE_WEAPON_ROW  0
#define TILE_STAIRS_COL  6
#define TILE_STAIRS_ROW  0

/* Environment */
#define TILE_FLOOR_COL   0
#define TILE_FLOOR_ROW   4
#define TILE_DOOR_COL    4
#define TILE_DOOR_ROW    6
#define TILE_WALL_COL    5
#define TILE_WALL_ROW    8

/* Actors */
#define TILE_HERO_COL    1
#define TILE_HERO_ROW    8

#define TILE_WIDTH       16
#define TILE_HEIGHT      16
#define TILEMAP_COLS     12
#define TILEMAP_ROWS     11

/*
 * Graphics mode state
 */
extern int graphics_enabled;
extern SDL_Window *game_window;
extern SDL_Texture *tileset_texture;
extern SDL_Renderer *tileset_renderer;

/*
 * Function prototypes
 */
int  create_graphics_window(void);
int  load_tileset(SDL_Renderer *renderer, const char *filename);
void unload_tileset(void);
int  get_tile_index(char ch, int *col, int *row);
void render_tile(int screen_x, int screen_y, int tile_col, int tile_row);
int  render_dungeon_tile(int screen_x, int screen_y, char ch);
void graphics_draw_char(int x, int y, char c);
int  graphics_read_key(void);

#endif /* ROGUE_GRAPHICS */

#endif /* GRAPHICS_H */
