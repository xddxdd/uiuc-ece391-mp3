#ifndef _MULTIPROCESSING_H_
#define _MULTIPROCESSING_H_

#include "../paging.h"
#include "../x86_desc.h"
#include "../fs/ece391fs.h"
#include "../lib/lib.h"
#include "../devices/vga_text.h"
#include "../fs/unified_fs.h"
#include "../devices/keyboard.h"
#include "../devices/qemu_vga.h"
#include "../lib/chinese_input.h"

#define STRING_END              '\0'
#define SPACE                   ' '
#define USER_PROCESS_ADDR       0x08048000
#define USER_STACK_ADDR         (0x08400000 - 0x4)
#define USER_PAGE_SIZE          0x400000               // 4 MB
#define PD_ADDR_OFFSET          22
#define PROCESS_PYSC_BASE_ADDR  2                      // 8-12 MB
#define MAX_NUM_FD_ENTRY        8                      // Up to 8 open files per task
 // Use the higher 19 bits to get top of 8KB kernel stack
#define KER_STACK_BITMASK       0xFFFFE000
#define MAX_ARG_LENGTH          128
#define MAX_PROGRAMS_NUM        8

#define KERNEL_STACK_BASE_ADDR  0x800000               // 8 MB
#define USER_KMODE_STACK_SIZE   0x2000                 // 8 kB

#define PROGRAM_MAX_LEN 0x300000                       // 3MB
#define PROGRAM_HEADER_LEN 4
#define PROGRAM_HEADER_OFFSET 36
extern char program_header[PROGRAM_HEADER_LEN];

typedef struct process_control_block {
    fd_array_t fd_array[MAX_NUM_FD_ENTRY];
    uint8_t present;                        // whether this process is present
    int32_t parent_pid;                     // parent process id;
    uint32_t esp;                           // save esp;
    uint32_t ebp;                           // save ebp;
    uint32_t eip;                           // save eip;
    uint32_t terminal;                      // terminal id
    uint32_t vidmap;                        // is vidmap enabled
    char cmd[MAX_ARG_LENGTH + 1];           // process executable name
    char arg[MAX_ARG_LENGTH + 1];           // argument to process
} process_t;

typedef process_t pcb_t;

typedef struct {
    int32_t active_process;
    int screen_x;
    int screen_y;
    uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE + 1];
    int keyboard_buffer_top;
    volatile int keyboard_buffer_enable;
    utf8_state_t utf8_state;                        // UTF-8 character state
    chinese_input_buf_t chinese_input_buf;          // Chinese IME state
    uint8_t welcome_shown;                          // Has shown logo on this terminal
} terminal_t;

#define TERMINAL_COUNT 3
#define PROCESS_COUNT 8

#define TERMINAL_DIRECT_ADDR 0xb7000
#define TERMINAL_ALT_START 0xb9000
#define TERMINAL_ALT_SIZE 0x1000

// Wrapper for interrupts, etc, to force operation
// onto displayed terminal instead of active terminal
#define ONTO_DISPLAY_WRAP(code) {               \
    video_mem = (char*) TERMINAL_DIRECT_ADDR;   \
    int active_tid = active_terminal_id;        \
    active_terminal_id = displayed_terminal_id; \
    code;                                       \
    active_terminal_id = active_tid;            \
    video_mem = (char*) VIDEO;                  \
}

extern volatile terminal_t terminals[TERMINAL_COUNT];
extern int32_t displayed_terminal_id;
extern int32_t active_terminal_id;
extern int32_t active_process_id;

process_t* process_get_active_pcb();
process_t* process_get_pcb(int32_t pid);
void process_init();
int32_t process_allocate();
int32_t process_create(const char* command);
int32_t process_halt(uint8_t status);
void process_switch_paging(int32_t pid);
void process_switch_context(int32_t pid);

void terminal_switch_active(uint32_t tid);
void terminal_switch_display(uint32_t tid);

void executable_patching(const char* process);

#endif
