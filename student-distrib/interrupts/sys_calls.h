#ifndef _SYS_CALL_H
#define _SYS_CALL_H

#include "../fs/unified_fs.h"
#include "../lib/lib.h"
#include "multiprocessing.h"

// 128 MB + 4 MB + 0xB8000 (VIDEO)
#define USER_VIDEO              (33 * 0x400000 + 0xb8000)

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

#endif
