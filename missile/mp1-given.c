
/* mp1-given.c
 * Code I wrote so that the students don't have to. In the kernel, these
 * reside in rtc.c
 * Mark Murphy 2007
 */

#include "mp1.h"

/* These are defined in mp1.c */
extern char base_alive[3];
extern struct missile* mp1_missile_list;
extern int mp1_score;

/* Check if an explosion at (x,y) touches any of the three bases */
static int explode_base(int x, int y){
	int bases_killed = 0;
	if(y >= 23){
		if(17 <= x && x <= 23){
			if(base_alive[0])
				bases_killed++;
			base_alive[0] = 0;
		}else
		if(37 <= x && x <= 43){
			if(base_alive[1])
				bases_killed++;
			base_alive[1] = 0;
		}else
		if(57 <= x && x <= 63){
			if(base_alive[2])
				bases_killed++;
			base_alive[2] = 0;
		}
	}
	return bases_killed;
}

/* missile_explode()
 * Check if an exploding missile has side-effects - killing other missiles,
 * killing bases, etc.
 * Arguments : struct missile *m - the missile which exploded
 * Returns   : zero if no game state was changed, nonzero if game state
 *             (other missiles or bases) has changed
 */
int missile_explode(struct missile *m){
	struct missile *i = mp1_missile_list;
	int exploded = 0;
	if(!m->exploded){
		m->exploded = 50;
	}
	if(m->c == 'e'){
		exploded += explode_base(m->x>>16, m->y>>16);
	}

	while(i){
		if(i != m){
			int dx, dy;
			dx = (m->x>>16) - (i->x>>16);
			dy = (m->y>>16) - (i->y>>16);
			if(dx >= -2 && dx <= 2 && dy >= -1 && dy <= 1 &&
			   i->exploded == 0 && i->c == 'e' && m->c == '*'){
				mp1_score++;
				exploded++;	
				i->exploded = 50;
			}
		}
		i = i->next;
	}
	return exploded;
}
