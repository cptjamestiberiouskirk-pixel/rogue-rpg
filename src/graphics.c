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
#include "font.h"

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
	SDL_RenderPresent(tileset_renderer);

	graphics_enabled = 1;
	fprintf(stderr, "Graphics window created successfully: %dx%d\n", window_width, window_height);
	return 0;
}

/*
 * load_tileset: Load tileset texture from tilemap.bmp
 * Returns: 0 on success, -1 on failure
 */
int
load_tileset(SDL_Renderer *renderer, const char *filename)
{
	SDL_Surface *surface;

	if (!renderer) {
		fprintf(stderr, "load_tileset: renderer is NULL\n");
		return -1;
	}

	tileset_renderer = renderer;

	/* Load BMP file */
	surface = SDL_LoadBMP(filename);
	if (!surface) {
		fprintf(stderr, "Failed to load tileset from '%s': %s\n", 
			filename, SDL_GetError());
		return -1;
	}

	tileset_texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	if (!tileset_texture) {
		fprintf(stderr, "Failed to create tileset texture: %s\n", SDL_GetError());
		return -1;
	}

	fprintf(stderr, "Tileset loaded successfully from '%s'\n", filename);
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
	case '!':  /* Potion */
		*col = TILE_POTION_COL;
		*row = TILE_POTION_ROW;
		return 0;
	case '?':  /* Scroll */
		*col = TILE_SCROLL_COL;
		*row = TILE_SCROLL_ROW;
		return 0;
	case ')':  /* Weapon */
		*col = TILE_WEAPON_COL;
		*row = TILE_WEAPON_ROW;
		return 0;
	case '%':  /* Stairs */
		*col = TILE_STAIRS_COL;
		*row = TILE_STAIRS_ROW;
		return 0;
	case '.':  /* Floor */
		*col = TILE_FLOOR_COL;
		*row = TILE_FLOOR_ROW;
		return 0;
	case '#':  /* Wall */
		*col = TILE_WALL_COL;
		*row = TILE_WALL_ROW;
		return 0;
	case '+':  /* Door */
		*col = TILE_DOOR_COL;
		*row = TILE_DOOR_ROW;
		return 0;
	case '@':  /* Hero */
		*col = TILE_HERO_COL;
		*row = TILE_HERO_ROW;
		return 0;
	default:
		/* No tile mapping - fallback to ASCII */
		return -1;
	}
}

/*
 * render_tile: Draw a single tile from sprite sheet to screen
 * screen_x, screen_y: destination pixel coordinates on screen
 * tile_col, tile_row: tile grid coordinates in tilemap.bmp
 */
void
render_tile(int screen_x, int screen_y, int tile_col, int tile_row)
{
	SDL_Rect src_rect;
	SDL_Rect dst_rect;

	if (!tileset_renderer || !tileset_texture) {
		return;
	}

	/* Source: The specific sprite on the sheet */
	src_rect.x = tile_col * TILE_WIDTH;
	src_rect.y = tile_row * TILE_HEIGHT;
	src_rect.w = TILE_WIDTH;
	src_rect.h = TILE_HEIGHT;

	/* Destination: Where to draw on the screen */
	dst_rect.x = screen_x;
	dst_rect.y = screen_y;
	dst_rect.w = TILE_WIDTH;
	dst_rect.h = TILE_HEIGHT;

	/* Render */
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

	if (!tileset_renderer || !tileset_texture)
		return;

	/* Try to get sprite coordinates */
	if (get_tile_index(ch, &tile_col, &tile_row) == 0) {
		render_tile(screen_x, screen_y, tile_col, tile_row);
	}
	/* If no tile mapping, ASCII rendering in curses.c handles it */
}

/*
 * graphics_draw_char: Render an 8x8 character scaled to 16x16 pixels
 * x, y: character grid coordinates (not pixel coordinates)
 * c: ASCII character to draw (0-127)
 * 
 * BUG FIX: Corrected bit order - font8x8_basic uses MSB-first encoding
 * (Bit 7 = leftmost pixel, Bit 0 = rightmost pixel)
 */
