/*
 * Various input/output functions
 *
 * io.c		1.4		(A.I. Design) 12/10/84
 */

#include	"rogue.h"
#include	"curses.h"

#define AC(a) (-((a)-11))
#define PT(i,j) ((COLS==40)?i:j)
/*
 * msg:
 *	Display a message at the top of the screen.
 */
static int newpos = 0;

#define MSG_LOG_SIZE 100

static char message_log[MSG_LOG_SIZE][BUFSIZE];
static int message_log_start = 0;
static int message_log_count = 0;

static void message_log_store(const char *msg);
static int message_log_slot(int index);
static const char *message_log_get(int index);

/* VARARGS1 */
/*@ nope, it was not vargars. But now it is */
void
ifterse(const char *tfmt, const char *fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);

	if (expert)
		vmsg(tfmt, argp);
	else
		vmsg(fmt, argp);

	va_end(argp);
}

//@ va_list variant of msg()
void
vmsg(const char *fmt, va_list argp)
{
	/*
	 * if the string is "", just clear the line
	 */
	if (*fmt == '\0')
	{
		move(0, 0);
		clrtoeol();
		mpos = 0;
		return;
	}
	/*
	 * otherwise add to the message and flush it out
	 */
	doadd(fmt, argp);
	endmsg();
}

//@ varargs variant, now a wrapper for vmsg()
void
msg(const char *fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);

	vmsg(fmt, argp);

	va_end(argp);
}
/* VARARGS1
 * @ now for real
 */
/*
 * addmsg:
 *	Add things to the current message
 */
void
addmsg(const char *fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);

	doadd(fmt, argp);

	va_end(argp);
}

/*
 * endmsg:
 *	Display a new msg (giving him a chance to see the previous one
 *	if it is up there with the -More-)
 */
void
endmsg(void)
{
	if (save_msg)
		strcpy(huh, msgbuf);
	if (mpos) {
		look(FALSE);
		move(0,mpos);
		more(" More ");
	}
	/*
	 * All messages should start with uppercase, except ones that
	 * start with a pack addressing character
	 */
	if (is_lower(msgbuf[0]) && msgbuf[1] != ')')
		msgbuf[0] = toupper(msgbuf[0]);
	message_log_store(msgbuf);
	putmsg(0,msgbuf);
	mpos = newpos;
	newpos = 0;
}


/*
 *  More:  tag the end of a line and wait for a space
 */
void
more(msg)
	char *msg;
{
	int x, y;
	register int i, msz;
	char mbuf[80];
	int morethere = TRUE;
	int covered = FALSE;

	msz = strlen(msg);
	getxy(&x,&y);
	/*
	 * it is reasonable to assume that if the you are no longer
	 * on line 0, you must have wrapped.
	 */
	if (x != 0) {
		x=0;
		y=COLS;
	}
	if ((y+msz)>COLS) {
		move(x,y=COLS-msz);
		covered = TRUE;
	}

	for(i=0;i<msz;i++) {
		mbuf[i] = inch();
		if ((i+y) < (COLS-2))
			move(x,y+i+1);
		mbuf[i+1] = 0;
	}

	move(x,y);
	standout();
	addstr(msg);
	standend();

	while (readchar() != ' ') {
		if (covered && morethere) {
			move(x,y);
			addstr(mbuf);
			morethere = FALSE;
		}
		else if (covered)
		{
			move(x,y);
			standout();
			addstr(msg);
			standend();
			morethere = TRUE;
		}
	}
	move(x,y);
	addstr(mbuf);
}


/*@
* arguments changed from fixed ints to va_list.
* no need of a varargs version as this is only used internally by io.c
* varargs-aware functions
*/
/*
 * doadd:
 *	Perform an add onto the message buffer
 */
void
doadd(const char *fmt, va_list argp)
{

	vsnprintf(&msgbuf[newpos], BUFSIZE - newpos, fmt, argp);
	newpos = strlen(msgbuf);
}

static void
message_log_store(const char *msg)
{
	int slot;

	if (msg == NULL || *msg == '\0')
		return;

	if (message_log_count < MSG_LOG_SIZE) {
		slot = message_log_slot(message_log_count);
		message_log_count++;
	} else {
		message_log_start = message_log_slot(1);
		slot = message_log_slot(message_log_count - 1);
	}

	strncpy(message_log[slot], msg, BUFSIZE - 1);
	message_log[slot][BUFSIZE - 1] = '\0';
}

static int
message_log_slot(int index)
{
	return (message_log_start + index) % MSG_LOG_SIZE;
}

