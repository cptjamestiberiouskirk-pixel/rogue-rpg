/*
 *  Cursor motion header for Monochrome display
 */

/*@
 * Contains only the curses-related declarations needed for the game files.
 *
 * Headers used only by curses.c were moved to curses_dos.h
 * Headers used by both curses.c and game files were moved to curses_common.h
 * Headers not related to curses or provided externally were moved elsewhere.
 * Unused headers were removed.
 *
 * The DOS curses implementation, curses.c, shall NOT include this header
 *
 * This is, along with the included curses_common.h header, is the curses
 * public API as used by the game.
 */

#include "curses_common.h"

#define stdscr	NULL
#define hw	stdscr
#define eatme	stdscr

//@ Original macros
#define	wclear	clear
#ifdef mvwaddch
#undef mvwaddch
#endif
#define mvwaddch(w,a,b,c)	mvaddch(a,b,c)
#ifdef getyx
#undef getyx
#endif
#define getyx(a,b,c)	getxy(&b,&c)
#define getxy	getrc

//@ Modified macros
#ifdef inch
#undef inch
#endif
#define inch	cur_inch
#ifdef standend
#undef standend
#endif
#define standend	cur_standend
#ifdef standout
#undef standout
#endif
#define standout	cur_standout
#define endwin	cur_endwin

//@ Function mappings
#define beep	cur_beep
#ifdef move
#undef move
#endif
#define move	cur_move
#ifdef clear
#undef clear
#endif
#define clear	cur_clear
#ifdef clrtoeol
#undef clrtoeol
#endif
#define clrtoeol	cur_clrtoeol
#ifdef mvaddstr
#undef mvaddstr
#endif
#define mvaddstr	cur_mvaddstr
#ifdef mvaddch
#undef mvaddch
#endif
#define mvaddch	cur_mvaddch
#ifdef mvinch
#undef mvinch
#endif
#define mvinch	cur_mvinch
#ifdef addch
#undef addch
#endif
#define addch	cur_addch
#ifdef addstr
#undef addstr
#endif
#define addstr	cur_addstr
#ifdef box
#undef box
#endif
#define box	cur_box
#define printw	cur_printw
#ifdef getch
#undef getch
#endif
#define getch	cur_getch  //@ no longer used
#define getch_timeout	cur_getch_timeout


/*@
 * Global variables declarations. All defined in curses.c
 */
extern int LINES, COLS;
extern int is_saved;
extern int scr_type;
#ifdef ROGUE_DOS_CURSES
extern bool iscuron;
extern int old_page_no;
extern int scr_ds;
extern int svwin_ds;
#endif

/*
 * we need to know location of screen being saved
 * @ used in save.c
 */
extern char savewin[];
