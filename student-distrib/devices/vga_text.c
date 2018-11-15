#include "vga_text.h"
#include "qemu_vga.h"
#include "../data/vga_fonts.h"
#include "../lib/lib.h"

/* void vga_text_set_color(uint8_t x, uint8_t y, uint8_t foreground, uint8_t background)
 * @input: x, y - coordinate of character whose color to be changed.
 *         foreground - foreground color to be changed, using VGA text mode palette.
 *         background - background color to be changed, using VGA text mode palette.
 * @output: the character at (x, y) has color changed to *foreground* and *background*
 * @description: change the color of character at (x, y) in VGA text mode.
 *               Initially used to draw Aqua on exception screen, but can be used for others.
 */
void vga_text_set_color(uint8_t x, uint8_t y, uint8_t foreground, uint8_t background) {
    uint8_t color = (background & 0xF) << 4 | (foreground & 0xF);
    *(uint8_t*) (VIDEO + (y * SCREEN_WIDTH + x) * 2 + 1) = color;
}

/* void vga_text_set_character(uint8_t x, uint8_t y, uint8_t ch)
 * @input: x, y - coordinate of character whose color to be changed.
 *         ch - character to be set at (x, y).
 * @output: the character at (x, y) is changed to *ch*
 * @description: change the character at (x, y) in VGA text mode.
 *               Initially used to draw Aqua on exception screen, but can be used for others.
 */
void vga_text_set_character(uint8_t x, uint8_t y, uint8_t ch) {
    // *(uint8_t*) (VIDEO + (y * SCREEN_WIDTH + x) * 2) = ch;
}

/* void vga_text_disable_cursor()
 * @output: the cursor on VGA text mode is no longer there
 * @description: disable the blinking cursor.
 */
void vga_text_disable_cursor() {
    cli();
    // Read the value set at VGA's cursor start register,
    // by first sending an index and then reading the data port
    outb(VGA_REG_CURSOR_START, VGA_PORT_INDEX);
    char data = inb(VGA_PORT_DATA);
    // Replace the value of cursor start register with bit 5 set,
    // here bit 5 is the Cursor Disable field, and value of 1 means
    // the VGA text mode cursor is disabled
    outb(VGA_REG_CURSOR_START, VGA_PORT_INDEX);
    outb(0x20 | data, VGA_PORT_DATA);
    sti();
}

/* void vga_text_enable_cursor()
 * @output: the cursor on VGA text mode appears again
 * @description: enable the blinking cursor.
 */
void vga_text_enable_cursor() {
    cli();
    // Read the value set at VGA's cursor start register,
    // by first sending an index and then reading the data port
    outb(VGA_REG_CURSOR_START, VGA_PORT_INDEX);
    char data = inb(VGA_PORT_DATA);
    // Replace the value of cursor start register with bit 5 cleared,
    // here bit 5 is the Cursor Disable field, and value of 0 means
    // the VGA text mode cursor is enabled
    outb(VGA_REG_CURSOR_START, VGA_PORT_INDEX);
    outb(0xdf & data, VGA_PORT_DATA);
    sti();
}

/* void vga_text_set_cursor_pos(uint8_t x, uint8_t y)
 * @output: the cursor on VGA text mode changes to (x, y)
 * @description: set the position of the blinking cursor.
 */
void vga_text_set_cursor_pos(uint8_t x, uint8_t y) {
    cli();
    // Calculate the character position on VGA plane, row major order
    uint16_t char_pos = (y * SCREEN_WIDTH + x) % (SCREEN_WIDTH * SCREEN_HEIGHT);
    // Replace the value of VGA Cursor Location High Register with high 8 bits
    // of the calculated position
    outb(VGA_REG_CURSOR_LOCATION_HIGH, VGA_PORT_INDEX);
    outb((uint8_t) (char_pos >> 8), VGA_PORT_DATA);
    // Replace the value of VGA Cursor Location Low Register with low 8 bits
    // of the calculated position
    outb(VGA_REG_CURSOR_LOCATION_LOW, VGA_PORT_INDEX);
    outb((uint8_t) char_pos, VGA_PORT_DATA);
    sti();
}


#define VIDEO       0xB8000
#define NUM_COLS    80
#define NUM_ROWS    25
#define ATTRIB      0x7
#define BACKSPACE   0x8
#define NULL_CHAR   0

int screen_x;
int screen_y;
char* video_mem = (char *)VIDEO;

/* void clear(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory */
void clear(void) {
    int32_t i, j;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
    }
    for(i = 0; i < SCREEN_HEIGHT * FONT_HEIGHT; i++) {
        for(j = 0; j < SCREEN_WIDTH * FONT_WIDTH; j++) {
            qemu_vga_pixel_set(j, i, 0, 0, 0);
        }
    }
    screen_x = 0;
    screen_y = 0;
    vga_text_set_cursor_pos(screen_x, screen_y);
}

