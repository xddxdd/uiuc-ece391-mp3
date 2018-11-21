/*
 * Header file used to initialize paging
 */

#ifndef _PAGING_H
#define _PAGING_H

#include "lib/types.h"
#include "x86_desc.h"
#include "lib/lib.h"

// Some constants
// Table address offset
#define TB_ADDR_OFFSET    12
#define VIDEO_MEM_INDEX   0xb8 // 0xb8000 >> 12

// Alternate video memory locations for multiple terminals.
#define VIDEO_MEM_ALT_START 0xb9
#define VIDEO_MEM_ALT_END 0xbc

// Memory space for Sound Blaster 16. Takes 64KB space.
#define SB16_MEM_BEGIN 0x10 // 0x10000 >> 12
#define SB16_MEM_END 0x20   // 0x20000 >> 12

#define PAGE_TABLE_USERMAP_LOCATION 33  // 132-136M

// function used to initial paging
void init_paging();

#endif
