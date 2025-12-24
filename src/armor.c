/*
 * This file contains misc functions for dealing with armor
 * @(#)armor.c		1.2 (AI Design)		2/12/84
 *
 */

#include "rogue.h"
#include "curses.h"

void waste_time(void);
void update_armor_class(void);

/*
 * waste_time:
 *	Do nothing but let other things happen
 */
void
waste_time()
{
	do_daemons();
	do_fuses();
}

/*
 * update_armor_class:
 *	Recalculate the player's armor class based on equipped items
 */
void
update_armor_class()
{
	int ac = pstats.s_arm; // base 10
	if (cur_armor) ac = min(ac, cur_armor->o_ac);
	if (cur_helmet) ac = min(ac, cur_helmet->o_ac);
	if (cur_gloves) ac = min(ac, cur_gloves->o_ac);
	if (cur_boots) ac = min(ac, cur_boots->o_ac);
	if (cur_shield) ac = min(ac, cur_shield->o_ac);
	if (ISRING(LEFT, R_PROTECT)) ac -= cur_ring[LEFT]->o_ac;
	if (ISRING(RIGHT, R_PROTECT)) ac -= cur_ring[RIGHT]->o_ac;
	pstats.s_arm = ac;
}

/*
 * wear:
 *	The player wants to wear something, so let him/her put it on.
 */
void
wear()
{
	register THING *obj;
	register char *sp;

	if ((obj = get_item("wear", 0)) == NULL)  // 0 means any type
		return;
	switch (obj->o_type) {
	case ARMOR:
		if (cur_armor != NULL) {
			msg("you are already wearing some armor.");
			after = FALSE;
			return;
		}
		break;
	case HELMET:
		if (cur_helmet != NULL) {
			msg("you are already wearing a helmet.");
			after = FALSE;
			return;
		}
		break;
	case GLOVES:
		if (cur_gloves != NULL) {
			msg("you are already wearing gloves.");
			after = FALSE;
			return;
		}
		break;
	case BOOTS:
		if (cur_boots != NULL) {
			msg("you are already wearing boots.");
			after = FALSE;
			return;
		}
		break;
	case SHIELD:
		if (cur_shield != NULL) {
			msg("you are already wearing a shield.");
			after = FALSE;
			return;
		}
		break;
	default:
		msg("you can't wear that");
		return;
	}
	waste_time();
	obj->o_flags |= ISKNOW ;
	sp = inv_name(obj, TRUE);
	switch (obj->o_type) {
	case ARMOR: cur_armor = obj; break;
	case HELMET: cur_helmet = obj; break;
	case GLOVES: cur_gloves = obj; break;
	case BOOTS: cur_boots = obj; break;
	case SHIELD: cur_shield = obj; break;
	}
	update_armor_class();
	msg("you are now wearing %s", sp);
}

/*
 * take_off:
 *	Get the armor off of the player's back
 */
void
take_off()
{
	register THING *obj;

	if ((obj = cur_armor) == NULL) {
		after = FALSE;
		msg("you aren't wearing any armor");
		return;
	}
	if (!can_drop(cur_armor))
		return;
	cur_armor = NULL;
	msg("you used to be wearing %c) %s", pack_char(obj), inv_name(obj, TRUE));
}

/*
 * unequip:
 *	Take off an equipped item
 */
void
unequip()
{
	msg("Unequip which item? (w)eapon, (a)rmor, (h)elmet, (g)loves, (b)oots, (s)hield, (r)ing");
	byte ch = readchar();
	THING *obj = NULL;
	switch (ch) {
	case 'w': obj = cur_weapon; break;
	case 'a': obj = cur_armor; break;
	case 'h': obj = cur_helmet; break;
	case 'g': obj = cur_gloves; break;
	case 'b': obj = cur_boots; break;
	case 's': obj = cur_shield; break;
	case 'r':
		if (cur_ring[LEFT] && cur_ring[RIGHT]) {
			msg("Left or right ring? (l/r)");
			byte hand = readchar();
			if (hand == 'l') obj = cur_ring[LEFT];
			else if (hand == 'r') obj = cur_ring[RIGHT];
			else { msg("Invalid"); after = FALSE; return; }
		} else if (cur_ring[LEFT]) obj = cur_ring[LEFT];
		else if (cur_ring[RIGHT]) obj = cur_ring[RIGHT];
		else { msg("No ring equipped"); after = FALSE; return; }
		break;
	default: msg("Invalid choice"); after = FALSE; return;
	}
	if (obj == NULL) {
		msg("Not equipped");
		after = FALSE;
		return;
	}
	if (!can_drop(obj)) return;
	// Now remove
	switch (ch) {
	case 'w': cur_weapon = NULL; break;
	case 'a': cur_armor = NULL; break;
	case 'h': cur_helmet = NULL; break;
	case 'g': cur_gloves = NULL; break;
	case 'b': cur_boots = NULL; break;
	case 's': cur_shield = NULL; break;
	case 'r':
		if (obj == cur_ring[LEFT]) cur_ring[LEFT] = NULL;
		else cur_ring[RIGHT] = NULL;
		break;
	}
	update_armor_class();
	msg("you removed %s", inv_name(obj, TRUE));
}
