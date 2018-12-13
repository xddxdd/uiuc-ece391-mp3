#include "sys_calls.h"
#include "../fs/unified_fs.h"
#include "multiprocessing.h"
#include "../devices/acpi.h"
#include "../devices/vga_text.h"
#include "../devices/qemu_vga.h"
#include "../lib/status_bar.h"
// System calls for checkpoint 3.

/*
 * int32_t syscall_halt (uint8_t status)
 * system call halt
 * INPUT: status - process status
 * OUTPUT: none
 */
int32_t syscall_halt (uint8_t status)
{
    // printf("halt with status %d\n", status);
    cli();
    int32_t ret = process_halt(status);
    sti();
    return ret;
}

/*
 * int32_t syscall_execute (const uint8_t* command)
 * system call execute
 * INPUT: command - command for system call
 * OUTPUT: SUCCESS and FAIL
 */
int32_t syscall_execute (const uint8_t* command)
{
    cli();
    int32_t ret = process_create((const char*) command);
    sti();
    return ret;
}

/*
 * int32_t syscall_read (int32_t fd, void* buf, int32_t nbytes)
 * system call read
 * INPUT: fd - file descriptor
 *        buf - buffer used to write to terminal
 *        nbytes - number of bytes to write
 * OUTPUT: SUCCESS and FAIL
 */
int32_t syscall_read (int32_t fd, void* buf, int32_t nbytes){
    pcb_t* pcb = process_get_active_pcb();
    return unified_read(pcb->fd_array, fd, buf, nbytes);
}

/*
 * int32_t syscall_write (int32_t fd, void* buf, int32_t nbytes)
 * system call write
 * INPUT: fd - file descriptor
 *        buf - buffer used to write to terminal
 *        nbytes - number of bytes to write
 * OUTPUT: SUCCESS and FAIL
 */
int32_t syscall_write (int32_t fd, const void* buf, int32_t nbytes){
    pcb_t* pcb = process_get_active_pcb();
    return unified_write(pcb->fd_array, fd, buf, nbytes);
}

/*
 * int32_t syscall_open (const uint8_t* filename)
 * system call open
 * INPUT: filename - file name
 * OUTPUT: SUCCESS and FAIL
 */
int32_t syscall_open (const uint8_t* filename){
    pcb_t* pcb = process_get_active_pcb();
    return unified_open(pcb->fd_array, (const char*) filename);
}

/*
 * int32_t syscall_close (int32_t fd)
 * system call close
 * INPUT: filename - file name
 * OUTPUT: SUCCESS and FAIL
 */
int32_t syscall_close (int32_t fd){
    pcb_t* pcb = process_get_active_pcb();
    return unified_close(pcb->fd_array, fd);
}


// System calls for checkpoint 4.
/*
 * int32_t syscall_getargs (uint8_t* buf, int32_t nbytes)
 * @input: buf - where the argument should be written to
 *         nbytes - max length of argument
 * @output: buf - written with argument to current process
 *          ret val - SUCCESS / FAIL
 * @description: system call to get argument of execute
 *     of current process. If the buf is too small, the call will fail;
 *     otherwise, argument is copied into buf.
 */
int32_t syscall_getargs (uint8_t* buf, int32_t nbytes){
    if(buf == NULL) return FAIL;
    pcb_t* pcb = process_get_active_pcb();
    if(pcb->arg[0] == 0) return FAIL;
    if(strlen(pcb->arg) > nbytes) return FAIL;
    memset(buf, 0, nbytes);
    memcpy(buf, pcb->arg, nbytes > MAX_ARG_LENGTH ? MAX_ARG_LENGTH : nbytes);
    // printf("arg: %s\n", pcb->arg);
    return SUCCESS;
}

/*
 * int32_t syscall_vidmap (uint8_t** screen_start)
 * @input: none
 * @output: screen_start - double pointer used to hold the video memory address
 *                        for user programs
 *          ret val -  video memory address for user programs
 * @description: system call to maps the text-mode video memory into
 *               user space at a pre-set virtual address
 */
