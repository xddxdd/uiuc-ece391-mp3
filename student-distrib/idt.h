#ifndef _IDT_H_
#define _IDT_H_

#include "x86_desc.h"
#include "exceptions.h"

#ifndef ASM
    void idt_init();
#endif

#endif
