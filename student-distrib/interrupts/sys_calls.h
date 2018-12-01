#ifndef _SYS_CALL_H
#define _SYS_CALL_H

#include "../lib/lib.h"

// 128 MB + 4 MB + 0xB8000 (VIDEO)
#define USER_VIDEO              (33 * 0x400000 + 0xb8000)

// System calls for checkpoint 3.
int32_t syscall_halt (uint8_t status);
int32_t syscall_execute (const uint8_t* command);
int32_t syscall_read (int32_t fd, void* buf, int32_t nbytes);
int32_t syscall_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t syscall_ioctl (int32_t fd, int32_t op);
int32_t syscall_open (const uint8_t* filename);
int32_t syscall_close (int32_t fd);

// System calls for checkpoint 4.
int32_t syscall_getargs (uint8_t* buf, int32_t nbytes);
int32_t syscall_vidmap (uint8_t** screen_start);

//Extra credit system calls.
int32_t syscall_set_handler (int32_t signum, void* handler_address);
int32_t syscall_sigreturn (void);
int32_t syscall_shutdown(void);
int32_t syscall_reboot(void);

#endif
