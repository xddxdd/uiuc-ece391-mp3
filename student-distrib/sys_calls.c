#include "sys_calls.h"


static uint32_t process_count = 0;

/* PCB */

/*
 * A null function placeholder for stdin and stdout.
 * INPUT:  none.
 * OUTPUT: none.
 * RETURN: SYSCALL_SUCCESS.
 */
int32_t null_func()
{
   return SYSCALL_SUCCESS;
}

/* File operations table.
* The entries are functions for:
* open, read, write, close,
* respectively.
*/
/* File operations table for stdin. */
int32_t (*stdin_table[FILE_OP_NUM])() = {null_func, terminal_read, null_func, null_func};

/* File operations table for stdout. */
int32_t (*stdout_table[FILE_OP_NUM])() = {null_func, null_func, terminal_write, null_func};

/* File operations table for rtc. */
int32_t (*rtc_table[FILE_OP_NUM])() = {rtc_open, rtc_read, rtc_write, rtc_close};

/* File operations table for file. */
int32_t (*file_table[FILE_OP_NUM])() = {file_open, file_read, file_write, file_close};

/* File operations rable for directory. */
int32_t (*dir_table[FILE_OP_NUM])() = {dir_open, dir_read, dir_write, dir_close};

/* pcb_init - Added by jinghua3.
 *
 * Initialize the process control block
 * for a process with process id = pid.
 * INPUT:  int32_t pid which is the process id.
 * OUTPUT: init pcb for process whose process id is pid.
 * RETURN: A pointer to the initialized pcb.
 */
pcb_t* pcb_init(int32_t pid)
{
    int i, j;
    // Set the address of pcb at the top of the kernel stack.
    pcb_t * pcb;
    pcb = (pcb_t *)KERNEL_STACK_BASE_ADDR - process_count * USER_KMODE_STACK_SIZE;
    // Initialize all the fd_entries.
    for (i=0; i<MAX_NUM_FD_ENTRY; i++)
    {
        for (j=0; j<FILE_OP_NUM; j++)
        {
            pcb->fd_entry[i].fileOpTable_ptr[j] = null_func;
        }
        pcb->fd_entry[i].inode = NOT_ASSIGNED;
    		pcb->fd_entry[i].file_position = NOT_ASSIGNED;
    		pcb->fd_entry[i].flags = FD_ENTRY_NOT_ASSIGNED;
    }
    // Initalize stdin.
    for(i=0; i<FILE_OP_NUM; i++)
    {
        pcb->fd_entry[STDIN_ENTRY].fileOpTable_ptr[i] = stdin_table[i];
    }
	pcb->fd_entry[STDIN_ENTRY].flags = FD_ENTRY_ASSIGNED;
    // Initialize stdout.
    for(i=0; i<FILE_OP_NUM; i++)
    {
        pcb->fd_entry[STDOUT_ENTRY].fileOpTable_ptr[i] = stdout_table[i];
    }
	pcb->fd_entry[STDOUT_ENTRY].flags = FD_ENTRY_ASSIGNED;
    pcb->current_pid = pid;
    pcb->parent_pid = NOT_ASSIGNED;
    pcb->parent_pcb = NULL;
    pcb->esp = NOT_ASSIGNED;
    return pcb;
}

// System calls for checkpoint 3.

int32_t halt (uint8_t status)
{
    if (process_count == 1)
    {
        execute ((uint8_t*)"shell");
    }
    // extract current pcb
    pcb_t * pcb;
    pcb = (pcb_t *)KERNEL_STACK_BASE_ADDR - process_count * USER_KMODE_STACK_SIZE;
    // modify TSS:
    // ss0: kernel’s stack segment, esp0: process’s kernel-mode stack
    tss.esp0 = KERNEL_STACK_BASE_ADDR - (pcb->parent_pid - 1) * USER_KMODE_STACK_SIZE - 0x4;
    // restore paging
    _halt_paging(pcb->parent_pid);
    // decrease process_count
    process_count--;
    // store status in eax
    asm volatile ("       \n\
        movl %%ecx, %%esp \n\
        movl %%ebx, %%eax \n\
        jmp halt_return   \n\
        "
        :
        : "b" (status), "c" (pcb->esp)
        : "eax", "esp"
    );
    return SYSCALL_SUCCESS;
}

