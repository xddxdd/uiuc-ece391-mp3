#include "multiprocessing.h"
#include "../devices/keyboard.h"
#include "../devices/qemu_vga.h"
#include "../data/uiuc.h"
#include "../lib/status_bar.h"

char program_header[PROGRAM_HEADER_LEN] = {0x7f, 0x45, 0x4c, 0x46};

volatile terminal_t terminals[TERMINAL_COUNT];

int32_t displayed_terminal_id = 0;
int32_t active_terminal_id = 0;
int32_t active_process_id = -1;

/* process_t* process_get_active_pcb()
 * @output: returns the PCB of currently active process.
 * @description: as stated above.
 */
process_t* process_get_active_pcb() {
    return process_get_pcb(active_process_id);
}

/* process_t* process_get_pcb(int32_t pid)
 * @input: pid - the id of process we want the pcb for.
 * @output: returns the PCB of process with given PID.
 * @description: as stated above.
 */
process_t* process_get_pcb(int32_t pid) {
    if(pid < 0 || pid >= PROCESS_COUNT) return NULL;
    return (process_t*) (KERNEL_STACK_BASE_ADDR - (pid + 1) * USER_KMODE_STACK_SIZE);
}

/* void process_init()
 * @output: multiprocessing structures get initialized
 * @description: as stated above.
 */
void process_init() {
    int i;
    for(i = 0; i < PROCESS_COUNT; i++) {
        // There's no process running initially
        process_t* process = process_get_pcb(i);
        process->present = 0;
    }
    for(i = 0; i < TERMINAL_COUNT; i++) {
        // There's nothing on any terminal screen, no process running
        terminals[i].active_process = -1;
        terminals[i].screen_x = 0;
        terminals[i].screen_y = 0;

        // Clear keyboard buffer
        terminals[i].keyboard_buffer_top = 0;
        terminals[i].keyboard_buffer_enable = 0;
        memset(terminals[i].keyboard_buffer, 0, KEYBOARD_BUFFER_SIZE + 1);
        int j;
        for(j = 0; j < TERMINAL_ALT_SIZE; j++) {
            if(1 == j % 2) {
                *((char*) TERMINAL_ALT_START + TERMINAL_ALT_SIZE * i + j) = ATTRIB;
            } else {
                *((char*) TERMINAL_ALT_START + TERMINAL_ALT_SIZE * i + j) = ' ';
            }
        }

        // Clear Chinese input buffer
        terminals[i].chinese_input_buf.enabled = 0;
        terminals[i].chinese_input_buf.buf_len = 0;
        terminals[i].chinese_input_buf.pos = 0;
        terminals[i].chinese_input_buf.len = 0;
        terminals[i].chinese_input_buf.page = 0;
        memset(terminals[i].chinese_input_buf.buf, 0, CHINESE_INPUT_BUF_LEN + 1);
    }
}

/* int32_t process_allocate()
 * @output: a pid if can be allocated, otherwise -1 (FAIL).
 * @description: find a free PID and reserves it for process creation.
 */
int32_t process_allocate() {
    int i;
    for(i = 0; i < PROCESS_COUNT; i++) {
        process_t* process = process_get_pcb(i);
        if(0 == process->present) {
            process->present = 1;
            sti();
            return i;
        }
    }
    return -1;
}

/* int32_t process_create(const char* command)
 * @input: command - command entered via shell
 * @output: current terminal switches to the command called,
 *          or returns FAIL when the command is invalid
 * @description: actual code for the execute system call,
 *     creates a new process with given parameter and switches to it.
 */
