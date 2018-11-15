#include "exceptions.h"
#include "../devices/vga_text.h"
// #include "data/aqua.h"

char *exceptions[20] = {
    "Division by 0",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating Point Exception"
};

/* void exception_handler_real(char* message)
 * @input: message - reason of this exception
 * @output: - a picture of anime character Aqua, on bottom right of screen
 *          - a big 00P5
 *          - reason of this exception
 * @effects: - interrupts are disabled
 *           - system put into infinite loop
 * @description: the function to handle all the different exceptions.
 */
void exception_handler_real(uint32_t id, pushal_t pushal, uint32_t err_code, iret_t iret) {
    cli();  // Disable interruption

    if(id == ERR_PAGE_FAULT) {
        uint32_t page_fault_pos;
        asm volatile (
            "movl %%cr2, %0"
            :"=r" (page_fault_pos)
        );
    }

    // Print the big 00P5 and exception message
    /*
    printf("+---+ +---+ +---+ +---+\n");
    printf("|   | |   | |   | |    \n");
    printf("| . | | . | +---+ +---+\n");
    printf("|   | |   | |         |\n");
    printf("+---+ +---+ +     +---+\n");
    */
    printf("The following exception happened:\n");
    printf(exceptions[id]);

    /*
    // Draw aqua on the bottom right of the screen
    // int x, y;
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
    */

    // Infinite loop
    asm volatile (".1: hlt; jmp .1;");
    sti();
}
