#ifndef _IDT_H_
#define _IDT_H_

#include "exceptions.h"
#include "interrupt_wrap.h"
#include "x86_desc.h"

#ifndef ASM
    void idt_init();
#endif

#define VECTOR_DEVICE_KEYBOARD 0x21
#define VECTOR_DEVICE_RTC 0x28
#define VECTOR_SYSTEM_CALL 0x80

#endif