int32_t process_create(const char* command) {
    if(NULL == command) return FAIL;
    if(command[0] == STRING_END) return FAIL;

    // initialize filename buffer
    uint8_t filename[ECE391FS_MAX_FILENAME_LEN + 1];
    memset(filename, 0, ECE391FS_MAX_FILENAME_LEN + 1);
    uint8_t argument[MAX_ARG_LENGTH + 1];
    memset(argument, 0, MAX_ARG_LENGTH + 1);

    // get filename and argument
    int space_begin = 0;
    while(command[space_begin] == SPACE) space_begin++;
    int space_separate = space_begin;
    while(command[space_separate] != STRING_END && command[space_separate] != SPACE) space_separate++;
    int space_separate_end = space_separate;
    while(command[space_separate_end] == SPACE) space_separate_end++;
    int space_end = space_separate_end;
    if(space_separate != space_separate_end) {
        while(command[space_end] != STRING_END) space_end++;
        while(command[space_end - 1] == SPACE) space_end--;
    }

    if(space_separate - space_begin > ECE391FS_MAX_FILENAME_LEN) return FAIL;

    memcpy(filename, (char*) (command + space_begin), space_separate - space_begin);
    memcpy(argument, (char*) (command + space_separate_end), space_end - space_separate_end);
    // printf("cmd: \"%s\"\narg: \"%s\"\n", filename, argument);

    // Try to allocate PID for new process
    int32_t pid = process_allocate();
    if(-1 == pid) return FAIL;
    process_t* process = process_get_pcb(pid);
    if(FAIL == unified_init(process->fd_array)) return FAIL;

    // Check if file is a program
    char buf[FILE_HEADER_LEN];
    int32_t fd;
    if((FAIL == (fd = unified_open(process->fd_array, (char*) filename)))
        || (FAIL == unified_read(process->fd_array, fd, buf, PROGRAM_HEADER_LEN))
        || (process->fd_array[fd].pos != PROGRAM_HEADER_LEN)
        || (0 != strncmp((char*) buf, program_header, PROGRAM_HEADER_LEN))
        || (FAIL == unified_close(process->fd_array, fd))) {
        process->present = 0;
        return FAIL;
    }

    // If this is a terminal starting, display splash later
    uint8_t display_splash_later = terminals[active_terminal_id].active_process == -1;

    // Create process structure
    process->parent_pid = active_process_id;
    process->esp = USER_STACK_ADDR;
    process->ebp = USER_STACK_ADDR;
    process->terminal = active_terminal_id;
    terminals[active_terminal_id].active_process = pid;
    process->vidmap = 0;
    memcpy(process->cmd, filename, MAX_ARG_LENGTH + 1);
    memcpy(process->arg, argument, MAX_ARG_LENGTH + 1);

    // Change paging configuration, load program
    process_switch_paging(pid);
    if(FAIL == (fd = unified_open(process->fd_array, (char*) filename))) return FAIL;
    if(FAIL == unified_read(process->fd_array, fd, (char*) USER_PROCESS_ADDR, PROGRAM_MAX_LEN)) return FAIL;
    if(FAIL == unified_close(process->fd_array, fd)) return FAIL;
    process->eip = (*(uint32_t*) (USER_PROCESS_ADDR + 24));

    // Patch program
    executable_patching((char*) filename);

    // Save the kernel stack of current process
    // Must be done directly in this function, or we'll screw up the kernel stack
    //   and get a page fault when attempting to return to this process
    process_t* ori_process = process_get_pcb(active_process_id);
    if(NULL != ori_process) {
        asm volatile("movl %%esp, %0":"=r" (ori_process->esp));
        asm volatile("movl %%ebp, %0":"=r" (ori_process->ebp));
    } else if(display_splash_later && qemu_vga_enabled) {
        // This is a terminal starting, show the splash image
        qemu_vga_show_picture(UIUC_IMAGE_WIDTH, UIUC_IMAGE_HEIGHT, 16, (uint8_t*) UIUC_IMAGE_DATA);
        // and a line indicating Chinese capability
        puts("本系统支持中文显示和输入 Chinese display & input supported\n");
    }

    // Switch over to the new process
    process_switch_context(pid);
    // Nobody but GCC cares
    return SUCCESS;
}

/* int32_t process_halt(uint8_t status)
 * @input: status - return code of the process.
 * @output: system switch to process's parent, if there's any,
 *          or recreate a shell if the process has no parent
 * @description: return to the process's parent when a process
 *     is done with its work. Can be called from the process itself
 *     using system call, or can be called elsewhere (like interrupt handler)
 *     to implement Ctrl+C, restart on exception, etc.
 */
