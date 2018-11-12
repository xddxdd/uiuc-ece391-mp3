#ifndef _SYS_CALL_H
#define _SYS_CALL_H

#include "fs/unified_fs.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "fs/ece391fs.h"
#include "devices/rtc.h"
#include "devices/keyboard.h"

#define SYSCALL_SUCCESS 1
#define SYSCALL_FAIL    0

// some contants
#define SPACE                   ' '
#define FILE_HEADER_LEN         40                     // 40 Bytes
#define FILE_EXE_HEADER_0       0x7F
#define FILE_EXE_HEADER_1       0x45
#define FILE_EXE_HEADER_2       0x4C
#define FILE_EXE_HEADER_3       0x46
#define USER_PROCESS_ADDR       0x08048000
#define USER_STACK_ADDR         0x08400000 - 0x4
#define USER_PAGE_SIZE          0x400000               // 4 MB
#define KERNEL_STACK_BASE_ADDR  0x800000               // 8 MB
#define USER_KMODE_STACK_SIZE   0x2000                 // 8 kB
#define PD_ADDR_OFFSET          22
#define PROCESS_PYSC_BASE_ADDR  2                      // 8 MB
#define MAX_NUM_FD_ENTRY        8                      // Up to 8 open files per task
#define FILE_OP_NUM             4
#define STDIN_ENTRY             0
#define STDOUT_ENTRY            1
#define FD_ENTRY_ASSIGNED       1
#define FD_ENTRY_NOT_ASSIGNED   0
#define NOT_ASSIGNED            -1
#define KER_STACK_BITMASK       0xFFFFE000             // Use the higher 19 bits
                                                       // to get top of 8KB kernel stack

// Entry of the file array in process control block.
// typedef struct file_descriptor_entry {
//     int32_t (*fileOpTable_ptr[FILE_OP_NUM])();         // pointer to file operations table
//     int32_t inode;
//     int32_t file_position;
//     int32_t flags;
// } fd_entry_t;

typedef struct process_control_block {
    fd_array_t fd_array[MAX_NUM_FD_ENTRY];
    uint32_t current_pid;                                // current process id;
    uint32_t parent_pid;                                // parent process id;
    struct process_control_block * parent_pcb;
    uint32_t esp;                                       // save esp;
    uint32_t ebp;                                       // save ebp;
    // more entries to be added......
} pcb_t;

// System calls for checkpoint 3.
int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);

// System calls for checkpoint 4.
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);

//Extra credit system calls.
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);

// Helper functions for sytstem calls
pcb_t* get_pcb_ptr();
pcb_t* pcb_init(int32_t pid);

// minor functions for execute()
void _execute_parse (const uint8_t* command, uint8_t* filename);
int32_t _execute_exe_check (fd_array_t* fd_array, int32_t fd);
void _execute_paging ();
int32_t _execute_pgm_loader (fd_array_t* fd_array, int32_t fd);
void _execute_context_switch ();
void _halt_paging (int32_t pid);

#endif
