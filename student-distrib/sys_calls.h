#include "fs/ece391fs.h"
#include "x86_desc.h"
#include "lib.h"
#include "devices/rtc.h"

#define SYSCALL_SUCCESS 1
#define SYSCALL_FAIL    0

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
