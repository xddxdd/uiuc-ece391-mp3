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
#define VIDEO_MEM_INDEX   0xB8      // 0xB8000 >> 12

// Alternate memory position to store video info
#define VIDEO_ALT_MEM_INDEX 0xB9    // 0xB9000 >> 12

// Memory space for Sound Blaster 16. Takes 64KB space.
#define SB16_MEM_BEGIN 0x10 // 0x10000 >> 12
#define SB16_MEM_END 0x20   // 0x20000 >> 12

// Memory space for QEMU VGA device, Takes 64KB space.
#define QEMU_VGA_MEM_BEGIN 0xa0
#define QEMU_VGA_MEM_END 0xb0

// function used to initial paging
void init_paging();

#endif