int32_t process_halt(uint8_t status) {
    // extract current pcb
    process_t* process = process_get_pcb(active_process_id);
    if(NULL == process) return FAIL;

    // Close all files on process halt, as required by gradesheet
    fd_array_t* fd_array = process->fd_array;
    int i;
    for(i = 0; i < MAX_NUM_FD_ENTRY; i++) {
        unified_close(fd_array, i);
    }

    if(-1 == process->parent_pid) {
        // This process is shell, need to be restarted
        // Remove this process from the terminal, making terminal empty
        process->present = 0;
        active_process_id = -1;
        // Don't release the terminal, or splash image will show again
        // !!! REVERT THIS CHANGE IF SYSTEM IS BUGGY !!!
        // terminals[active_terminal_id].active_process = -1;

        // Create a new shell process.
        // This call MUST BE directly in the halt call, not wrapped in subroutine
        //     otherwise the structure of the kernel stack will be WRONG and system
        //     will PAGE FAULT
        process_create("shell");
    } else {
        // Prepare kernel stack of process's parent
        int32_t parent = process->parent_pid;
        process->present = 0;
        tss.esp0 = KERNEL_STACK_BASE_ADDR - parent * USER_KMODE_STACK_SIZE - 0x4;
        // Regenerate paging configuration with stored params of parent
        process_switch_paging(parent);
        process = process_get_pcb(parent);
        if(NULL == process) return FAIL;
        // Make parent proces active
        active_process_id = parent;
        terminals[active_terminal_id].active_process = parent;
        // Switch kernel stack to parent process
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
        // Simply do a return.
        // The parent's stack here is preserved by execute call,
        //     so this is equivalent to returning from the execute call.
        // EAX stores return value of execute call, which is the status code.
    }
    // Nobody but GCC cares
    return SUCCESS;
}

/* void process_switch_paging(int32_t pid)
 * @input: pid - pid of process we're setting up paging for
 * @output: paging configuration updated for process #pid
 * @description: updates the PDE and PTE for execution of a process,
 *     and applies the changes.
 */
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
    // first process gets 8-12M, second 12-16M, etc.
    page_directory[PD_index].pde_MB.PB_addr = PROCESS_PYSC_BASE_ADDR + pid;

    // Redirect video mem R/W to main display / alt display based on terminal ids
    if(active_terminal_id == displayed_terminal_id) {
        // This process is displayed on screen, let it directly operate on video mem
        page_table[VIDEO_MEM_INDEX].PB_addr = VIDEO_MEM_INDEX;
        page_table_usermap[VIDEO_MEM_INDEX].PB_addr = VIDEO_MEM_INDEX;
    } else {
        // This process is hidden in background, redirect it elsewhere
        page_table[VIDEO_MEM_INDEX].PB_addr = VIDEO_MEM_ALT_START + process->terminal;
        page_table_usermap[VIDEO_MEM_INDEX].PB_addr = VIDEO_MEM_ALT_START + process->terminal;
    }

    // Enable video memory map to userspace only when process asked to do so
    // Other elements of this table is initialized in paging.c
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

/* void process_switch_context(int32_t pid)
 * @input: pid - pid of process we're switching to
 * @output: system start to execute another process
 * @description: creates an artificial IRET stack and let the system run another process
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
        popl    %%edx          \n\
        orl     $0x0200, %%edx \n\
        pushl   %%edx          \n\
        pushl   %%ecx          \n\
        pushl   %%eax          \n\
        iret                   \n\
        "
        :
        : "a"(process->eip), "b"(USER_DS), "c"(USER_CS), "d"(process->esp)
        : "memory"
    );
}

/* void terminal_switch_active(uint32_t tid)
 * @input: tid - id of terminal we're switching to
 * @output: the process running on terminal #tid is put to active running
 * @description: switch to process running on terminal #tid, to implement
 *     multiprocessing with background process scheduling.
 *   Must be wrapped in CLI/STI.
 */