static const char *
message_log_get(int index)
{
	return message_log[message_log_slot(index)];
}

/*
 * putmsg:
 *  put a msg on the line, make sure that it will fit, if it won't
 *  scroll msg sideways until he has read it all
 */
void
putmsg(msgline,msg)
	int msgline;
	char *msg;
{
	register char *curmsg, *lastmsg=0, *tmpmsg;
	int curlen;

	curmsg = msg;
	do {
		scrlmsg(msgline,lastmsg,curmsg);
		newpos = curlen = strlen(curmsg);
		if (curlen > COLS) {
			more(" Cont ");
			lastmsg = curmsg;
			do {
				tmpmsg = strpbrk(curmsg," ");
				/*
				 * If there are no blanks in line
				 */
				if ((tmpmsg==0 || tmpmsg>=&lastmsg[COLS]) && lastmsg==curmsg) {
					curmsg = &lastmsg[COLS];
					break;
				}
				if ((tmpmsg >= (lastmsg+COLS)) || ((signed)strlen(curmsg) < COLS))
					break;
				curmsg = tmpmsg + 1;
			} while (1);
		}
	} while (curlen > COLS);
}

/*
 * show_message_log:
 *	Display recent messages with scrolling support
 */
void
show_message_log(void)
{
	int total = message_log_count;
	int top;
	int lines_per_page;
	bool done = FALSE;

	wdump();
	clear();

	lines_per_page = LINES - 4;
	if (lines_per_page < 1)
		lines_per_page = 1;

	if (total == 0) {
		mvaddstr(0, 0, "Event log is empty.");
		mvaddstr(2, 0, "Press space to continue.");
		cur_refresh();
		for (;;) {
			int ch = readchar();
			if (ch == ' ' || ch == 'q' || ch == ESCAPE)
				break;
		}
		wrestor();
		cur_refresh();
		return;
	}

	top = (total > lines_per_page) ? total - lines_per_page : 0;

	while (!done) {
		int row;
		int idx;
		int limit = min(total, top + lines_per_page);

		clear();
		move(0, 0);
		printw("Event Log (last %d entries)", MSG_LOG_SIZE);
		mvaddstr(1, 0, "k: older  j: newer  b: page up  f: page down  space/q: exit");

		for (row = 0, idx = top; idx < limit; row++, idx++) {
			const char *entry = message_log_get(idx);
			int sequence = total - idx;
			move(row + 2, 0);
			printw("%3d %s", sequence, entry);
		}

		cur_refresh();
		switch (readchar()) {
		case 'k':
			if (top > 0)
				top--;
			break;
		case 'j':
			if (top + lines_per_page < total)
				top++;
			break;
		case 'b':
			top = max(0, top - lines_per_page);
			break;
		case 'f':
			if (top + lines_per_page < total)
				top = min(total - lines_per_page, top + lines_per_page);
			break;
		case 'q':
		case ' ':
		case ESCAPE:
			done = TRUE;
			break;
		default:
			break;
		}
	}

	wrestor();
	cur_refresh();
}

/*
 * scrlmsg:  scroll a message accross the line
 * @ renamed to avoid conflict with <curses.h>.
 * @ Purpose is completely unrelated to curses
 */
void
scrlmsg(msgline,str1,str2)
	int msgline;
	char *str1, *str2;
{
	char *fmt;

	if (COLS > 40)
		fmt = "%.80s";
	else
		fmt = "%.40s";

	if (str1 == 0) {
		move(msgline,0);
		if ((signed)strlen(str2) < COLS)
			clrtoeol();
		printw(fmt,str2);
	}
	else
		while (str1 <= str2) {
			move(msgline,0);
			printw(fmt,str1++);
			if ((signed)strlen(str1) < (COLS-1))
				clrtoeol();
		}
}
/*
 * io_unctrl:
 *	Print a readable version of a certain character
 *	@ renamed to avoid conflict with <curses.h>
 *	@ same purpose but different behavior, so not using the curses version
 */
char *
io_unctrl(byte ch)
{
	static char chstr[9];		/* Defined in curses library */

	if (is_space(ch))
		strcpy(chstr," ");
	else if (!is_print(ch))
		if (ch < ' ')
			sprintf(chstr, "^%c", ch + '@');
		else
			sprintf(chstr, "\\x%x",ch);
	else {
		chstr[0] = ch;
		chstr[1] = 0;
	}

	return chstr;
}

/*
 * status:
 *	Display the important stats line.  Keep the cursor where it was.
 */
