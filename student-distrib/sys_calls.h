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
#define SPACE               ' '
#define FILE_HEADER_LEN     40
#define FILE_EXE_HEADER_0   0x7F
#define FILE_EXE_HEADER_1   0x45
#define FILE_EXE_HEADER_2   0x4C
#define FILE_EXE_HEADER_3   0x46

#define USER_PROCESS        0x08048000
#define TD_ADDR_OFFSET      22

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
int32_t _execute_exe_check (const uint8_t* filename);
int32_t _execute_paging (const uint8_t* filename);

#endif
