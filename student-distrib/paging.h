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
#define TB_ADDR_OFFSET    12
#define VIDEO_MEM_INDEX 184  // 0xB8000/(4*1024), VIDEO/4KB

// Memory space for Sound Blaster 16. Takes 64KB space.
#define SB16_MEM_BEGIN 0x10 // 0x10000 >> 12
#define SB16_MEM_END 0x20   // 0x20000 >> 12

// function used to initial paging
void init_paging();

#endif
