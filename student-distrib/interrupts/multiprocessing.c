#include "multiprocessing.h"

char program_header[PROGRAM_HEADER_LEN] = {0x7f, 0x45, 0x4c, 0x46};

terminal_t terminals[TERMINAL_COUNT];

int32_t displayed_terminal_id = 0;
int32_t active_terminal_id = 0;
int32_t active_process_id = -1;

process_t* process_get_active_pcb() {
    return process_get_pcb(active_process_id);
}

process_t* process_get_pcb(int32_t pid) {
    if(pid < 0 || pid >= PROCESS_COUNT) return NULL;
    return (process_t*) (KERNEL_STACK_BASE_ADDR - (pid + 1) * USER_KMODE_STACK_SIZE);
}

void process_init() {
    int i;
    for(i = 0; i < PROCESS_COUNT; i++) {
        process_t* process = process_get_pcb(i);
        process->present = 0;
    }
    for(i = 0; i < TERMINAL_COUNT; i++) {
        terminals[i].active_process = -1;
        terminals[i].screen_x = 0;
        terminals[i].screen_y = 0;
    }
}

int32_t process_allocate() {
    int i;
    cli();
    for(i = 0; i < PROCESS_COUNT; i++) {
        process_t* process = process_get_pcb(i);
        if(0 == process->present) {
            process->present = 1;
            sti();
            return i;
        }
    }
    sti();
    return -1;
}

int32_t process_create(const char* command) {
    if(NULL == command) return FAIL;

    // initialize filename buffer
    uint8_t filename[ECE391FS_MAX_FILENAME_LEN + 1];
    memset(filename, 0, ECE391FS_MAX_FILENAME_LEN + 1);
    uint8_t argument[MAX_ARG_LENGTH + 1];
    memset(argument, 0, MAX_ARG_LENGTH + 1);
    // get filename and argument
    int i = 0;
    while(command[i] != STRING_END && command[i] != SPACE) i++;
    memcpy(filename, command, i);
    if(command[i] == SPACE) {
        // command includes an argument
        int j = i + 1;
        while(command[j] != STRING_END) j++;
        memcpy(argument, command + i + 1, j - i - 1);
    }

    // Temporary file descriptor array for reading program files
	fd_array_t fd_array[MAX_OPEN_FILES];
	if(FAIL == unified_init(fd_array)) return FAIL;
    int32_t fd;

    // Check if file is a program
    char buf[FILE_HEADER_LEN];
    if(FAIL == (fd = unified_open(fd_array, (char*) filename))) return FAIL;
    if(FAIL == unified_read(fd_array, fd, buf, PROGRAM_HEADER_LEN)) return FAIL;
    if(fd_array[fd].pos != PROGRAM_HEADER_LEN) return FAIL;
    if(0 != strncmp((char*) buf, program_header, PROGRAM_HEADER_LEN)) return FAIL;
    if(FAIL == unified_close(fd_array, fd)) return FAIL;

    // Try to allocate PID for new process
    int32_t pid = process_allocate();
    if(-1 == pid) return FAIL;

    // Create process structure
    process_t* process = process_get_pcb(pid);
    unified_init(process->fd_array);
    process->parent_pid = active_process_id;
    process->esp = USER_STACK_ADDR;
    process->ebp = USER_STACK_ADDR;
    process->terminal = active_terminal_id;
    terminals[active_terminal_id].active_process = pid;
    process->vidmap = 0;
    memcpy(process->arg, argument, MAX_ARG_LENGTH + 1);

    // Change paging configuration, load program
    process_switch_paging(pid);
    if(FAIL == (fd = unified_open(fd_array, (char*) filename))) return FAIL;
    if(FAIL == unified_read(fd_array, fd, (char*) USER_PROCESS_ADDR, PROGRAM_MAX_LEN)) return FAIL;
    if(FAIL == unified_close(fd_array, fd)) return FAIL;
    process->eip = (*(uint32_t*) (USER_PROCESS_ADDR + 24));

    process_t* ori_process = process_get_pcb(active_process_id);
    if(NULL != ori_process) {
        asm volatile("movl %%esp, %0":"=r" (ori_process->esp));
        asm volatile("movl %%ebp, %0":"=r" (ori_process->ebp));
        // printf("saved %d, esp %x, ebp %x\n", active_process_id, ori_process->esp, ori_process->ebp);
    }

    process_switch_context(pid);
    return SUCCESS;
}

int32_t process_halt(uint8_t status) {
    // extract current pcb
    process_t* process = process_get_pcb(active_process_id);
    if(NULL == process) return FAIL;

    if(-1 == process->parent_pid) {
        // This process is shell, need to be restarted
        process->present = 0;
        active_process_id = -1;
        terminals[active_terminal_id].active_process = -1;
        process_create("shell");
    } else {
        int32_t parent = process->parent_pid;
        process->present = 0;
        tss.esp0 = KERNEL_STACK_BASE_ADDR - parent * USER_KMODE_STACK_SIZE - 0x4;
        process_switch_paging(parent);
        process = process_get_pcb(parent);
        if(NULL == process) return FAIL;
        active_process_id = parent;
        terminals[active_terminal_id].active_process = parent;
        asm volatile ("         \n\
            movl %%ecx, %%esp   \n\
            movl %%edx, %%ebp   \n\
            movl %%ebx, %%eax   \n\
            leave               \n\
            ret                 \n\
            "
            :
            : "b" (status), "c" (process->esp), "d" (process->ebp)
            : "eax", "ebp", "esp"
        );
    }
    return SUCCESS;
}

