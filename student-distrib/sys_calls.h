#ifndef _SYS_CALLS_H
#define _SYS_CALLS_H

#include "fs/ece391fs.h"
#include "x86_desc.h"
#include "paging.h"
#include "lib.h"
#include "devices/rtc.h"

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
// minor functions for execute()
void _execute_parse (const uint8_t* command, uint8_t* filename);
int32_t _execute_exe_check (int32_t* fd);
void _execute_paging ();
int32_t _execute_pgm_loader (int32_t* fd);
void _execute_context_switch ();

#endif
