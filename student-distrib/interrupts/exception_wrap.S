#define ASM 1

// Exception wrapper for exceptions without error code
// Push an pseudo error code onto stack
#define EXCEPTION_WRAP(name, id)     \
    .globl name                     ;\
    name:                           ;\
        pushl $0                    ;\
        pushal                      ;\
        pushl $id                   ;\
        call exception_handler_real ;\
        addl $4, %esp               ;\
        popal                       ;\
        addl $4, %esp               ;\
        iret

// Exception wrapper for exceptions without error code
#define EXCEPTION_WRAP_ERR(name, id) \
    .globl name                     ;\
    name:                           ;\
        pushal                      ;\
        pushl $id                   ;\
        call exception_handler_real ;\
        addl $4, %esp               ;\
        popal                       ;\
        addl $4, %esp               ;\
        iret

// Handlers for exceptions, simple prints out a string.
// Read ISA Reference Manual, Vol 3, 5.14 for what these exceptions are.
EXCEPTION_WRAP    (exception_divide_by_zero,               0x00);
EXCEPTION_WRAP    (exception_debug,                        0x01);
EXCEPTION_WRAP    (exception_nmi_interrupt,                0x02);
EXCEPTION_WRAP    (exception_breakpoint,                   0x03);
EXCEPTION_WRAP    (exception_overflow,                     0x04);
EXCEPTION_WRAP    (exception_bounds_range_exceeded,        0x05);
EXCEPTION_WRAP    (exception_invalid_opcode,               0x06);
EXCEPTION_WRAP    (exception_device_not_available,         0x07);
EXCEPTION_WRAP_ERR(exception_double_fault,                 0x08);
EXCEPTION_WRAP    (exception_coprocessor_segment_overrun,  0x09);
EXCEPTION_WRAP_ERR(exception_invalid_tss,                  0x0a);
EXCEPTION_WRAP_ERR(exception_segment_not_present,          0x0b);
EXCEPTION_WRAP_ERR(exception_stack_segment_fault,          0x0c);
EXCEPTION_WRAP_ERR(exception_general_protection_fault,     0x0d);
EXCEPTION_WRAP_ERR(exception_page_fault,                   0x0e);
EXCEPTION_WRAP    (exception_x87_fpu_error,                0x10);
EXCEPTION_WRAP_ERR(exception_alignment_check,              0x11);
EXCEPTION_WRAP    (exception_machine_check,                0x12);
EXCEPTION_WRAP    (exception_simd_fpe,                     0x13);
