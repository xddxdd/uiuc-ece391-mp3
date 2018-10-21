#include "exceptions.h"

EXCEPTION_HANDLER(exception_divide_by_zero, "Divide by 0");
EXCEPTION_HANDLER(exception_nmi_interrupt, "NMI Interrupt");
EXCEPTION_HANDLER(exception_breakpoint, "Breakpoint reached");
EXCEPTION_HANDLER(exception_overflow, "Overflow");
EXCEPTION_HANDLER(exception_bounds_range_exceeded, "Bounds range exceeded");
EXCEPTION_HANDLER(exception_invalid_opcode, "Invalid Opcode");
EXCEPTION_HANDLER(exception_device_not_available, "Device unavailable");
EXCEPTION_HANDLER(exception_double_fault, "Double fault");
EXCEPTION_HANDLER(exception_coprocessor_segment_overrun, "Coprocessor segment overrun");
EXCEPTION_HANDLER(exception_invalid_tss, "Invalid TSS");
EXCEPTION_HANDLER(exception_segment_not_present, "Segment not present");
EXCEPTION_HANDLER(exception_stack_segment_fault, "Stack segment fault");
EXCEPTION_HANDLER(exception_general_protection_fault, "General protection fault");
EXCEPTION_HANDLER(exception_page_fault, "Page fault");
EXCEPTION_HANDLER(exception_x87_fpu_error, "X87 FPU Error");
EXCEPTION_HANDLER(exception_alignment_check, "Alignment check");
EXCEPTION_HANDLER(exception_machine_check, "Machine check");
EXCEPTION_HANDLER(exception_simd_fpe, "SIMD Floating Point Exception");
