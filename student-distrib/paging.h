/*
 * Header file used to initialize paging
 */

#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"
#include "x86_desc.h"
#include "lib.h"

// Some constants
// Table address offset
#define TB_addr_offset    12
#define VIDEO_MEM_INDEX 184  // 0xB8000/(4*1024), VIDEO/4KB

// function used to initial paging
void init_paging();

#endif
