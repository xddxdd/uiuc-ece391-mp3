/*
 * Header file used to initialize paging
 */

#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"
#include "x86_desc.h"

// Some constants
// Table address offset
#define TB_addr_offset    12

// function used to initial paging
void init_paging();

#endif