void
status(void)
{
	int oy, ox;
	static int last_lvl = -1;
	static int last_hp = -1;
	static int last_maxhp = -1;
	static str_t last_str = (str_t)-1;
	static str_t last_maxstr = (str_t)-1;
	static int last_ac = -999;
	static int last_slvl = -1;
	static int last_purse = -1;
	static long last_exp = -1L;

	SIG2();

	getyx(stdscr, oy, ox);
	/* Only redraw if any displayed field changed */
	if (last_lvl != level || last_hp != pstats.s_hpt || last_maxhp != max_hp ||
		last_str != pstats.s_str || last_maxstr != max_stats.s_str ||
		last_ac != pstats.s_arm || last_exp != pstats.s_exp ||
		last_slvl != pstats.s_lvl || last_purse != purse) {
		if (is_color)
			yellow();

	/*
	 * Level:
	 */
		move(PT(22,23),0);
		printw("Lvl:%d  HP:%d/%d  Str:%d(%d)  AC:%d  Exp:%ld/%ld  Gold:%d",
			level, pstats.s_hpt, max_hp, pstats.s_str, max_stats.s_str,
			pstats.s_arm, pstats.s_exp, e_levels[pstats.s_lvl], purse);

		if (is_color)
			standend();

		/* Snapshot current values */
		last_lvl = level;
		last_hp = pstats.s_hpt;
		last_maxhp = max_hp;
		last_str = pstats.s_str;
		last_maxstr = max_stats.s_str;
		last_ac = pstats.s_arm;
		last_exp = pstats.s_exp;
		last_slvl = pstats.s_lvl;
		last_purse = purse;
	}

	move(oy, ox);
}

/*
 * wait_for
 *	Sit around until the guy types the right key
 */
void
wait_for(byte ch)
{
	/*@
	 * stdio and ncurses will map all stream line endings to '\n'
	 * Hooray ANSI! :)
	 *
	register char c;

	if (ch == '\n')
		while ((c = readchar()) != '\n' && c != '\r')
			continue;
	else
	 */
	while (readchar() != ch)
		continue;
}

/*@
 * Wait with a message until user press Enter
 * New function, used to block before leaving the game
 */
void
wait_msg(const char *msg)
{
	standend();
	move(LINES-1,0);
	cursor(TRUE);
	if (*msg)
	{
		printw("[Press Enter to %s]", msg);
	}
	else
	{
		printw("[Press Enter]");
	}
	flush_type();
	wait_for('\n');
	move(LINES-1,0);
}

/*
 * show_win:
 *	Function used to display a window and wait before returning
 *	@ a window? looks like a single message to me!
 */
void
show_win(message)
	char *message;
{
	mvaddstr(0,0,message);
	move(hero.y, hero.x);
	wait_for(' ');
}


/*
 * str_attr:  format a string with attributes.
 *
 *    formats:
 *        %i - the following character is turned inverse vidio
 *        %I - All characters upto %$ or null are turned inverse vidio
 *        %u - the following character is underlined
 *        %U - All characters upto %$ or null are underlined
 *        %$ - Turn off all attributes
 *
 *     Attributes do not nest, therefore turning on an attribute while
 *     a different one is in effect simply changes the attribute.
 *
 *     "No attribute" is the default and is set on leaving this routine
 *
 *     Eventually this routine will contain colors and character intensity
 *     attributes.  And I'm not sure how I'm going to interface this with
 *     printf certainly '%' isn't a good choice of characters.  jll.
 */
void
str_attr(str)
	char *str;
{
#ifdef LUXURY
	register int is_attr_on = FALSE, was_touched = FALSE;

	while(*str)
	{
		if (was_touched == TRUE)
		{
			standend();
			is_attr_on = FALSE;
			was_touched = FALSE;
		}
	if (*str == '%')
	{
		str++;
		switch(*str)
		{
		case 'u':
					was_touched = TRUE;
				case 'U':
			uline();
					is_attr_on = TRUE;
					str++;
					break;
				case 'i':
					was_touched = TRUE;
				case 'I':
					standout();
					is_attr_on = TRUE;
					str++;
					break;
				case '$':
					if (is_attr_on)
						was_touched = TRUE;
					str++;
					continue;
			 }
		}
		if ((*str == '\n') || (*str == '\r'))
		{
			str++;
			printw("\n");
		}
		else if (*str != 0)
			addch(*str++);
	}
	if (is_attr_on)
		standend();
#else
	while (*str)
	{
		if (*str == '%') {
			str++;
			standout();
		}
		addch(*str++);
		standend();
	}
#endif //LUXURY
}