int32_t execute (const uint8_t* command)
{
    /* Parsing */
    printf("System call [execute]: start parsing\n");                           /* for testing */
    // initialize filename buffer
    uint8_t filename[ECE391FS_MAX_FILENAME_LEN];
    memset(filename, 0, ECE391FS_MAX_FILENAME_LEN);
    // get filename
    _execute_parse(command, filename);
    printf("Filename is %s\n", filename);                                       /* for testing */
    /* Executable check */
    printf("System call [execute]: start executable check\n");                  /* for testing */
    // check if the file exist
    int32_t fd;
    if(ECE391FS_CALL_FAIL == file_open(&fd, (char*)filename))
    {
        printf("Error: system call [execute]: cannot open file %s\n", filename);
        return SYSCALL_FAIL;
    }
    if (SYSCALL_SUCCESS != _execute_exe_check(&fd))
    {
        printf("Error: system call [execute]: %s is not executable\n", filename);
        return SYSCALL_FAIL;
    }
    /* Pgaing */
    printf("System call [execute]: start paging\n");                            /* for testing */
    _execute_paging();
    /* User-level program loader */
    if (SYSCALL_SUCCESS != _execute_pgm_loader(&fd))
    {
        printf("Error: system call [execute]: cannot load file to memory\n");
        return SYSCALL_FAIL;
    }
    // close the file since it has been copied to memory
    if(ECE391FS_CALL_FAIL == file_close(&fd))
    {
        printf("Error: system call [execute]: cannot close file %s\n", filename);
        return SYSCALL_FAIL;
    }
    /* Create PCB */
    pcb_t* pcb = pcb_init(process_count);
    pcb->parent_pid = process_count - 1;
    uint32_t esp;
    asm volatile (
        "movl %%esp, %0"
        :"=r" (esp)
    );
    pcb->esp = esp;
    pcb->parent_pcb = process_count <= 1 ? NULL :
                      (pcb_t *)KERNEL_STACK_BASE_ADDR -
                               (process_count - 1) * USER_KMODE_STACK_SIZE;
    /* Context Switch */
    printf("System call [execute]: start context switch\n");                    /* for testing */
    _execute_context_switch();
    return SYSCALL_SUCCESS;
}

int32_t read (int32_t fd, void* buf, int32_t nbytes){

    return SYSCALL_SUCCESS;
}

int32_t write (int32_t fd, const void* buf, int32_t nbytes){
    printf("lalala\n");
    return SYSCALL_SUCCESS;
}

int32_t open (const uint8_t* filename){

    return SYSCALL_SUCCESS;
}

int32_t close (int32_t fd){

    return SYSCALL_SUCCESS;
}


// System calls for checkpoint 4.
int32_t getargs (uint8_t* buf, int32_t nbytes){

    return SYSCALL_SUCCESS;
}

int32_t vidmap (uint8_t** screen_start){

    return SYSCALL_SUCCESS;
}


//Extra credit system calls.
int32_t set_handler (int32_t signum, void* handler_address){

    return SYSCALL_SUCCESS;
}

int32_t sigreturn (void){

    return SYSCALL_SUCCESS;
}


/* Helper functions for sytstem calls */
/*
 * _execute_parse(const uint8_t* command, uint8_t* filename)
 *    @description: helper function called by sytstem call execute()
 *                  to parse the commands
 *    @input: command -  a space-separated sequence of words.
 *                       The first word is the file name of the program to be
 *                       executed, and the rest of the command—stripped of
 *                       leading spaces—should be provided to the new
 *                       program on request via the getargs system call
 *            filename - empry buffer used to hold the filename as a return
 *                       value. Its size should be no smaller than ECE391FS_MAX_FILENAME_LEN
 *    @output: filename - as described above
 *    @note: need to add support for arguments parsing
 */
void _execute_parse (const uint8_t* command, uint8_t* filename)
{
    // loop variable
    int index;
    // parsing
    for (index = 0; index < ECE391FS_MAX_FILENAME_LEN; index++)
    {
        // end of filename
        if (command[index] == SPACE) break;
        // read filename to the filename buffer
        else filename[index] = command[index];
    }
    printf("finish _execute_parse()\n");                                        /* for testing */
    return;
}

/*
 * int32_t _execute_exe_check (int32_t* fd)
 *    @description: helper function called by sytstem call execute()
 *                  to check if the file is executable or not
 *    @input: fd - the input file descriptor
 *    @output: SYSCALL_FAIL if fail, and SYSCALL_SUCCESS if succeed
 */