void terminal_switch_active(uint32_t tid) {
    if(tid < 0 || tid >= TERMINAL_COUNT) return;

    // Save the kernel stack of current process
    process_t* process = process_get_pcb(active_process_id);
    if(NULL != process) {
        asm volatile("movl %%esp, %0":"=r" (process->esp));
        asm volatile("movl %%ebp, %0":"=r" (process->ebp));
        // printf("saved %d, esp %x, ebp %x\n", active_process_id, process->esp, process->ebp);
    }

    terminals[active_terminal_id].active_process = active_process_id;
    // Switch to another terminal and corresponding process
    active_terminal_id = tid;
    active_process_id = terminals[tid].active_process;
    if(active_process_id == -1) {
        // If nothing is running there, create a shell
        process_create("shell");
    } else {
        // Switch to that process, just as done in process_halt, except the status part
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
    }
    // If a Ctrl+C is scheduled but not yet done, kill the current process
    if(ctrl_c_pending && (active_terminal_id == displayed_terminal_id)) {
        ctrl_c_pending = 0;
        syscall_halt(255);
    }
}

/* void terminal_switch_display(uint32_t tid)
 * @input: tid - id of terminal we're switching display to
 * @output: displayed terminal switches to #tid
 * @description: changes the terminal displayed on screen
 */
void terminal_switch_display(uint32_t tid) {
    if(tid < 0 || tid >= TERMINAL_COUNT) return;

    cli();  // Begin critical section

    char* addr;

    // Copy current terminal content to an alternate location
    addr = (char*) (TERMINAL_ALT_START + (displayed_terminal_id << TB_ADDR_OFFSET));
    memcpy(addr, (char*) TERMINAL_DIRECT_ADDR, TERMINAL_ALT_SIZE);

    // Switch displayed terminal id
    displayed_terminal_id = tid;
    process_switch_paging(active_process_id);

    // Copy target terminal content to current display
    addr = (char*) (TERMINAL_ALT_START + (displayed_terminal_id << TB_ADDR_OFFSET));
    memcpy((char*) TERMINAL_DIRECT_ADDR, addr, TERMINAL_ALT_SIZE);

    // Set cursor position
    int32_t tmp = active_terminal_id;
    active_terminal_id = displayed_terminal_id;
    vga_text_set_cursor_pos(terminals[displayed_terminal_id].screen_x, terminals[displayed_terminal_id].screen_y);
    active_terminal_id = tmp;

    qemu_vga_switch_terminal(displayed_terminal_id);
    ONTO_DISPLAY_WRAP(status_bar_switch_terminal(displayed_terminal_id));

    sti();  // End critical section
}

/* void executable_patching(const char* process)
 * @input: process - process name to be patched
 * @output: some code in process gets replaced
 * @description: patches process on execution, now used to replace shell prompt.
 */
void executable_patching(const char* process) {
    if(NULL == process) return;
    int pos, i;
    if(0 == strncmp(process, "shell", 6)) {
        // Change shell prompt from 391OS to nulsh
        {
            char needle[]  = "391OS> ";
            char replace[] = "nulsh> ";
            int needle_len = 7;
            for(pos = USER_PROCESS_ADDR; pos < USER_PROCESS_ADDR + PROGRAM_MAX_LEN - needle_len; pos++) {
                if(0 == strncmp((char*) pos, needle, needle_len)) {
                    for(i = 0; i < needle_len; i++) {
                        *(char*) (pos + i) = replace[i];
                    }
                    break;
                }
            }
        }
        // Change shell start prompt to refer to nullOS
        {
            char needle[]  = "Starting 391 Shell";
            char replace[] = "load nullOS shell ";
            int needle_len = 18;
            for(pos = USER_PROCESS_ADDR; pos < USER_PROCESS_ADDR + PROGRAM_MAX_LEN - needle_len; pos++) {
                if(0 == strncmp((char*) pos, needle, needle_len)) {
                    for(i = 0; i < needle_len; i++) {
                        *(char*) (pos + i) = replace[i];
                    }
                    break;
                }
            }
        }
    }
}
