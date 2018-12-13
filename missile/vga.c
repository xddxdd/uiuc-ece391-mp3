
/* vga.c
 * A small set of functions to interface (from userspace) to the VGA in
 * text-mode.
 * Mark Murphy 2007
 */

#include <stdint.h>
#include "ece391support.h"
#include "ece391syscall.h"

void mp1_poke_helper(int pos, int data) {
	ece391_poke((pos / 2) % 80, (pos / 2) / 80, data);
}

void clear_screen(){
	int x, y;
	for(x = 0; x < 80; x++){
		for(y = 0; y < 25; y++) {
			ece391_poke(x, y, 0x720);
		}
	}
}

void write_char(char c, int x, int y){
	ece391_poke(x, y, (uint32_t) (0x700 | c));
}

void write_string(char *s, int x, int y){
	while(*s){
		ece391_poke(x, y, (uint32_t) (0x700 | (*s++)));
		x++;
		if(x >= 80) { x = 0; y++; }
	}
}
