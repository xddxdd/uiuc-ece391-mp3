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
#define TB_ADDR_OFFSET_MB 22
#define VIDEO_MEM_INDEX   0xb8      // 0xb8000 >> 12
#define VIDEO_MEM_DIRECT_INDEX 0xb7 // Index used to directly access video mem

// Alternate video memory locations for multiple terminals.
#define VIDEO_MEM_ALT_START 0xb9
#define VIDEO_MEM_ALT_END 0xbc

// Memory space for Sound Blaster 16. Takes 64KB space.
#define SB16_MEM_BEGIN 0x10 // 0x10000 >> 12
#define SB16_MEM_END 0x20   // 0x20000 >> 12

// Memory space for searching ACPI data. Takes 128KB space.
#define ACPI_MEM_BEGIN 0xe0 // 0xe0000 >> 12
#define ACPI_MEM_END 0x100  // 0x100000 >> 12

#define PAGE_TABLE_USERMAP_LOCATION 33  // 132-136M

// Memory space for QEMU VGA device, Takes 64KB space.
#define QEMU_VGA_MEM_BEGIN 0xa0
#define QEMU_VGA_MEM_END 0xb0

// function used to initial paging
void init_paging();

#endif
