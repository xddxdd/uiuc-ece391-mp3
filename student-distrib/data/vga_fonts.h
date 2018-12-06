#ifndef _VGA_FONTS_H_
#define _VGA_FONTS_H_

#include "../lib/types.h"

#define FONT_WIDTH 12
#define FONT_HEIGHT 24

extern uint8_t font_data[256][FONT_HEIGHT * FONT_WIDTH];

#endif
