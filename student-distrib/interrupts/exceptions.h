#ifndef _EXCEPTIONS_H_
#define _EXCEPTIONS_H_

#ifndef ASM
    #include "../lib/lib.h"

    void exception_handler_real(char* message);

    #define EXCEPTION_HEADER(name, message) \
        void name();

    #define EXCEPTION_HANDLER(name, message) \
        void name() { exception_handler_real(message); }

    EXCEPTION_HEADER(exception_divide_by_zero, "Divide by 0");
    EXCEPTION_HEADER(exception_debug, "Debug");
    EXCEPTION_HEADER(exception_nmi_interrupt, "NMI Interrupt");
    EXCEPTION_HEADER(exception_breakpoint, "Breakpoint reached");
    EXCEPTION_HEADER(exception_overflow, "Overflow");
    EXCEPTION_HEADER(exception_bounds_range_exceeded, "Bounds range exceeded");
    EXCEPTION_HEADER(exception_invalid_opcode, "Invalid Opcode");
    EXCEPTION_HEADER(exception_device_not_available, "Device unavailable");
    EXCEPTION_HEADER(exception_double_fault, "Double fault");
    EXCEPTION_HEADER(exception_coprocessor_segment_overrun, "Coprocessor segment overrun");
    EXCEPTION_HEADER(exception_invalid_tss, "Invalid TSS");
    EXCEPTION_HEADER(exception_segment_not_present, "Segment not present");
    EXCEPTION_HEADER(exception_stack_segment_fault, "Stack segment fault");
    EXCEPTION_HEADER(exception_general_protection_fault, "General protection fault");
    EXCEPTION_HEADER(exception_page_fault, "Page fault");
    EXCEPTION_HEADER(exception_x87_fpu_error, "X87 FPU Error");
    EXCEPTION_HEADER(exception_alignment_check, "Alignment check");
    EXCEPTION_HEADER(exception_machine_check, "Machine check");
    EXCEPTION_HEADER(exception_simd_fpe, "SIMD Floating Point Exception");
#endif

#endif
