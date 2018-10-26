#ifndef _IDT_H_
#define _IDT_H_

#include "exceptions.h"
#include "interrupt_wrap.h"
#include "x86_desc.h"

#ifndef ASM
    void idt_init();
#endif

// As designed in x86 PC
#define VECTOR_DEVICE_KEYBOARD 0x21
#define VECTOR_DEVICE_RTC 0x28
#define VECTOR_DEVICE_SERIAL1 0x24
#define VECTOR_DEVICE_SERIAL2 0x23

// As used in this MP
#define VECTOR_SYSTEM_CALL 0x80

#endif
