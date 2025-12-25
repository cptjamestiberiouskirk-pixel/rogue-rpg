/*
 * Routines dealing specifically with rings
 *
 * rings.c		1.4 (AI Design)		12/13/84
 */

#include "rogue.h"
#include "curses.h"

static int	gethand(void);
static void apply_ring_effect(THING *obj);
static int ring_power(const THING *ring);

static void apply_ring_effect(THING *obj)
{
	if (obj == NULL)
		return;

	switch (obj->o_which) {
	case R_ADDSTR:
		chg_str(obj->o_ac);
		break;
	case R_SEEINVIS:
		invis_on();
		break;
	case R_AGGR:
		aggravate();
		break;
	default:
		break;
	}
}

static int
ring_power(const THING *ring)
{
	if (ring == NULL)
		return -1000;

	int score = 0;

	switch (ring->o_which) {
	case R_ADDSTR:
		score = 60 + ring->o_ac * 12;
		break;
	case R_ADDDAM:
		score = 55 + ring->o_ac * 10;
		break;
	case R_ADDHIT:
		score = 50 + ring->o_ac * 8;
		break;
	case R_PROTECT:
		score = 50 + ring->o_ac * 10;
		break;
	case R_REGEN:
		score = 45;
		break;
	case R_DIGEST:
		score = 35;
		break;
	case R_SEARCH:
		score = 30;
		break;
	case R_SEEINVIS:
		score = 28;
		break;
	case R_SUSTSTR:
	case R_SUSTARM:
		score = 32;
		break;
	case R_STEALTH:
		score = 34;
		break;
	case R_NOP:
		score = 5;
		break;
	case R_TELEPORT:
	case R_AGGR:
		score = -150;
		break;
	default:
		score = 0;
		break;
	}

	if (ring->o_flags & ISCURSED)
		score -= 150;

	return score;
}

bool
auto_equip_ring(THING *obj, bool silent)
{
	if (obj == NULL || obj->o_type != RING)
		return FALSE;
	if (obj->o_flags & ISCURSED)
		return FALSE;

	int target_hand = -1;
	const int new_power = ring_power(obj);

	if (cur_ring[LEFT] == NULL && cur_ring[RIGHT] == NULL) {
		if (new_power >= 0)
			target_hand = LEFT;
	}
	else if (cur_ring[LEFT] == NULL) {
		if (new_power >= 0)
			target_hand = LEFT;
	}
	else if (cur_ring[RIGHT] == NULL) {
		if (new_power >= 0)
			target_hand = RIGHT;
	}
	else {
		int left_power = ring_power(cur_ring[LEFT]);
		int right_power = ring_power(cur_ring[RIGHT]);
		int worst_hand = (left_power <= right_power) ? LEFT : RIGHT;
		if (new_power > ring_power(cur_ring[worst_hand])) {
			THING *replaced = cur_ring[worst_hand];
			if (!can_drop(replaced))
				return FALSE;
			target_hand = worst_hand;
		} else {
			return FALSE;
		}
	}

	if (target_hand < 0)
		return FALSE;

	if (cur_ring[target_hand] != NULL) {
		if (!can_drop(cur_ring[target_hand]))
			return FALSE;
	}

	cur_ring[target_hand] = obj;
	apply_ring_effect(obj);
	update_armor_class();
	if (!silent) {
		msg("Auto-equipped %s on %s hand!", inv_name(obj, TRUE),
			target_hand == LEFT ? "left" : "right");
	}
	return TRUE;
}

/*
 * ring_on:
 *	Put a ring on a hand
 */
void
ring_on()
{
	register THING *obj;
	register int ring = -1;

	if ((obj = get_item("put on", RING)) == NULL)
		goto no_ring;
	/*
	 * Make certain that it is somethings that we want to wear
	 */
	if (obj->o_type != RING) {
		msg("you can't put that on your finger");
		goto no_ring;
	}

	/*
	 * find out which hand to put it on
	 */
	if (is_current(obj))
		goto no_ring;

	if (cur_ring[LEFT] == NULL)
		ring = LEFT;
	if (cur_ring[RIGHT] == NULL)
		ring = RIGHT;
	if (cur_ring[LEFT] == NULL && cur_ring[RIGHT] == NULL)
		if ((ring = gethand()) < 0)
			goto no_ring;
	if (ring < 0) {
		msg("you already have a ring on each hand");
		goto no_ring;
	}
	cur_ring[ring] = obj;
	apply_ring_effect(obj);
	update_armor_class();

	msg("%swearing %s (%c)", noterse("you are now "),
		inv_name(obj, TRUE), pack_char(obj));
	return ;

no_ring:
	after = FALSE;
	return;
}

/*
 * ring_off:
 *	Take off a ring
 */
void
ring_off(void)
{
	register int ring;
	register THING *obj;
	register char packchar;

	if (cur_ring[LEFT] == NULL && cur_ring[RIGHT] == NULL) {
		msg("you aren't wearing any rings");
		after = FALSE;
		return;
	} else if (cur_ring[LEFT] == NULL)
		ring = RIGHT;
	else if (cur_ring[RIGHT] == NULL)
		ring = LEFT;
	else
		if ((ring = gethand()) < 0)
			return;
	mpos = 0;
	obj = cur_ring[ring];
	if (obj == NULL) {
		msg("not wearing such a ring");
		after = FALSE;
		return;
	}
	packchar = pack_char(obj);
	if (can_drop(obj))
		msg("was wearing %s(%c)", inv_name(obj, TRUE), packchar);
}

/*
 * gethand:
 *	Which hand is the hero interested in?
 */
static
int
gethand(void)
{
	register int c;

	for (;;) {
		msg("left hand or right hand? ");
		if ((c = readchar()) == ESCAPE)  {
			after = FALSE;
			return -1;
		}
		mpos = 0;
		if (c == 'l' || c == 'L')
			return LEFT;
		else if (c == 'r' || c == 'R')
			return RIGHT;
		msg("please type L or R");
	}
	return -1;
}

/*
 * ring_eat:
 *	How much food does this ring use up?
 */
int
ring_eat(int hand)
{
	if (cur_ring[hand] == NULL)
		return 0;
	switch (cur_ring[hand]->o_which) {
	case R_REGEN:
		return 2;
	case R_SUSTSTR:
	case R_SUSTARM:
	case R_PROTECT:
	case R_ADDSTR:
	case R_STEALTH:
		return 1;
	case R_SEARCH:
		return(rnd(5)==0);
	case R_ADDHIT:
	case R_ADDDAM:
		return (rnd(3) == 0);
	case R_DIGEST:
		return -rnd(2);
	case R_SEEINVIS:
		return (rnd(5) == 0);
	default:
		return 0;
	}
}

/*
 * ring_num:
 *	Print ring bonuses
 */
char *
ring_num(THING *obj)
{
	if (!(obj->o_flags & ISKNOW))
		return "";
	switch (obj->o_which) {
	when R_PROTECT:
	case R_ADDSTR:
	case R_ADDDAM:
	case R_ADDHIT:
		ring_buf[0] = ' ';
		strcpy(&ring_buf[1], num(obj->o_ac, 0, RING));
	otherwise:
		return "";
	}
	return ring_buf;
}