void process_switch_paging(int32_t pid) {
    if(pid < 0 || pid >= PROCESS_COUNT) return;
    process_t* process = process_get_pcb(pid);
    if(NULL == process) return;

    // get the entry in page directory for the input process
    uint32_t PD_index = (uint32_t) USER_PROCESS_ADDR >> PD_ADDR_OFFSET;
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
    page_directory[PD_index].pde_MB.PB_addr = PROCESS_PYSC_BASE_ADDR + pid;

    // Direct video mem R/W to main display / alt display based on terminal ids
    if(active_terminal_id == displayed_terminal_id) {
        page_table[VIDEO_MEM_INDEX].PB_addr = VIDEO_MEM_INDEX;
        page_table_usermap[VIDEO_MEM_INDEX].PB_addr = VIDEO_MEM_INDEX;
    } else {
        page_table[VIDEO_MEM_INDEX].PB_addr = VIDEO_MEM_ALT_START + process->terminal;
        page_table_usermap[VIDEO_MEM_INDEX].PB_addr = VIDEO_MEM_ALT_START + process->terminal;
    }

    page_table_usermap[VIDEO_MEM_INDEX].present = process->vidmap;

    // flush the TLB by writing to the page directory base register (CR3)
    // reference: https://wiki.osdev.org/TLB
    asm volatile ("          \n\
        movl %%cr3, %%eax    \n\
        movl %%eax, %%cr3    \n\
        "
        : : : "eax", "cc"
    );
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
void process_switch_context(int32_t pid) {
    // Switch the current process
    if(pid < 0 || pid >= PROCESS_COUNT) return;
    process_t* process = process_get_pcb(pid);
    if(NULL == process) return;
    active_process_id = pid;
    tss.esp0 = KERNEL_STACK_BASE_ADDR - active_process_id * USER_KMODE_STACK_SIZE - 0x4;
    // 1. set up stack: (top) EIP, CS, EFLAGS, ESP, SS (bottom)
    // 2. set DS to point to the correct entry in GDT for the user mode data segment
    // reference: https://www.felixcloutier.com/x86/IRET:IRETD.html
    asm volatile ("            \n\
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
        : "a"(process->eip), "b"(USER_DS), "c"(USER_CS), "d"(process->esp)
        : "cc"
    );
}

void terminal_switch_active(uint32_t tid) {
    if(tid < 0 || tid >= TERMINAL_COUNT) return;

    process_t* process = process_get_pcb(active_process_id);
    if(NULL != process) {
        asm volatile("movl %%esp, %0":"=r" (process->esp));
        asm volatile("movl %%ebp, %0":"=r" (process->ebp));
        // printf("saved %d, esp %x, ebp %x\n", active_process_id, process->esp, process->ebp);
    }

    cli();
    terminals[active_terminal_id].active_process = active_process_id;

    active_terminal_id = tid;
    active_process_id = terminals[tid].active_process;
    if(active_process_id == -1) {
        sti();
        process_create("shell");
    } else {
        tss.esp0 = KERNEL_STACK_BASE_ADDR - active_process_id * USER_KMODE_STACK_SIZE - 0x4;
        process_switch_paging(active_process_id);
        process_t* process = process_get_pcb(active_process_id);
        if(NULL == process) return;

        // printf("restore %d, esp %x, ebp %x\n", active_process_id, process->esp, process->ebp);
        asm volatile ("         \n\
            movl %0, %%esp      \n\
            movl %1, %%ebp      \n\
            "
            :
            : "r" (process->esp), "r" (process->ebp)
            : "memory"
        );
        sti();
    }
}

void terminal_switch_display(uint32_t tid) {
    if(tid < 0 || tid >= TERMINAL_COUNT) return;

    cli();  // Begin critical section

    char* addr;

    // Copy current terminal content to an alternate location
    addr = (char*) (TERMINAL_ALT_START + (displayed_terminal_id << TB_ADDR_OFFSET));
    memcpy(addr, (char*) TERMINAL_DIRECT_ADDR, TERMINAL_ALT_SIZE);
    terminals[displayed_terminal_id].screen_x = screen_x;
    terminals[displayed_terminal_id].screen_y = screen_y;
    terminals[displayed_terminal_id].keyboard_buffer_top = keyboard_buffer_top;
    terminals[displayed_terminal_id].keyboard_buffer_enable = keyboard_buffer_enable;
    memcpy(terminals[displayed_terminal_id].keyboard_buffer, keyboard_buffer, KEYBOARD_BUFFER_SIZE + 1);

    // Switch displayed terminal id
    process_switch_paging(active_process_id);
    displayed_terminal_id = tid;

    // Copy target terminal content to current display
    addr = (char*) (TERMINAL_ALT_START + (displayed_terminal_id << TB_ADDR_OFFSET));
    memcpy((char*) TERMINAL_DIRECT_ADDR, addr, TERMINAL_ALT_SIZE);
    screen_x = terminals[displayed_terminal_id].screen_x;
    screen_y = terminals[displayed_terminal_id].screen_y;
    keyboard_buffer_top = terminals[displayed_terminal_id].keyboard_buffer_top;
    keyboard_buffer_enable = terminals[displayed_terminal_id].keyboard_buffer_enable;
    memcpy(keyboard_buffer, terminals[displayed_terminal_id].keyboard_buffer, KEYBOARD_BUFFER_SIZE + 1);

    sti();  // End critical section

    // Set cursor position
    vga_text_set_cursor_pos(screen_x, screen_y);

    // terminal_switch_active(tid);
}
