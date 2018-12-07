#ifndef _STATUS_BAR_H_
#define _STATUS_BAR_H_

#include "lib.h"

// Screen Y resolution is 480, 16 pixels per row,
// 30 rows in total, upper 25 rows occupied by shell
#define STATUS_BAR_Y_START 25
#define STATUS_BAR_Y_END 30

#define STATUS_BAR_X_MSG_START 0
#define STATUS_BAR_X_MSG_END 60
#define STATUS_BAR_X_TIME_START 60
#define STATUS_BAR_X_TIME_END 80

#define ATTR_YELLOW_ON_BLACK 0x0e
#define ATTR_WHITE_ON_BLUE 0x1f

void status_bar_switch_terminal(uint8_t tid);
void status_bar_update_message(char* msg, uint32_t len, uint8_t attr);
void status_bar_update_clock();

#endif
