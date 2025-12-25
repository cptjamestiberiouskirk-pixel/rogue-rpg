/*
 * Graphics rendering implementation for RoguePC
 *
 * graphics.c
 */

#ifdef ROGUE_GRAPHICS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "graphics.h"

int graphics_enabled = 0;
SDL_Texture *tileset_texture = NULL;
SDL_Renderer *tileset_renderer = NULL;

/*
 * load_tileset: Load tilemap.png and create SDL texture
 * Returns: 0 on success, -1 on failure
 */
int
load_tileset(SDL_Renderer *renderer, const char *filename)
{
	SDL_Surface *surface;

	if (!renderer) {
		fprintf(stderr, "Graphics renderer not initialized\n");
		return -1;
	}

	tileset_renderer = renderer;

	/* Load using SDL_LoadBMP (no external image library required) */
	surface = SDL_LoadBMP(filename);

	if (!surface) {
		fprintf(stderr, "Failed to load tileset: %s\n", SDL_GetError());
		return -1;
	}

	tileset_texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	if (!tileset_texture) {
		fprintf(stderr, "Failed to create tileset texture: %s\n", SDL_GetError());
		return -1;
	}

	graphics_enabled = 1;
	return 0;
}

/*
 * unload_tileset: Cleanup tileset resources
 */
void
unload_tileset(void)
{
	if (tileset_texture) {
		SDL_DestroyTexture(tileset_texture);
		tileset_texture = NULL;
	}
	tileset_renderer = NULL;
	graphics_enabled = 0;
}

/*
 * get_tile_index: Map character to tileset coordinates
 * Returns: 0 on success, -1 if character has no tile mapping
 */
int
get_tile_index(char ch, int *col, int *row)
{
	if (!col || !row)
		return -1;

	switch (ch) {
	case '#':  /* Wall */
		*col = TILE_WALL_COL;
		*row = TILE_WALL_ROW;
		return 0;
	case '.':  /* Floor */
		*col = TILE_FLOOR_COL;
		*row = TILE_FLOOR_ROW;
		return 0;
	case '+':  /* Door */
		*col = TILE_DOOR_COL;
		*row = TILE_DOOR_ROW;
		return 0;
	case '@':  /* Player */
		*col = TILE_PLAYER_COL;
		*row = TILE_PLAYER_ROW;
		return 0;
	default:
		/* Monsters and other entities: render as ASCII for now */
		return -1;
	}
}

/*
 * render_tile: Draw a single tile from tileset to screen
 * screen_x, screen_y: destination in pixels
 * tile_col, tile_row: tile coordinates in tileset grid
 */
void
render_tile(int screen_x, int screen_y, int tile_col, int tile_row)
{
	SDL_Rect src_rect, dst_rect;

	if (!tileset_texture || !tileset_renderer)
		return;

	/* Source rect in tileset (accounting for 1px spacing) */
	src_rect.x = tile_col * (TILE_WIDTH + 1);
	src_rect.y = tile_row * (TILE_HEIGHT + 1);
	src_rect.w = TILE_WIDTH;
	src_rect.h = TILE_HEIGHT;

	/* Destination rect on screen */
	dst_rect.x = screen_x;
	dst_rect.y = screen_y;
	dst_rect.w = TILE_WIDTH;
	dst_rect.h = TILE_HEIGHT;

	SDL_RenderCopy(tileset_renderer, tileset_texture, &src_rect, &dst_rect);
}

/*
 * render_dungeon_tile: Render character as tile if graphics enabled
 * Falls back to ASCII rendering if no tile mapping exists
 */
void
render_dungeon_tile(int screen_x, int screen_y, char ch)
{
	int tile_col, tile_row;

	if (!graphics_enabled)
		return;

	if (get_tile_index(ch, &tile_col, &tile_row) == 0) {
		render_tile(screen_x, screen_y, tile_col, tile_row);
	}
	/* If no tile mapping, ASCII rendering handles it */
}

#endif /* ROGUE_GRAPHICS */