int32_t _execute_exe_check (int32_t* fd)
{
    // read the header of the file
	char buf[FILE_HEADER_LEN];
    memset(buf, 0, FILE_HEADER_LEN);
    uint32_t offset = 0;
	if(ECE391FS_CALL_FAIL == file_read(fd, &offset, buf, FILE_HEADER_LEN))
    {
        printf("_execute_exe_check(): file read fails\n");                       /* for testing */
        return SYSCALL_FAIL;
    }
    if(offset != FILE_HEADER_LEN)
    {
        printf("_execute_exe_check(): header length does not match\n");         /* for testing */
        return SYSCALL_FAIL;
    }
    // check if the file is executable
    if ((buf[0] != FILE_EXE_HEADER_0) |
        (buf[1] != FILE_EXE_HEADER_1) |
        (buf[2] != FILE_EXE_HEADER_2) |
        (buf[3] != FILE_EXE_HEADER_3))
        {
            printf("_execute_exe_check(): file not executable\n");              /* for testing */
            return SYSCALL_FAIL;
        }
    printf("System call [execute]: finish _execute_exe_check()\n");             /* for testing */
    return SYSCALL_SUCCESS;
}

/*
 * void _execute_paging ()
 *    @description: helper function called by sytstem call execute()
 *                  to set up a single 4 MB page directory that maps
 *                  virtual address 0x08000000 (128 MB) to the right physical
 *                  memory address (either 8 MB or 12 MB)
 *    @input: none
 *    @output: none
 *    @side effect: flush the TLB
 */
void _execute_paging ()
{
    // get the entry in page directory for the input process
    uint32_t PD_index = (uint32_t) USER_PROCESS_ADDR >> PD_ADDR_OFFSET;
    printf("_execute_paging(): PD_index is %x\n", PD_index);                    /* for testing */
    // present
    page_directory[PD_index].pde_MB.present = 1;
    // read only
    page_directory[PD_index].pde_MB.read_write = 1;
    // user level
    page_directory[PD_index].pde_MB.user_supervisor = 1;
    page_directory[PD_index].pde_MB.write_through = 0;
    page_directory[PD_index].pde_MB.cache_disabled = 0;
    page_directory[PD_index].pde_MB.accessed = 0;
    page_directory[PD_index].pde_MB.dirty = 0;
    // 4MB page
    page_directory[PD_index].pde_MB.page_size = 1;
    page_directory[PD_index].pde_MB.global = 0;
    page_directory[PD_index].pde_MB.avail = 0;
    page_directory[PD_index].pde_MB.pat = 0;
    page_directory[PD_index].pde_MB.reserved = 0;
    // physical address = PROCESS_PYSC_BASE_ADDR + process_count
    page_directory[PD_index].pde_MB.PB_addr = PROCESS_PYSC_BASE_ADDR + process_count;
    // increment the process_count
    process_count++;
    printf("_execute_paging(): process count is %d\n", process_count);          /* for testing */
    // flush the TLB by writing to the page directory base register (CR3)
    // reference: https://wiki.osdev.org/TLB
    asm volatile ("              \n\
  	        movl %%cr3, %%eax    \n\
  	        movl %%eax, %%cr3    \n\
            "
            : : : "eax", "cc"
    );
    printf("System call [execute]: finish _execute_paging()\n");                /* for testing */
    return;
}

/*
 * int32_t _execute_pgm_loader (int32_t* fd)
 *    @description: helper function called by sytstem call execute()
 *                  to copy the program image to the user process memory
 *    @input: fd - the input file descriptor
 *    @output: SYSCALL_FAIL if fail, and SYSCALL_SUCCESS if succeed
 *    @side effect: modify the user memory
 */
int32_t _execute_pgm_loader (int32_t* fd)
{
    // loop variable
    int32_t index;
    // read the content of the file
    // ------------ to be edited ------------
    int filelen = 0x2000;
	char buf[filelen];
    memset(buf, 0, filelen);
    uint32_t offset = 0;
	if(ECE391FS_CALL_FAIL == file_read(fd, &offset, buf, filelen))
    {
        printf("_execute_pgm_loader(): file read fails\n");                     /* for testing */
        return SYSCALL_FAIL;
    }
    // get the user memory address (logical)
    char* user_mem = (char *)USER_PROCESS_ADDR;
    // copy the program file data into the memory for that program
    for (index = 0; index < filelen; index++)
    {
        if (buf[index] == 0)
        {
            // printf("_execute_pgm_loader(): break at index %d\n", index);        /* for testing */
            // break;
        }
        // else *(uint8_t *)(user_mem + index) = buf[index];
        *(uint8_t *)(user_mem + index) = buf[index];
    }
    printf("System call [execute]: finish _execute_pgm_loader()\n");            /* for testing */
    return SYSCALL_SUCCESS;
}