void
graphics_draw_char(int x, int y, char c)
{
	int i, j;
	const unsigned char *bitmap;
	int screen_x, screen_y;

	if (!tileset_renderer)
		return;

	/* Clamp character to valid ASCII range */
	c = (unsigned char)c & 0x7F;

	/* Get bitmap from font.h */
	bitmap = font8x8_basic[(unsigned char)c];

	/* Convert character grid position to pixel coordinates */
	screen_x = x * TILE_WIDTH;
	screen_y = y * TILE_HEIGHT;

	/* BUG FIX: Set draw color to WHITE for text visibility */
	SDL_SetRenderDrawColor(tileset_renderer, 255, 255, 255, 255);

	/* Draw 8x8 font scaled to 16x16 (2x scale) */
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			/* BUG FIX: Read bits MSB-first (0x80 >> j) instead of LSB-first (1 << j)
			 * Font bitmap encoding: Bit 7 = leftmost pixel, Bit 0 = rightmost pixel
			 */
			if (bitmap[i] & (0x80 >> j)) {
				/* Draw 2x2 pixel block for better visibility */
				SDL_RenderDrawPoint(tileset_renderer, screen_x + j * 2,     screen_y + i * 2);
				SDL_RenderDrawPoint(tileset_renderer, screen_x + j * 2 + 1, screen_y + i * 2);
				SDL_RenderDrawPoint(tileset_renderer, screen_x + j * 2,     screen_y + i * 2 + 1);
				SDL_RenderDrawPoint(tileset_renderer, screen_x + j * 2 + 1, screen_y + i * 2 + 1);
			}
		}
	}

	/* Restore color to black for background */
	SDL_SetRenderDrawColor(tileset_renderer, 0, 0, 0, 255);

	/* BUG FIX: Force immediate screen update to prevent black screen */
	SDL_RenderPresent(tileset_renderer);
}

/*
 * graphics_read_key: Read keyboard input from SDL events
 * Returns: ASCII character code if key pressed
 * 
 * Handles SDL_QUIT and SDL_WINDOWEVENT to prevent zombie window
 * Blocking behavior: Waits for key press (matches readchar() semantics)
 */
int
graphics_read_key(void)
{
	SDL_Event event;

	if (!tileset_renderer)
		return 0;

	/* Blocking loop - wait for key press */
	while (1) {
		/* BUG FIX: Use SDL_WaitEvent for proper blocking without CPU spin */
		if (SDL_WaitEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				/* User closed window - exit gracefully */
				fprintf(stderr, "Graphics window closed by user\n");
				SDL_Quit();
				exit(0);
				break;

			case SDL_KEYDOWN:
				{
					SDL_Keysym keysym = event.key.keysym;
					
					/* Handle special keys */
					switch (keysym.sym) {
					case SDLK_RETURN:
					case SDLK_KP_ENTER:
						return '\n';
					case SDLK_ESCAPE:
						return 27;  /* ESC */
					case SDLK_BACKSPACE:
						return '\b';
					case SDLK_TAB:
						return '\t';
					case SDLK_SPACE:
						return ' ';
					case SDLK_UP:
						return 'k';  /* Rogue movement */
					case SDLK_DOWN:
						return 'j';
					case SDLK_LEFT:
						return 'h';
					case SDLK_RIGHT:
						return 'l';
					}

					/* Handle printable ASCII characters */
					if (keysym.sym >= 32 && keysym.sym <= 126) {
						return (int)keysym.sym;
					}
				}
				break;

			case SDL_WINDOWEVENT:
				/* BUG FIX: Repaint window when exposed/restored */
				if (event.window.event == SDL_WINDOWEVENT_EXPOSED) {
					SDL_RenderPresent(tileset_renderer);
				}
				break;
			}
	}
}  /* end while(1) */
}

#endif /* ROGUE_GRAPHICS */
