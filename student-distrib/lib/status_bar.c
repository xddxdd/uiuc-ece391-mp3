#include "status_bar.h"
#include "../devices/cmos.h"
#include "../devices/qemu_vga.h"
#include "../interrupts/multiprocessing.h"

/* void status_bar_switch_terminal(uint8_t tid)
 * @input: tid - new terminal
 * @output: status bar refreshed on new terminal
 * @description: refreshes status message and time on new terminal
 */
void status_bar_switch_terminal(uint8_t tid) {
    if(tid > TERMINAL_COUNT) return;
    char s[] = "Switched to terminal #0";
    s[strlen(s) - 1] = '0' + tid;
    status_bar_update_message(s, strlen(s), ATTR_YELLOW_ON_BLACK);
    status_bar_update_clock();
}

/* void status_bar_update_message(char* msg, uint32_t len, uint8_t attr)
 * @input: msg, len - data and length of message for status bar
 * @output: attr - attribute
 * @description: display a message on status bar
 */
void status_bar_update_message(char* msg, uint32_t len, uint8_t attr) {
    if(NULL == msg) return;
    int i;
    for(i = 0; i < len && i < STATUS_BAR_X_MSG_END; i++) {
        qemu_vga_putc((STATUS_BAR_X_MSG_START + i) * FONT_ACTUAL_WIDTH,
            (STATUS_BAR_Y_END - 1) * FONT_ACTUAL_HEIGHT,
            msg[i], qemu_vga_get_terminal_color(attr),
            qemu_vga_get_terminal_color(attr >> 4));
    }
    for(i = len; i < STATUS_BAR_X_MSG_END; i++) {
        qemu_vga_putc((STATUS_BAR_X_MSG_START + i) * FONT_ACTUAL_WIDTH,
            (STATUS_BAR_Y_END - 1) * FONT_ACTUAL_HEIGHT,
            ' ', qemu_vga_get_terminal_color(attr),
            qemu_vga_get_terminal_color(attr >> 4));
    }
}

/* void status_bar_update_clock()
 * @output: clock on right bottom corner of screen gets updated
 * @description: updates the clock with CMOS time
 */
char prev_time[STATUS_BAR_X_TIME_END - STATUS_BAR_X_TIME_START] = {0};
void status_bar_update_clock() {
    char time[STATUS_BAR_X_TIME_END - STATUS_BAR_X_TIME_START] = "0000-00-00 00:00:00 ";
    uint32_t i = 0;
    cmos_read(NULL, &i, time, STATUS_BAR_X_TIME_END - STATUS_BAR_X_TIME_START);
    time[STATUS_BAR_X_TIME_END - STATUS_BAR_X_TIME_START - 1] = ' ';
    int active_tid = active_terminal_id;
    for(i = 0; i < STATUS_BAR_X_TIME_END - STATUS_BAR_X_TIME_START; i++) {
        if(time[i] == prev_time[i]) continue;
        for(active_terminal_id = 0; active_terminal_id < TERMINAL_COUNT; active_terminal_id++) {
            qemu_vga_putc((STATUS_BAR_X_TIME_START + i) * FONT_ACTUAL_WIDTH,
                (STATUS_BAR_Y_END - 1) * FONT_ACTUAL_HEIGHT,
                time[i], qemu_vga_get_terminal_color(ATTR_WHITE_ON_BLUE),
                qemu_vga_get_terminal_color(ATTR_WHITE_ON_BLUE >> 4));
        }
    }
    active_terminal_id = active_tid;
    memcpy(prev_time, time, STATUS_BAR_X_TIME_END - STATUS_BAR_X_TIME_START);
}
