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
 * Tile indices in tilemap.png (16px × 16px tiles, 12 cols × 11 rows)
 * Mapping: character → {col, row} in tileset
 * Each tile is followed by 1px spacing
 */
#define TILE_WALL_COL    5
#define TILE_WALL_ROW    8

#define TILE_FLOOR_COL   0
#define TILE_FLOOR_ROW   4

#define TILE_DOOR_COL    4
#define TILE_DOOR_ROW    6

#define TILE_PLAYER_COL  1
#define TILE_PLAYER_ROW  8

#define TILE_WIDTH       16
#define TILE_HEIGHT      16
#define TILEMAP_COLS     12
#define TILEMAP_ROWS     11

/*
 * Graphics mode state
 */
extern int graphics_enabled;
extern SDL_Texture *tileset_texture;
extern SDL_Renderer *tileset_renderer;

/*
 * Function prototypes
 */
int  load_tileset(SDL_Renderer *renderer, const char *filename);
void unload_tileset(void);
int  get_tile_index(char ch, int *col, int *row);
void render_tile(int screen_x, int screen_y, int tile_col, int tile_row);
void render_dungeon_tile(int screen_x, int screen_y, char ch);

#endif /* ROGUE_GRAPHICS */

#endif /* GRAPHICS_H */
