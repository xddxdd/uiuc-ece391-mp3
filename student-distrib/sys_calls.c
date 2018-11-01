#include "sys_calls.h"

// System calls for checkpoint 3.
int32_t halt (uint8_t status){
    return SYSCALL_SUCCESS;
}

int32_t execute (const uint8_t* command){
    ////////////////////////////// Parsing //////////////////////////////
    uint8_t* filename;
    // set memory space for filename
    memset(filename, 0, strlen(command));
    // get filename
    _execute_parse(command, filename);
    ///////////////////////// Executable check //////////////////////////
    if (SYSCALL_SUCCESS != _execute_exe_check(filename))
    {
        return SYSCALL_FAIL;
    }
    ////////////////////////////// Paging //////////////////////////////


    return SYSCALL_SUCCESS;
}

int32_t read (int32_t fd, void* buf, int32_t nbytes){

    return SYSCALL_SUCCESS;
}

int32_t write (int32_t fd, const void* buf, int32_t nbytes){

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

// Helper functions for sytstem calls
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
 *                       value. Its size should be no smaller than strlen(command).
 *    @output: none
 *    @note: need to add support for arguments parsing
 */
void _execute_parse (const uint8_t* command, uint8_t* filename)
{
    // loop variable
    int index;
    // parsing
    for (index = 0; index < strlen(command); index++)
    {
        // end of filename
        if (command[index] == SPACE)
        {
            break;
        }
    // read filename to the filename buffer
        else
        {
            filename[index] = command[index];
        }
    }
    return;
}

/*
 * int32_t _execute_exe_check(const uint8_t* filename)
 *    @description: helper function called by sytstem call execute()
 *                  to check if the file is executable or not
 *    @input: filename - the input filename
 *    @output: SYSCALL_FAIL if fail, and SYSCALL_SUCCESS if succeed
 */
int32_t _execute_exe_check (const uint8_t* filename)
{
    // check if the file exist
    ece391fs_file_info_t finfo;
    if(ECE391FS_CALL_FAIL == read_dentry_by_name(filename, &finfo)) return SYSCALL_FAIL;
    // read the header of the file
	char buf[FILE_HEADER_LEN];
	if(FILE_HEADER_LEN != read_data(finfo.inode, 0, buf, FILE_HEADER_LEN)) return SYSCALL_FAIL;
    // check if the file is executable
    if ((buf[0] != FILE_EXE_HEADER_0) |
        (buf[1] != FILE_EXE_HEADER_1) |
        (buf[2] != FILE_EXE_HEADER_2) |
        (buf[3] != FILE_EXE_HEADER_3))
        {
            return SYSCALL_FAIL;
        }
    return SYSCALL_SUCCESS;
}

int32_t _execute_paging (const uint8_t* filename)
{
    // get the entry in page directory for the input process
    uint32_t PD_index = (uint32_t) USER_PROCESS >> TD_ADDR_OFFSET;
    page_directory[PD_index].pde_MB.present = 1;
    page_directory[PD_index].pde_MB.read_write = 0;
    page_directory[PD_index].pde_MB.user_supervisor = 0;
    page_directory[PD_index].pde_MB.write_through = 0;
    page_directory[PD_index].pde_MB.cache_disabled = 0;
    page_directory[PD_index].pde_MB.accessed = 0;
    page_directory[PD_index].pde_MB.dirty = 0;
    page_directory[PD_index].pde_MB.page_size = 0;
    page_directory[PD_index].pde_MB.global = 0;
    page_directory[PD_index].pde_MB.avail = 0;
    page_directory[PD_index].pde_MB.pat = 0;
    page_directory[PD_index].pde_MB.reserved = 0;
    page_directory[PD_index].pde_MB.PB_addr = 2;
    return 0;
}
