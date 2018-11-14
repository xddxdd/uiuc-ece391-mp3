#include "unified_fs.h"
#include "ece391fs.h"
#include "../devices/keyboard.h"
#include "../devices/rtc.h"
#include "../devices/tux.h"

/* int32_t unified_init(fd_array_t* fd_array)
 * @input: fd_array - pointer to a file descriptor array
 * @output: ret val - SUCCESS / FAIL
 *          fd_array - initialized with stdin and stdout created
 * @description: Initialize file descriptor array.
 */
int32_t unified_init(fd_array_t* fd_array) {
    if(fd_array == NULL) return UNIFIED_FS_FAIL;
    // Initialize array to prevent page fault
    int i = 0;
    for(i = 0; i < MAX_OPEN_FILES; i++) {
        fd_array[i].interface = NULL;
        fd_array[i].inode = 0;
        fd_array[i].pos = 0;
        fd_array[i].flags = 0;
    }
    fd_array[FD_STDIN].interface = &terminal_stdin_if;
    fd_array[FD_STDOUT].interface = &terminal_stdout_if;
    return UNIFIED_FS_SUCCESS;
}

/* int32_t unified_open(fd_array_t* fd_array, const char* filename)
 * @input: fd_array - file descriptor array
 *         filename - file to be opened
 * @output: ret val - FAIL / file descriptor id
 *          fd_array[fd] - FS interface set, pos set to 0
 * @description: Uses correct FS driver to open a file, generate a file descriptor,
 *     write the descriptor into fd_array, and returns the descriptor.
 *   If filename is "tux", it will open the Tux Controller.
 *   If filename is "stdin", it will open STDIN (typically not needed).
 *   If filename is "stdout", it will open STDOUT (typically not needed).
 *   Otherwise it depends on file type in ECE391FS, file/folder/RTC.
 */
int32_t unified_open(fd_array_t* fd_array, const char* filename) {
    if(NULL == fd_array) return UNIFIED_FS_FAIL;

    // Try to allocate index in file descriptor array
    int fd = 0;
    while(fd < MAX_OPEN_FILES && fd_array[fd].interface != NULL) fd++;
    if(fd >= MAX_OPEN_FILES) return UNIFIED_FS_FAIL;

    ece391fs_file_info_t finfo;
    if(NULL == filename) {
        return UNIFIED_FS_FAIL;
    } else if(0 == strlen(filename)) {
        return UNIFIED_FS_FAIL;
    } else if(0 == strncmp("tux", filename, 4)) {
        // Trying to open Tux Controller
        fd_array[fd].interface = &tux_if;
    } else if(0 == strncmp("stdin", filename, 6)) {
        // Trying to open STDIN
        fd_array[fd].interface = &terminal_stdin_if;
    } else if(0 == strncmp("stdout", filename, 7)) {
        // Trying to open STDOUT
        fd_array[fd].interface = &terminal_stdout_if;
    } else if(ECE391FS_CALL_SUCCESS == read_dentry_by_name((char*) filename, &finfo)) {
        // File exists in ECE391FS
        switch(finfo.type) {
            case ECE391FS_FILE_TYPE_FILE:
                fd_array[fd].interface = &ece391fs_file_if;
                break;
            case ECE391FS_FILE_TYPE_FOLDER:
                fd_array[fd].interface = &ece391fs_dir_if;
                break;
            case ECE391FS_FILE_TYPE_RTC:
                fd_array[fd].interface = &rtc_if;
        }
    } else {
        return UNIFIED_FS_FAIL;
    }
    if(NULL == fd_array[fd].interface->open) return UNIFIED_FS_FAIL;
    if(UNIFIED_FS_FAIL == (*fd_array[fd].interface->open) (&fd_array[fd].inode, (char*) filename)) {
        return UNIFIED_FS_FAIL;
    }
    fd_array[fd].pos = 0;
    return fd;
}

/* int32_t unified_read(fd_array_t* fd_array, int32_t fd, void* buf, int32_t nbytes)
 * @input: fd_array - file descriptor array
 *         fd - file descriptor id
 *         buf - buffer to be written with file content
 *         nbytes - number of bytes to be read from file
 * @output: ret val - SUCCESS / FAIL
 *          buf - written with file content
 * @description: Unified read function, calls corresponding FS function to do the actual reading.
 */
int32_t unified_read(fd_array_t* fd_array, int32_t fd, void* buf, int32_t nbytes) {
    if(NULL == fd_array) return UNIFIED_FS_FAIL;
    if(fd < 0 || fd >= MAX_OPEN_FILES) return UNIFIED_FS_FAIL;
    if(fd_array[fd].interface == NULL) return UNIFIED_FS_FAIL;
    if(NULL == fd_array[fd].interface->read) return UNIFIED_FS_FAIL;
    return (*fd_array[fd].interface->read) (&fd_array[fd].inode, &fd_array[fd].pos, (char*) buf, nbytes);
}

/* int32_t unified_write(fd_array_t* fd_array, int32_t fd, void* buf, int32_t nbytes)
 * @input: fd_array - file descriptor array
 *         fd - file descriptor id
 *         buf - buffer to be read from for data to be written
 *         nbytes - number of bytes to be write to file
 * @output: ret val - SUCCESS / FAIL
 *          file written with data from buf
 * @description: Unified write function, calls corresponding FS function to do the actual writing.
 */
int32_t unified_write(fd_array_t* fd_array, int32_t fd, const void* buf, int32_t nbytes) {
    if(NULL == fd_array) return UNIFIED_FS_FAIL;
    if(fd < 0 || fd >= MAX_OPEN_FILES) return UNIFIED_FS_FAIL;
    if(fd_array[fd].interface == NULL) return UNIFIED_FS_FAIL;
    if(NULL == fd_array[fd].interface->write) return UNIFIED_FS_FAIL;
    return (*fd_array[fd].interface->write) (&fd_array[fd].inode, &fd_array[fd].pos, (const char*) buf, nbytes);
}

/* int32_t unified_close(fd_array_t* fd_array, int32_t fd)
 * @input: fd_array - file descriptor array
 *         fd - file descriptor id
 * @output: fd_array[fd] interface removed
 * @description: Closes a file.
 */
int32_t unified_close(fd_array_t* fd_array, int32_t fd) {
    if(NULL == fd_array) return UNIFIED_FS_FAIL;
    if(fd < 0 || fd >= MAX_OPEN_FILES) return UNIFIED_FS_FAIL;
    if(fd_array[fd].interface == NULL) return UNIFIED_FS_FAIL;
    if(NULL == fd_array[fd].interface->close) return UNIFIED_FS_FAIL;
    if(UNIFIED_FS_FAIL == (*fd_array[fd].interface->close) (&fd_array[fd].inode)) return UNIFIED_FS_FAIL;
    fd_array[fd].interface = NULL;
    return UNIFIED_FS_SUCCESS;
}
