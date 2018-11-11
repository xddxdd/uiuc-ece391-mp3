#ifndef _SYSCALL_WRAP_H_
#define _SYSCALL_WRAP_H_

#include "sys_calls.h"

#ifndef ASM
    extern void syscall_wrap();
#endif

#endif
