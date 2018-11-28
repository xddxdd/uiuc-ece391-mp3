#ifndef _EXCEPTIONS_H_
#define _EXCEPTIONS_H_

#ifndef ASM
    #include "../lib/lib.h"
    #include "sys_calls.h"

    typedef struct {
        uint32_t eip;
        uint32_t cs;
        uint32_t eflags;
        uint32_t esp;
        uint32_t ss;
    } iret_t;

    typedef struct {
        uint32_t edi;
        uint32_t esi;
        uint32_t ebp;
        uint32_t esp;
        uint32_t ebx;
        uint32_t edx;
        uint32_t ecx;
        uint32_t eax;
    } pushal_t;

    void exception_handler_real(uint32_t id, pushal_t pushal, uint32_t err_code, iret_t iret);

#endif

#define EXCEPTION_INVALID_TSS 0x0a
#define EXCEPTION_SEGMENT_NOT_PRESENT 0x0b
#define EXCEPTION_STACK_SEGMENT_FAULT 0x0c
#define EXCEPTION_GENERAL_PROTECTION_FAULT 0x0d
#define EXCEPTION_PAGE_FAULT 0x0e

#endif
