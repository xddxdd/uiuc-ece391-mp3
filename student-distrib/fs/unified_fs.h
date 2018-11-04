#ifndef _UNIFIED_FS_H_
#define _UNIFIED_FS_H_

#include "../lib.h"

typedef struct {
    int32_t (*open)(int32_t*, char*);
    int32_t (*read)(int32_t*, uint32_t*, char*, uint32_t);
    int32_t (*write)(int32_t*, uint32_t*, const char*, uint32_t);
    int32_t (*close)(int32_t*);
} unified_fs_interface_t;

typedef struct {
    unified_fs_interface_t* interface;
    int32_t inode;
    uint32_t pos;
    uint32_t flags;
} fd_array_t;

#define MAX_OPEN_FILES 8

#define UNIFIED_FS_SUCCESS 0
#define UNIFIED_FS_FAIL -1

#define FD_STDIN 0
#define FD_STDOUT 1

int32_t unified_init(fd_array_t* fd_array);
int32_t unified_open(fd_array_t* fd_array, const char* filename);
int32_t unified_read(fd_array_t* fd_array, int32_t fd, void* buf, int32_t nbytes);
int32_t unified_write(fd_array_t* fd_array, int32_t fd, const void* buf, int32_t nbytes);
int32_t unified_close(fd_array_t* fd_array, int32_t fd);

#endif