/* void clear_row(uint32_t row)
 * @input: row - the id of row to be cleared, should be in range 0 to (NUM_ROWS - 1)
 * @output: the row on screen gets cleared
 * @description: clear one row on the screen
 */
void clear_row(uint32_t row) {
    int32_t i, j;
    for(i = row * NUM_COLS; i < (row + 1) * NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
    }
    for(i = row * FONT_HEIGHT; i < row * FONT_HEIGHT + FONT_HEIGHT; i++) {
        for(j = 0; j < SCREEN_WIDTH * FONT_WIDTH; j++) {
            qemu_vga_pixel_set(j, i, 0, 0, 0);
        }
    }
}

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output. */
int32_t printf(int8_t *format, ...) {

    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            putc('%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    puts(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    puts(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            putc((uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            puts(*((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                putc(*buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}

/* int32_t puts(int8_t* s);
 *   Inputs: int_8* s = pointer to a string of characters
 *   Return Value: Number of bytes written
 *   Function: Output a string to the console */
int32_t puts(int8_t* s) {
    register int32_t index = 0;
    while (s[index] != '\0') {
        putc(s[index]);
        index++;
    }
    return index;
}

/* void putc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 * Function: Output a character to the console */
void putc(uint8_t c)
{
    // if reach the right bottom of the screen
    if (NUM_COLS * screen_y + screen_x >= NUM_COLS * NUM_ROWS)
    {
        roll_up();
    }

    if(c == '\n' || c == '\r') {
        screen_y++;
        if (screen_y >= NUM_ROWS)
        {
          roll_up();
        }
        clear_row(screen_y);    // Clear the new line for better display
        screen_x = 0;
    } else {
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1)) = c;
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = ATTRIB;
        qemu_vga_putc(screen_x, screen_y, c, 255, 255, 255, 0, 0, 0);

        screen_x++;
        if(screen_x >= NUM_COLS) {  // If the line is filled up
            screen_x = 0;
            screen_y++;
            // if reach the right bottom of the screen
            if (screen_y >= NUM_ROWS)
            {
                roll_up();
            }
            clear_row(screen_y);    // Clear the new line for better display
        }
    }
    vga_text_set_cursor_pos(screen_x, screen_y);
}

/* void roll_up();
 * Inputs: none
 * Return Value: void
 * Function:roll the page up one line */
void roll_up()
{
    int32_t index;
    for (index = 0; index < (NUM_ROWS - 1) * NUM_COLS; index++)
    {
        *(uint8_t *)(video_mem + (index << 1)) = *(uint8_t *)(video_mem + ((index + NUM_COLS) << 1));
        *(uint8_t *)(video_mem + (index << 1) + 1) = *(uint8_t *)(video_mem + ((index + NUM_COLS) << 1) + 1);
    }
    qemu_vga_scroll_up(FONT_HEIGHT);
    screen_x = 0;
    screen_y = NUM_ROWS - 1;
    clear_row(NUM_ROWS - 1);
}

/* void keyboard_echo(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the console (only used by keyboard driver) */
void keyboard_echo(uint8_t c)
{
    if (c == NULL_CHAR)
    {
        return;
    }
    // if reach the right bottom of the screen
    if (NUM_COLS * screen_y + screen_x >= NUM_COLS * NUM_ROWS)
    {
        roll_up();
    }
    // new line
    if(c == '\n' || c == '\r') {
        screen_y++;
        // if reach the right bottom of the screen
        if (screen_y >= NUM_ROWS)
        {
            roll_up();
        }
        clear_row(screen_y);    // Clear the new line for better display
        screen_x = 0;
    }
    // backspace
    else if (c == BACKSPACE)
    {
        screen_x--;
        // If the line is filled up
        if(screen_x < 0)
        {
            clear_row(screen_y);
            screen_x = NUM_COLS - 1;
            screen_y -= 1;
            if(screen_y < 0)
            {
                screen_x = 0;
                screen_y = 0;
            }
        }
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1)) = ' ';
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = ATTRIB;
        qemu_vga_putc(screen_x, screen_y, ' ', 0, 0, 0, 0, 0, 0);
    }
    // default
    else
    {
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1)) = c;
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = ATTRIB;
        qemu_vga_putc(screen_x, screen_y, c, 255, 255, 255, 0, 0, 0);
        screen_x++;
        // If the line is filled up
        if(screen_x >= NUM_COLS)
        {
            screen_x = 0;
            screen_y++;
            // if reach the right bottom of the screen
            if (screen_y >= NUM_ROWS)
            {
                roll_up();
            }
            clear_row(screen_y);    // Clear the new line for better display
        }
    }
    vga_text_set_cursor_pos(screen_x, screen_y);
}

/* void test_interrupts(void)
 * Inputs: void
 * Return Value: void
 * Function: increments video memory. To be used to test rtc */
void test_interrupts(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }
}
