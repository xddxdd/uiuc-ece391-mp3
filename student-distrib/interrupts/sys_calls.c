#include "sys_calls.h"
// System calls for checkpoint 3.

/*
 * int32_t halt (uint8_t status)
 * system call halt
 * INPUT: status - process status
 * OUTPUT: none
 */
int32_t halt (uint8_t status)
{
    return process_halt(status);
}

/*
 * int32_t execute (const uint8_t* command)
 * system call execute
 * INPUT: command - command for system call
 * OUTPUT: SUCCESS and FAIL
 */
int32_t execute (const uint8_t* command)
{
    return process_create((const char*) command);
}

/*
 * int32_t read (int32_t fd, void* buf, int32_t nbytes)
 * system call read
 * INPUT: fd - fild descriptor
 *        buf - buffer used to write to terminal
 *        nbytes - number of bytes to write
 * OUTPUT: SUCCESS and FAIL
 */
int32_t read (int32_t fd, void* buf, int32_t nbytes){
    pcb_t* pcb = process_get_active_pcb();
    return unified_read(pcb->fd_array, fd, buf, nbytes);
}

/*
 * int32_t write (int32_t fd, void* buf, int32_t nbytes)
 * system call write
 * INPUT: fd - fild descriptor
 *        buf - buffer used to write to terminal
 *        nbytes - number of bytes to write
 * OUTPUT: SUCCESS and FAIL
 */
int32_t write (int32_t fd, const void* buf, int32_t nbytes){
    pcb_t* pcb = process_get_active_pcb();
    return unified_write(pcb->fd_array, fd, buf, nbytes);
}

/*
 * int32_t open (const uint8_t* filename)
 * system call open
 * INPUT: filename - file name
 * OUTPUT: SUCCESS and FAIL
 */
int32_t open (const uint8_t* filename){
    pcb_t* pcb = process_get_active_pcb();
    return unified_open(pcb->fd_array, (const char*) filename);
}

/*
 * int32_t close (int32_t fd)
 * system call close
 * INPUT: filename - file name
 * OUTPUT: SUCCESS and FAIL
 */
int32_t close (int32_t fd){
    pcb_t* pcb = process_get_active_pcb();
    return unified_close(pcb->fd_array, fd);
}


// System calls for checkpoint 4.
/*
 * int32_t getargs (uint8_t* buf, int32_t nbytes)
 * @input: buf - where the argument should be written to
 *         nbytes - max length of argument
 * @output: buf - written with argument to current process
 *          ret val - SUCCESS / FAIL
 * @description: system call to get argument of execute
 *     of current process. If the buf is too small, the call will fail;
 *     otherwise, argument is copied into buf.
 */
int32_t getargs (uint8_t* buf, int32_t nbytes){
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
 * int32_t vidmap (uint8_t** screen_start)
 * @input: none
 * @output: screen_start - double pointer used to hold the video memory address
 *                        for user programs
 *          ret val -  video memory address for user programs
 * @description: system call to maps the text-mode video memory into
 *               user space at a pre-set virtual address
 */
int32_t vidmap (uint8_t** screen_start)
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
int32_t set_handler (int32_t signum, void* handler_address){
    return FAIL;
}

int32_t sigreturn (void){
    return FAIL;
}