int32_t syscall_vidmap (uint8_t** screen_start)
{
    // Check whether screen_start is in user app range
    if(NULL == screen_start) return FAIL;
    if(((uint32_t) screen_start >> PD_ADDR_OFFSET)
        != ((uint32_t) USER_PROCESS_ADDR >> PD_ADDR_OFFSET)) return FAIL;

    // Enable vidmap for this process, reset its paging
    process_get_active_pcb()->vidmap = 1;
    process_switch_paging(active_process_id);

    // Return the address
    *screen_start = (uint8_t *) USER_VIDEO;
    return SUCCESS;
}

//Extra credit system calls.
int32_t syscall_set_handler (int32_t signum, void* handler_address){
    return FAIL;
}

int32_t syscall_sigreturn (void){
    return FAIL;
}

/*
 * int32_t syscall_ioctl (int32_t fd, int32_t op)
 * system call ioctl
 * INPUT: fd - file descriptor
 *        op - ioctl operation number
 * OUTPUT: SUCCESS and FAIL
 */
int32_t syscall_ioctl (int32_t fd, int32_t op){
    pcb_t* pcb = process_get_active_pcb();
    return unified_ioctl(pcb->fd_array, fd, op);
}

/* int32_t syscall_shutdown(void)
 * @output: system shuts down
 * @description: shuts down system using ACPI
 */
int32_t syscall_shutdown(void) {
    return acpi_shutdown();
}

/* int32_t syscall_reboot(void)
 * @output: system reboots
 * @description: reboots system using 8042 keyboard controller
 */
int32_t syscall_reboot(void) {
    return acpi_reboot();
}

/* int32_t syscall_ps(void)
 * @output: status & output handles of all terminals and processes
 * @description: similar to the ps command on linux.
 */
int32_t syscall_ps(void) {
    int pid;
    for(pid = 0; pid < PROCESS_COUNT; pid++) {
        process_t* pcb = process_get_pcb(pid);
        if(!pcb->present) {
            printf("P#%d (NULL)\n", pid);
        } else {
            printf("P#%d %s %s, parent=%d, terminal=%d", pid,
                pcb->cmd, pcb->arg, pcb->parent_pid, pcb->terminal);
            if(terminals[pcb->terminal].active_process == pid) {
                puts(" (active)");
            }
            puts("\n    Files: ");
            int fd;
            for(fd = 0; fd < MAX_NUM_FD_ENTRY; fd++) {
                if(NULL != pcb->fd_array[fd].interface) {
                    putc('+');
                } else {
                    putc('-');
                }
            }
            putc('\n');
        }
    }
    int tid;
    for(tid = 0; tid < TERMINAL_COUNT; tid++) {
        printf("T#%d pid=%d\n", tid, terminals[tid].active_process);
    }
    return SUCCESS;
}

/* int32_t syscall_poke(uint32_t x, uint32_t y, uint32_t data)
 * @input: x, y - screen coordinate
 *         data - bit 0 - 7: character to display
 *                bit 8 - 15: attribute of character
 * @output: character on (x, y) changed to data
 * @description: poke function as an alternate to vidmap.
 *     Supports both text mode and QEMU VGA.
 */
int32_t syscall_poke(uint32_t x, uint32_t y, uint32_t data) {
    if(x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
        // printf("%d %d %x F\n", x, y, data);
        return FAIL;
    }
    uint8_t ch = (uint8_t) data;
    uint8_t attrib = (uint8_t) (data >> 8);

    // printf("%d %d %x\n", x, y, data);

    *(uint8_t *)(video_mem + ((NUM_COLS * y + x) << 1)) = ch;
    *(uint8_t *)(video_mem + ((NUM_COLS * y + x) << 1) + 1) = attrib;

    qemu_vga_putc(x * FONT_ACTUAL_WIDTH, y * FONT_ACTUAL_HEIGHT,
        ch, qemu_vga_get_terminal_color(attrib), qemu_vga_get_terminal_color(attrib >> 4));

    return SUCCESS;
}

/* int32_t syscall_status_msg(char* msg, uint32_t len, uint8_t attr)
 * @input: msg, len - data and length of message for status bar
 * @output: attr - attribute
 * @description: display a message on status bar
 */
int32_t syscall_status_msg(char* msg, uint32_t len, uint8_t attr) {
    if(NULL == msg) return FAIL;
    status_bar_update_message(msg, len, attr);
    return SUCCESS;
}
