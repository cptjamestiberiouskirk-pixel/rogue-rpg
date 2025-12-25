/*
 * Graphics rendering implementation for RoguePC
 * SDL2-based tile rendering system
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
SDL_Window *game_window = NULL;
SDL_Texture *tileset_texture = NULL;
SDL_Renderer *tileset_renderer = NULL;

/*
 * create_graphics_window: Initialize SDL window and renderer for graphics mode
 * Called from main.c when -g flag is set
 * Window size: 80 columns × 24 rows × 16px per tile
 */
int
create_graphics_window(void)
{
	int window_width = 80 * TILE_WIDTH;    /* 1280px */
	int window_height = 24 * TILE_HEIGHT;  /* 384px */

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Graphics: SDL initialization failed: %s\n", SDL_GetError());
		return -1;
	}

	game_window = SDL_CreateWindow(
		"RoguePC - Graphics Mode",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		window_width,
		window_height,
		SDL_WINDOW_SHOWN
	);

	if (!game_window) {
		fprintf(stderr, "Graphics: Failed to create window: %s\n", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	tileset_renderer = SDL_CreateRenderer(game_window, -1, 
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (!tileset_renderer) {
		fprintf(stderr, "Graphics: Failed to create renderer: %s\n", SDL_GetError());
		SDL_DestroyWindow(game_window);
		game_window = NULL;
		SDL_Quit();
		return -1;
	}

	/* Clear window with black background */
	SDL_SetRenderDrawColor(tileset_renderer, 0, 0, 0, 255);
	SDL_RenderClear(tileset_renderer);
	
	/* Draw a test rectangle to verify rendering works */
	SDL_Rect test_rect = {100, 100, 200, 100};
	SDL_SetRenderDrawColor(tileset_renderer, 255, 0, 0, 255);  /* Red */
	SDL_RenderFillRect(tileset_renderer, &test_rect);
	
	SDL_RenderPresent(tileset_renderer);

	graphics_enabled = 1;
	fprintf(stderr, "Graphics window created successfully: %dx%d\n", window_width, window_height);
	fprintf(stderr, "tileset_renderer = %p, graphics_enabled = %d\n", (void*)tileset_renderer, graphics_enabled);
	return 0;
}

/*
 * load_tileset: Create tileset texture from procedural tiles
 * Returns: 0 on success, -1 on failure
 * 
 * Note: Since standard image libraries are not reliably available,
 * this implementation creates tile textures procedurally from simple
 * colored rectangles. In production, tiles would be loaded from
 * tilemap.bmp via SDL_LoadBMP(filename).
 */
int
load_tileset(SDL_Renderer *renderer, const char *filename)
{
	SDL_Surface *surface;
	Uint32 *pixels;
	int pitch_pixels;
	int i, j, tile_idx;
	SDL_Color tile_colors[] = {
		{100, 100, 100, 255},  /* Tile 0: gray floor */
		{150, 150, 150, 255},  /* Tile 1: light gray wall */
		{200, 100, 100, 255},  /* Tile 2: red player */
		{100, 150, 100, 255},  /* Tile 3: green plant */
		{200, 200, 100, 255},  /* Tile 4: yellow item */
		{100, 100, 200, 255},  /* Tile 5: blue door */
		{0, 0, 0, 255},        /* Tile 6: black empty */
		{255, 255, 255, 255},  /* Tile 7: white border */
		{50, 50, 50, 255}      /* Tile 8: dark wall */
	};
	int num_colors = sizeof(tile_colors) / sizeof(tile_colors[0]);

	if (!renderer) {
		fprintf(stderr, "Graphics renderer not initialized\n");
		return -1;
	}

	tileset_renderer = renderer;

	/* Try loading from file first */
	surface = SDL_LoadBMP(filename);
	
	if (surface) {
		tileset_texture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface);
		if (tileset_texture) {
			return 0;
		}
	}

	/* Fallback: Create procedural tileset (12x11 grid of 16x16 tiles) */
	fprintf(stderr, "Creating procedural tileset (file '%s' not found)\n", filename);
	
	surface = SDL_CreateRGBSurface(0, 
		TILEMAP_COLS * TILE_WIDTH + (TILEMAP_COLS - 1),
		TILEMAP_ROWS * TILE_HEIGHT + (TILEMAP_ROWS - 1),
		32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

	if (!surface) {
		fprintf(stderr, "Failed to create tile surface: %s\n", SDL_GetError());
		return -1;
	}

	pixels = (Uint32 *)surface->pixels;
	pitch_pixels = surface->pitch / sizeof(Uint32);

	/* Fill surface with tiles */
	for (i = 0; i < TILEMAP_ROWS; i++) {
		for (j = 0; j < TILEMAP_COLS; j++) {
			int base_x = j * (TILE_WIDTH + 1);
			int base_y = i * (TILE_HEIGHT + 1);
			tile_idx = (i * TILEMAP_COLS + j) % num_colors;
			SDL_Color color = tile_colors[tile_idx];
			Uint32 pixel = SDL_MapRGB(surface->format, color.r, color.g, color.b);

			/* Draw tile rectangle */
			for (int y = 0; y < TILE_HEIGHT; y++) {
				for (int x = 0; x < TILE_WIDTH; x++) {
					if (base_x + x < surface->w && base_y + y < surface->h) {
						pixels[(base_y + y) * pitch_pixels + (base_x + x)] = pixel;
					}
				}
			}
		}
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
 * unload_tileset: Cleanup tileset and SDL resources
 */
void
unload_tileset(void)
{
	if (tileset_texture) {
		SDL_DestroyTexture(tileset_texture);
		tileset_texture = NULL;
	}
	if (tileset_renderer) {
		SDL_DestroyRenderer(tileset_renderer);
		tileset_renderer = NULL;
	}
	if (game_window) {
		SDL_DestroyWindow(game_window);
		game_window = NULL;
	}
	SDL_Quit();
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
 * render_tile: Draw a single tile to screen
 * screen_x, screen_y: destination in pixels
 * tile_col, tile_row: tile coordinates (for color selection)
 */
void
render_tile(int screen_x, int screen_y, int tile_col, int tile_row)
{
	SDL_Rect dst_rect;
	Uint8 r, g, b;
	static int draw_count = 0;

	if (!tileset_renderer) {
		if (draw_count == 0)
			fprintf(stderr, "render_tile: tileset_renderer is NULL!\n");
		draw_count++;
		return;
	}

	dst_rect.x = screen_x;
	dst_rect.y = screen_y;
	dst_rect.w = TILE_WIDTH;
	dst_rect.h = TILE_HEIGHT;

	/* Determine color based on tile type */
	if (tile_col == TILE_WALL_COL && tile_row == TILE_WALL_ROW) {
		/* Wall: dark gray */
		r = 64; g = 64; b = 64;
	} else if (tile_col == TILE_FLOOR_COL && tile_row == TILE_FLOOR_ROW) {
		/* Floor: tan/beige */
		r = 200; g = 190; b = 170;
	} else if (tile_col == TILE_DOOR_COL && tile_row == TILE_DOOR_ROW) {
		/* Door: brown */
		r = 139; g = 69; b = 19;
	} else if (tile_col == TILE_PLAYER_COL && tile_row == TILE_PLAYER_ROW) {
		/* Player: bright yellow */
		r = 255; g = 255; b = 0;
	} else {
		/* Default: dark background */
		r = 0; g = 0; b = 0;
	}

	if (draw_count < 5) {
		fprintf(stderr, "Drawing rect at (%d,%d) size %dx%d color (%d,%d,%d)\n",
			screen_x, screen_y, TILE_WIDTH, TILE_HEIGHT, r, g, b);
		draw_count++;
	}

	SDL_SetRenderDrawColor(tileset_renderer, r, g, b, 255);
	SDL_RenderFillRect(tileset_renderer, &dst_rect);

	/* Draw border for visibility */
	SDL_SetRenderDrawColor(tileset_renderer, 128, 128, 128, 255);
	SDL_RenderDrawRect(tileset_renderer, &dst_rect);
}

/*
 * render_dungeon_tile: Render character as tile if graphics enabled
 * Falls back to ASCII rendering if no tile mapping exists
 */
void
render_dungeon_tile(int screen_x, int screen_y, char ch)
{
	int tile_col, tile_row;
	static int render_count = 0;

	if (!graphics_enabled)
		return;

	if (!tileset_renderer) {
		if (render_count == 0)
			fprintf(stderr, "render_dungeon_tile: tileset_renderer is NULL!\n");
		render_count++;
		return;
	}

	if (get_tile_index(ch, &tile_col, &tile_row) == 0) {
		if (render_count < 10) {
			fprintf(stderr, "Rendering tile '%c' at (%d,%d) -> tile (%d,%d)\n", 
				ch, screen_x, screen_y, tile_col, tile_row);
			render_count++;
		}
		render_tile(screen_x, screen_y, tile_col, tile_row);
	}
	/* If no tile mapping, ASCII rendering handles it */
}

#endif /* ROGUE_GRAPHICS */
