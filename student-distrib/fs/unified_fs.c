#include "unified_fs.h"
#include "ece391fs.h"
#include "../devices/keyboard.h"
#include "../devices/rtc.h"
#include "../devices/tux.h"

int32_t unified_init(fd_array_t* fd_array) {
    // Initialize array to prevent page fault
    int i = 0;
    for(i = 0; i < MAX_OPEN_FILES; i++) {
        fd_array[i].interface = NULL;
        fd_array[i].inode = 0;
        fd_array[i].pos = 0;
        fd_array[i].flags = 0;
    }
    fd_array[0].interface = &terminal_stdin_if;
    fd_array[1].interface = &terminal_stdout_if;
    return UNIFIED_FS_SUCCESS;
}

int32_t unified_open(fd_array_t* fd_array, const char* filename) {
    // Try to allocate index in file descriptor array
    int fd = 0;
    while(fd < MAX_OPEN_FILES && fd_array[fd].interface != NULL) fd++;
    if(fd >= MAX_OPEN_FILES) return UNIFIED_FS_FAIL;

    ece391fs_file_info_t finfo;
    if(0 == strncmp("tux", filename, 4)) {
        // Trying to open Tux Controller
        fd_array[fd].interface = &tux_if;
    } else if(0 == strncmp("stdin", filename, 6)) {
        // Trying to open Tux Controller
        fd_array[fd].interface = &terminal_stdin_if;
    } else if(0 == strncmp("stdout", filename, 7)) {
        // Trying to open Tux Controller
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

int32_t unified_read(fd_array_t* fd_array, int32_t fd, void* buf, int32_t nbytes) {
    if(fd < 0 || fd >= MAX_OPEN_FILES) return UNIFIED_FS_FAIL;
    if(fd_array[fd].interface == NULL) return UNIFIED_FS_FAIL;
    if(NULL == fd_array[fd].interface->read) return UNIFIED_FS_FAIL;
    return (*fd_array[fd].interface->read) (&fd_array[fd].inode, &fd_array[fd].pos, (char*) buf, nbytes);
}

int32_t unified_write(fd_array_t* fd_array, int32_t fd, const void* buf, int32_t nbytes) {
    if(fd < 0 || fd >= MAX_OPEN_FILES) return UNIFIED_FS_FAIL;
    if(fd_array[fd].interface == NULL) return UNIFIED_FS_FAIL;
    if(NULL == fd_array[fd].interface->write) return UNIFIED_FS_FAIL;
    return (*fd_array[fd].interface->write) (&fd_array[fd].inode, &fd_array[fd].pos, (const char*) buf, nbytes);
}

int32_t unified_close(fd_array_t* fd_array, int32_t fd) {
    if(fd < 0 || fd >= MAX_OPEN_FILES) return UNIFIED_FS_FAIL;
    if(fd_array[fd].interface == NULL) return UNIFIED_FS_FAIL;
    if(NULL == fd_array[fd].interface->close) return UNIFIED_FS_FAIL;
    if(UNIFIED_FS_FAIL == (*fd_array[fd].interface->close) (&fd_array[fd].inode)) return UNIFIED_FS_FAIL;
    fd_array[fd].interface = NULL;
    return UNIFIED_FS_SUCCESS;
}
