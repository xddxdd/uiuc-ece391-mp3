#include "idt.h"

/* void idt_init()
 * @output: idt[] - IDT table with exception handlers and interrupt handlers
 *                  inserted, ready to be loaded by LIDT
 * @description: generates an IDT table for handling exceptions and interrupts.
 */
void idt_init() {
    int i;
    for(i = 0; i < NUM_VEC; i++) {
        idt[i].present = 0;     // IDT presence will be enabled on SET_IDT_ENTRY call
        idt[i].dpl = (i == VECTOR_SYSTEM_CALL) ? 3 : 0; // system calls in ring3
                                                        // all others in kernel ring0
        idt[i].reserved0 = 0;   // Read ISA Reference Manual for x86, Volume 3,
        idt[i].reserved1 = 1;   // 5.11 IDT Descriptors for why these "magic numbers"
        idt[i].reserved2 = 1;   // are set
        idt[i].reserved3 = (i >= VECTOR_INTERRUPT_START && i <= VECTOR_INTERRUPT_END) ? 0 : 1;
        idt[i].reserved4 = 0;
        idt[i].size = 1;        // All our handlers are 32 bit.
        idt[i].seg_selector = KERNEL_CS;    // All handlers run in kernel space.
    }

    // Read ISA Reference Manual, Vol 3, 5.14 for what these exceptions are.
    SET_IDT_ENTRY(idt[0x00], exception_divide_by_zero);
    SET_IDT_ENTRY(idt[0x01], exception_debug);
    SET_IDT_ENTRY(idt[0x02], exception_nmi_interrupt);
    SET_IDT_ENTRY(idt[0x03], exception_breakpoint);
    SET_IDT_ENTRY(idt[0x04], exception_overflow);
    SET_IDT_ENTRY(idt[0x05], exception_bounds_range_exceeded);
    SET_IDT_ENTRY(idt[0x06], exception_invalid_opcode);
    SET_IDT_ENTRY(idt[0x07], exception_device_not_available);
    SET_IDT_ENTRY(idt[0x08], exception_double_fault);
    SET_IDT_ENTRY(idt[0x09], exception_coprocessor_segment_overrun);
    SET_IDT_ENTRY(idt[0x0A], exception_invalid_tss);
    SET_IDT_ENTRY(idt[0x0B], exception_segment_not_present);
    SET_IDT_ENTRY(idt[0x0C], exception_stack_segment_fault);
    SET_IDT_ENTRY(idt[0x0D], exception_general_protection_fault);
    SET_IDT_ENTRY(idt[0x0E], exception_page_fault);
    // idt[0x0F] is reserved by Intel
    SET_IDT_ENTRY(idt[0x10], exception_x87_fpu_error);
    SET_IDT_ENTRY(idt[0x11], exception_alignment_check);
    SET_IDT_ENTRY(idt[0x12], exception_machine_check);
    SET_IDT_ENTRY(idt[0x13], exception_simd_fpe);
    // idt[0x14 to 0x1F] is reserved by Intel

    // Enable interrupt handling for devices attached to i8259 PIC
    SET_IDT_ENTRY(idt[VECTOR_DEVICE_RTC], interrupt_rtc_wrap);
    SET_IDT_ENTRY(idt[VECTOR_DEVICE_KEYBOARD], interrupt_keyboard_wrap);
    SET_IDT_ENTRY(idt[VECTOR_DEVICE_SERIAL1], interrupt_serial1_wrap);
    SET_IDT_ENTRY(idt[VECTOR_DEVICE_SERIAL2], interrupt_serial2_wrap);
    SET_IDT_ENTRY(idt[VECTOR_DEVICE_SB16], interrupt_sb16_wrap);
    SET_IDT_ENTRY(idt[VECTOR_DEVICE_PIT], interrupt_pit_wrap);
    SET_IDT_ENTRY(idt[VECTOR_DEVICE_MOUSE], interrupt_mouse_wrap);

    // system call
    SET_IDT_ENTRY(idt[VECTOR_SYSTEM_CALL], syscall_wrap);
}