/*
 * key_state:
 */
void
SIG2(void)
{
	static int key_init = TRUE;
	static int numl, capsl;
	static int nspot, cspot, tspot;
	register int new_numl, new_capsl, new_fmode;
	static int bighand, littlehand;
	int showtime = FALSE, spare;
	int x, y;
#ifdef DEMO
	static int tot_time = 0;
#endif //DEMO
#ifdef ROGUE_DOS_CLOCK
	static unsigned int ntick = 0;

	//@ only update every 6 ticks, ~3 times per second
	if (tick < ntick)
		return;
	ntick = tick + 6;
#else
	static long cur_time = 0;
	long new_time = md_time();
#endif

	/*@
	 * Do not update between wdump()/wrestor() operations
	 * (when the user is in a non-game screen like inventory or discoveries)
	 * Or if the screen is not yet initialized.
	 */
	if (is_saved || scr_type < 0)
		return;
#ifndef __linux__
	regs->ax = 0x200;
	swint(SW_KEY, regs);
	new_numl = regs->ax;
#else
	new_numl = md_keyboard_leds();
#endif
	new_capsl = new_numl & 0x40;
	new_fmode = new_numl & 0x10;  //@ scroll lock
	new_numl &= 0x20;
#ifdef ROGUE_DOS_CLOCK
	/*
	 * set up the clock the first time here
	 */
	/*@
	 * using DOS INT 21h/AH=2Ch - Get System Time
	 * CH = hour (0-23)
	 * CL = minutes (0-59)
	 */
	if (key_init) {
		regs->ax = 0x2c << 8;
		swint(SW_DOS, regs);
		bighand = (regs->cx >> 8) % 12;  //@ force 12-hour display format
		littlehand = regs->cx & 0xFF;
		showtime = TRUE;
	}
	//@ 1092 ticks = 1 minute @ 18.2 ticks per second rate
	if (tick > 1092) {
		/*
		 * time os call kills jr and others we keep track of it
		 * ourselves
		 */
		littlehand = (littlehand + 1) % 60;
		if (littlehand == 0)
			bighand = (bighand + 1) % 12;
		tick = tick - 1092;
		ntick = tick + 6;
		showtime = TRUE;
	}
#else
	if (new_time - cur_time >= 60)
	{
		TM *local = md_localtime();
		bighand = local->hour % 12;
		littlehand = local->minute;
		cur_time = new_time - local->second;
		showtime = TRUE;
	}
#endif

	/*
	 * this is built for speed so set up once first time this
	 * is executed
	 */
	if (key_init || reinit)
	{
		reinit = key_init = FALSE;
		if (COLS == 40)
		{
			nspot = 10;
			cspot = 19;
			tspot = 35;
		}
		else
		{
			nspot = 20;
			cspot = 39;
			tspot = 75;
		}
		/*
		 * this will force all fields to be updated first time through
		 */
		numl = !new_numl;
		capsl = !new_capsl;
		showtime++;
		faststate = !new_fmode;
	}

	getxy(&x, &y);

	if (faststate != new_fmode)
	{

		faststate = new_fmode;
		count = 0;
		show_count();
		running = FALSE;
		move(LINES-1,0);
		if (faststate)
		{
			bold();
			addstr("Fast Play");
			standend();
		}
		else
		{
			addstr("         ");
		}
	}

	if (numl != new_numl)
	{
		numl = new_numl;
		count = 0;
		show_count();
		running = FALSE;
		move(24,nspot);
		if (numl)
		{
			bold();
			addstr("NUM LOCK");
			standend();
		}
		else
			addstr("        ");
	}
	if (capsl != new_capsl)
	{
		capsl = new_capsl;
		move(24,cspot);
		if (capsl)
		{
			bold();
			addstr("CAP LOCK");
			standend();
		}
		else
			addstr("        ");
	}
	if (showtime)
	{
		showtime = FALSE;
#ifdef DEMO
		/*
		 * Don't let them get by level 10 because they might do something
		 * nasty like disable the clock
		 */
		if (((tot_time++ - max_level) > DEMOTIME) || max_level > 10)
			demo(DEMOTIME);
#endif //DEMO
		/* work around the compiler buggie boos */
		spare = littlehand % 10;
		move(24,tspot);
		bold();
		printw("%2d:%1d%1d",bighand?bighand:12,littlehand/10,spare);
		standend();
	}
	move(x, y);
}

char *
noterse(str)
	char *str;
{
	return( terse || expert ? nullstr : str);
}
