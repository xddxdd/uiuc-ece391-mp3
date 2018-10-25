#include "exceptions.h"
#include "devices/vga_text.h"
#include "data/aqua.h"

// Handlers for exceptions, simple prints out a string.
// Read ISA Reference Manual, Vol 3, 5.14 for what these exceptions are.
EXCEPTION_HANDLER(exception_divide_by_zero, "Divide by 0");
EXCEPTION_HANDLER(exception_debug, "Debug");
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

/* void exception_handler_real(char* message)
 * @input: message - reason of this exception
 * @output: - a picture of anime character Aqua, on bottom right of screen
 *          - a big 00P5
 *          - reason of this exception
 * @effects: - interrupts are disabled
 *           - system put into infinite loop
 * @description: the function to handle all the different exceptions.
 */
void exception_handler_real(char* message) {
    int x, y;

    cli();  // Disable interruption

    // Draw aqua on the bottom right of the screen
    for(y = 0; y < AQUA_HEIGHT; y++) {
        for(x = 0; x < AQUA_WIDTH; x++) {
            // Loop through every pixel of Aqua
            if(aqua[x * AQUA_HEIGHT + y] == 0) {    // 0 references BLACK color in VGA text
                vga_text_set_color(
                    x + SCREEN_WIDTH - AQUA_WIDTH,  // Bottom right of the screen
                    y + SCREEN_HEIGHT - AQUA_HEIGHT,
                7, 0   // 0 references BLACK, 15 references WHITE
                            // make white text on black background
                );
            } else {
                vga_text_set_color(
                    x + SCREEN_WIDTH - AQUA_WIDTH,  // Bottom right of the screen
                    y + SCREEN_HEIGHT - AQUA_HEIGHT,
                    aqua[x * AQUA_HEIGHT + y],  // Set foreground and background color
                    aqua[x * AQUA_HEIGHT + y]   // to be the same
                );
            }
        }
    }

    // Print the big 00P5 and exception message
    printf("+---+ +---+ +---+ +---+\n");
    printf("|   | |   | |   | |    \n");
    printf("| . | | . | +---+ +---+\n");
    printf("|   | |   | |         |\n");
    printf("+---+ +---+ +     +---+\n");
    printf("The following exception happened:\n");
    printf(message);

    // Infinite loop
    asm volatile (".1: hlt; jmp .1;");
}