/*
 * void _execute_context_switch ()
 *    @description: helper function called by sytstem call execute()
 *                  to perform context switch
 *    @input: none
 *    @output: none
 *    @side effect: clobber some registers and flags (EIP, CS, EFLAGS, ESP, SS and DS)
 *                  modify TSS's ss0 and esp0
 */
void _execute_context_switch ()
{
    // get the user memory address (logical)
    char* user_mem = (char *)USER_PROCESS_ADDR;
    // get the entry point from bytes 24-27 of the executable
    uint32_t entry_point = (*(uint32_t *)(user_mem + 24));
    uint32_t user_kmode_stack = KERNEL_STACK_BASE_ADDR -
                                (process_count - 1) * USER_KMODE_STACK_SIZE - 0x4;
    printf("_execute_context_switch(): user mem addr is %x\n", user_mem);       /* for testing */
    printf("_execute_context_switch(): entry point is %x\n", entry_point);      /* for testing */
    printf("_execute_context_switch(): user stack is at %x\n", USER_STACK_ADDR);/* for testing */
    printf("_execute_context_switch(): user kmode stack is at %x\n",
           user_kmode_stack);                                                   /* for testing */
    // modify TSS:
    // ss0: kernel’s stack segment, esp0: process’s kernel-mode stack
    printf("_execute_context_switch(): preparing TSS for IRET\n");              /* for testing */
    tss.esp0 = user_kmode_stack;
    printf("_execute_context_switch(): preparing stack for IRET\n");            /* for testing */
    // 1. set up stack: (top) EIP, CS, EFLAGS, ESP, SS (bottom)
    // 2. set DS to point to the correct entry in GDT for the user mode data segment
    // reference: https://www.felixcloutier.com/x86/IRET:IRETD.html
    asm volatile ("                \n\
            andl    $0x00FF, %%ebx \n\
            movw    %%bx, %%ds     \n\
            pushl   %%ebx          \n\
            pushl   %%edx          \n\
            pushfl                 \n\
            pushl   %%ecx          \n\
            pushl   %%eax          \n\
            iret                   \n\
            "
            :
            : "a"(entry_point), "b"(USER_DS), "c"(USER_CS), "d"(USER_STACK_ADDR)
            : "cc"
    );
    asm volatile ("halt_return:");
    return;
}

/*
 * void _halt_paging (int32_t pid)
 *    @description: helper function called by sytstem call halt()
 *                  to deconstruct process page
 *    @input: pid - parent pid
 *    @output: none
 *    @side effect: flush the TLB
 */
void _halt_paging (int32_t pid)
{
    // get the entry in page directory for the input process
    uint32_t PD_index = (uint32_t) USER_PROCESS_ADDR >> PD_ADDR_OFFSET;
    printf("_halt_paging(): PD_index is %x\n", PD_index);                       /* for testing */
    // present
    page_directory[PD_index].pde_MB.present = 1;
    // read only
    page_directory[PD_index].pde_MB.read_write = 1;
    // user level
    page_directory[PD_index].pde_MB.user_supervisor = 1;
    page_directory[PD_index].pde_MB.write_through = 0;
    page_directory[PD_index].pde_MB.cache_disabled = 0;
    page_directory[PD_index].pde_MB.accessed = 0;
    page_directory[PD_index].pde_MB.dirty = 0;
    // 4MB page
    page_directory[PD_index].pde_MB.page_size = 1;
    page_directory[PD_index].pde_MB.global = 0;
    page_directory[PD_index].pde_MB.avail = 0;
    page_directory[PD_index].pde_MB.pat = 0;
    page_directory[PD_index].pde_MB.reserved = 0;
    // physical address = PROCESS_PYSC_BASE_ADDR + pid
    page_directory[PD_index].pde_MB.PB_addr = PROCESS_PYSC_BASE_ADDR + (pid - 1);
    // flush the TLB by writing to the page directory base register (CR3)
    // reference: https://wiki.osdev.org/TLB
    asm volatile ("              \n\
  	        movl %%cr3, %%eax    \n\
  	        movl %%eax, %%cr3    \n\
            "
            : : : "eax", "cc"
    );
    printf("System call [halt]: finish _halt_paging()\n");                      /* for testing */
    return;
}
