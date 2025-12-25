/*
 * Display abstraction layer for RoguePC
 * Handles both terminal (curses) and graphics (SDL2) rendering
 *
 * display.c
 */

#include <curses.h>
#include "rogue.h"

#ifdef ROGUE_GRAPHICS
#include "graphics.h"
extern SDL_Window *game_window;
extern SDL_Renderer *tileset_renderer;
#endif

/*
 * display_tile: Render a single dungeon tile to screen
 * Handles both ASCII terminal and graphics modes
 */
void
display_tile(int row, int col, char ch)
{
	if (row < 0 || row >= LINES || col < 0 || col >= COLS)
		return;

#ifdef ROGUE_GRAPHICS
	if (graphics_enabled && game_window && tileset_renderer) {
		int screen_x = col * TILE_WIDTH;
		int screen_y = row * TILE_HEIGHT;
		render_dungeon_tile(screen_x, screen_y, ch);
		/* Continue to render ASCII for reference */
	}
#endif

	/* Always render ASCII to terminal for compatibility */
	mvaddch(row, col, ch);
}

/*
 * display_refresh: Update display after rendering changes
 * Flushes both curses and graphics buffers
 */
void
display_refresh(void)
{
#ifdef ROGUE_GRAPHICS
	if (graphics_enabled && game_window && tileset_renderer) {
		SDL_RenderPresent(tileset_renderer);
	}
#endif

	refresh();
}

/*
 * display_clear: Clear entire display
 */
void
display_clear(void)
{
#ifdef ROGUE_GRAPHICS
	if (graphics_enabled && game_window && tileset_renderer) {
		SDL_SetRenderDrawColor(tileset_renderer, 0, 0, 0, 255);
		SDL_RenderClear(tileset_renderer);
	}
#endif

	clear();
}

/*
 * display_move: Move cursor and prepare for output
 */
void
display_move(int row, int col)
{
	if (row < 0 || row >= LINES || col < 0 || col >= COLS)
		return;

#ifdef ROGUE_GRAPHICS
	/* Graphics position tracking already handled in render_dungeon_tile */
#endif

	move(row, col);
}
