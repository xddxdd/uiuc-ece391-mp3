#ifndef _IDT_H_
#define _IDT_H_

#include "exception_wrap.h"
#include "interrupt_wrap.h"
#include "../x86_desc.h"
#include "syscall_wrap.h"

#ifndef ASM
    void idt_init();
#endif

// As designed in x86 PC
#define VECTOR_DEVICE_KEYBOARD 0x21
#define VECTOR_DEVICE_RTC 0x28
#define VECTOR_DEVICE_SERIAL1 0x24
#define VECTOR_DEVICE_SERIAL2 0x23
#define VECTOR_DEVICE_SB16 0x25
#define VECTOR_DEVICE_PIT 0x20

#define VECTOR_INTERRUPT_START 0x20
#define VECTOR_INTERRUPT_END 0x2F

// As used in this MP
#define VECTOR_SYSTEM_CALL 0x80

#endif
