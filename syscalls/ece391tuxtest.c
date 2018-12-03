#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define FAIL -1
#define PASS 0

int main ()
{
	int32_t fd;
	if(FAIL == (fd = ece391_open("tux"))) return FAIL;

	// List of buttons, see function interface of tux_read()
	char buttons[8][6] = {"START", "A", "B", "C", "Up", "Down", "Left", "Right"};

	uint8_t prev_buttons = 0;
	uint8_t curr_buttons = 0;
	uint8_t quit_count = 0;
	ece391_fdputs(1, (uint8_t*) "Press START 3 times to quit\n");
	while(1) {
		if(FAIL == ece391_read(fd, &curr_buttons, sizeof(uint8_t))) return FAIL;
		int i;
		for(i = 0; i < 8; i++) {
			if(!(prev_buttons & (1 << i)) && (curr_buttons & (1 << i))) {
				// A button has been pressed
				ece391_fdputs(1, (uint8_t*) "Press ");
				ece391_fdputs(1, (uint8_t*) buttons[i]);
				ece391_fdputs(1, (uint8_t*) "\n");
                // Print onto tux screen
                int len = ece391_strlen(buttons[i]);
                if(len > 4) len = 4;
                if(FAIL == ece391_write(fd, buttons[i], len)) return FAIL;
				// Add counter for quit test every time START is pressed
				if(i == 0) {
					quit_count++;
				} else {
					quit_count = 0;
				}
			} else if((prev_buttons & (1 << i)) && !(curr_buttons & (1 << i))) {
				// A button has been released
				ece391_fdputs(1, (uint8_t*) "Release ");
				ece391_fdputs(1, (uint8_t*) buttons[i]);
				ece391_fdputs(1, (uint8_t*) "\n");
                // Print onto tux screen
                if(FAIL == ece391_write(fd, "    ", 4)) return FAIL;
			}
		}
		prev_buttons = curr_buttons;

		// If START pressed more than 3 times in a row, quit
		if(quit_count >= 3) break;
	}

    if(FAIL == ece391_write(fd, (uint8_t*) "bye", 3)) return FAIL;
	if(FAIL == ece391_close(fd)) return FAIL;
    return PASS;
}
