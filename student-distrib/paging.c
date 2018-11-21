/*
 * Source file used to initialize paging
 */

#include "paging.h"

/* void init_paging()
 * @output: page table and page directory initialized.
 * @description: initialize page table and page directory table to utilize
 *                             virtual memory.
 */
void init_paging()
{
    // loop variable
    int index;
    // initialize Page Table
    for (index = 0; index < NUM_PTE; index++)
    {
        // initially, clear all flags
        page_table[index].present
            = ((index == VIDEO_MEM_INDEX) ||
                (index >= VIDEO_MEM_ALT_START && index < VIDEO_MEM_ALT_END) ||
                (index >= SB16_MEM_BEGIN && index < SB16_MEM_END))
                ? 1 : 0;
        page_table[index].read_write = 0;
        page_table[index].user_supervisor = 0;
        page_table[index].write_through = 0;
        page_table[index].cache_disabled = 0;
        page_table[index].accessed = 0;
        page_table[index].dirty = 0;
        page_table[index].pat = 0;
        page_table[index].global = 0;
        page_table[index].avail = 0;
        page_table[index].PB_addr = index;
    }
    // initialize Page Table for User memory mapping
    for (index = 0; index < NUM_PTE; index++)
    {
        // initially, clear all flags
        page_table_usermap[index].present = 0;
        page_table_usermap[index].read_write = 1;
        page_table_usermap[index].user_supervisor = 1;
        page_table_usermap[index].write_through = 0;
        page_table_usermap[index].cache_disabled = 0;
        page_table_usermap[index].accessed = 0;
        page_table_usermap[index].dirty = 0;
        page_table_usermap[index].pat = 0;
        page_table_usermap[index].global = 0;
        page_table_usermap[index].avail = 0;
        page_table_usermap[index].PB_addr = index;
    }
    // initialize the first 4MB memory (4kB page, where video memory is)
    page_directory[0].pde_KB.present = 1;
    page_directory[0].pde_KB.read_write = 0;
    page_directory[0].pde_KB.user_supervisor = 0;
    page_directory[0].pde_KB.write_through = 0;
    page_directory[0].pde_KB.cache_disabled = 0;
    page_directory[0].pde_KB.accessed = 0;
    page_directory[0].pde_KB.reserved = 0;
    page_directory[0].pde_KB.page_size = 0;
    page_directory[0].pde_KB.global = 0;
    page_directory[0].pde_KB.avail = 0;
    page_directory[0].pde_KB.PTB_addr = (unsigned int) page_table >> TB_ADDR_OFFSET;

    // initialize the first 4MB-8MB memory (4MB page, KERNEL)
    page_directory[1].pde_MB.present = 1;
    page_directory[1].pde_MB.read_write = 0;
    page_directory[1].pde_MB.user_supervisor = 0;
    page_directory[1].pde_MB.write_through = 0;
    page_directory[1].pde_MB.cache_disabled = 0;
    page_directory[1].pde_MB.accessed = 0;
    page_directory[1].pde_MB.dirty = 0;
    page_directory[1].pde_MB.page_size = 1;
    page_directory[1].pde_MB.global = 1;
    page_directory[1].pde_MB.avail = 0;
    page_directory[1].pde_MB.pat = 0;
    page_directory[1].pde_MB.reserved = 0;
    page_directory[1].pde_MB.PB_addr = 1;

    // initialize the first 4MB memory (4kB page, where video memory is)
    page_directory[PAGE_TABLE_USERMAP_LOCATION].pde_KB.present = 1;
    page_directory[PAGE_TABLE_USERMAP_LOCATION].pde_KB.read_write = 1;
    page_directory[PAGE_TABLE_USERMAP_LOCATION].pde_KB.user_supervisor = 1;
    page_directory[PAGE_TABLE_USERMAP_LOCATION].pde_KB.write_through = 0;
    page_directory[PAGE_TABLE_USERMAP_LOCATION].pde_KB.cache_disabled = 0;
    page_directory[PAGE_TABLE_USERMAP_LOCATION].pde_KB.accessed = 0;
    page_directory[PAGE_TABLE_USERMAP_LOCATION].pde_KB.reserved = 0;
    page_directory[PAGE_TABLE_USERMAP_LOCATION].pde_KB.page_size = 0;
    page_directory[PAGE_TABLE_USERMAP_LOCATION].pde_KB.global = 0;
    page_directory[PAGE_TABLE_USERMAP_LOCATION].pde_KB.avail = 0;
    page_directory[PAGE_TABLE_USERMAP_LOCATION].pde_KB.PTB_addr = (unsigned int) page_table_usermap >> TB_ADDR_OFFSET;

    // initialize the rest Page Directory
    for (index = 2; index < NUM_PDE; index++)
    {
        if(index == PAGE_TABLE_USERMAP_LOCATION) continue;
        page_directory[index].pde_MB.present = 0;
        page_directory[index].pde_MB.read_write = 0;
        page_directory[index].pde_MB.user_supervisor = 0;
        page_directory[index].pde_MB.write_through = 0;
        page_directory[index].pde_MB.cache_disabled = 0;
        page_directory[index].pde_MB.accessed = 0;
        page_directory[index].pde_MB.dirty = 0;
        page_directory[index].pde_MB.page_size = 0;
        page_directory[index].pde_MB.global = 0;
        page_directory[index].pde_MB.avail = 0;
        page_directory[index].pde_MB.pat = 0;
        page_directory[index].pde_MB.reserved = 0;
        page_directory[index].pde_MB.PB_addr = index;
    }

    // note: there might be some problems
    asm (
	"movl $page_directory, %%eax            ;"
	"andl $0xFFFFFC00, %%eax                    ;"
	"movl %%eax, %%cr3                                ;"
	"movl %%cr4, %%eax                                ;"
	"orl $0x00000010, %%eax                     ;"
	"movl %%eax, %%cr4                                ;"
	"movl %%cr0, %%eax                                ;"
	"orl $0x80000000, %%eax 	            ;"
	"movl %%eax, %%cr0                                 "
	: : : "eax", "cc" );

    return;
}
