#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"	// Added by jinghua3.
#include "ece391fs.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* Division by Zero Test - Added by jinghua3.
 *
 * Test exception handler division by zero.
 * Inputs: None
 * Outputs: None
 * Side Effects: freeze kernel.
 * Coverage: IDT Exception handler.
 * Files: idt.c
 */
void division_by_zero_test(){
	TEST_HEADER;

	int testVar;
	int one = 1;
	int zero = 0;
	testVar = one/zero;
}



/* Dereferencing NULL Test - Added by jinghua3.
 *
 * Attempt to dereference NULL, should trigger page fault.
 * Inputs: none
 * Outputs: none
 * Side Effects: freeze kernel.
 * Coverage: Exception hander.
 * Files: idt.c
 */
void dereferencing_null_test(){
	TEST_HEADER;

	int* ptr = NULL;
	int testVar;
	testVar = *(ptr);
}

/* Dereferencing in Non-exist Page Test - Added by jinghua3.
 *
 * Attempt to dereference an address in a nonexistent page, should trigger page fault.
 * For checkpoint 1, linear addr larger than 8MB should not be present.
 * So trying to dereference a linear addr larger than 0x800000 should trigger page fault.
 * Inputs: none
 * Outputs: none
 * Side Effects: freeze kernel.
 * Coverage: Exception hander.
 * Files: idt.c
 */
void deref_nonexist_page_test(){
	TEST_HEADER;

	int* ptr = (int*)(0x800000 + 8);
	int testVar;
	testVar = *(ptr);
}


/* Video Memory Paging Test - Added by jinghua3.
 *
 * Dereferencing linear address in range for video memory,
 * particularly just choose one address randomly, 0xB8000 + 8.
 * (video mem starts at 0xB8000)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging in range of Video Memory.
 * Files: paging.c
 */
int videoMem_paging_test(){
	TEST_HEADER;

	int * ptr = (int*)(0xB8000 + 8);
	int testVar;
	testVar = *ptr;
	return PASS;
}

/* Kernel Memory Paging Test - Added by jinghua3.
 *
 * Dereferencing linear address in range for kernel,
 * particularly just choose one address randomly, 0x400000 + 8.
 * (kernel addresses start at 0x400000)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging in range of Kernel Memory.
 * Files: paging.c
 */
int kerMem_paging_test(){
	TEST_HEADER;

	int * ptr = (int*)(0x400000 + 8);
	int testVar;
	testVar = *ptr;
	return PASS;
}


/* Paging Structure Test - Added by jinghua3.
 *
 * Test whether the page table has been initialized
 * Input: none
 * Output: PASS/FAIL
 * Side Effects: none
 * Coverage: paging, page directory and page table.
 * Files: paging.c
 */
int paging_struct_test(){
	TEST_HEADER;

	// test if any of the first 2 entries in page directory is null.
	if(page_directory[0].pde_KB.present!=1 || page_directory[1].pde_MB.present!=1){
		printf("\n page directory first two entries not all present. \n");
		return FAIL;
	}
	// test some page table entries in the first page table, just randomly choose indices 1,20,1000.
	if(page_table[1].present!=1 || page_table[20].present!=1 || page_table[1000].present!=1){
		printf("\n page table entries are not all present. \n");
		return FAIL;
	}

	return PASS;
}


/* RTC Test - Added by jinghua3.
 *
 * Enable RTC.
 * Input: None.
 * Output: Should print "tick" on screen in a particular rate.
 * Side Effects: none
 * Coverage: RTC
 * Files: rtc.c
 */
void rtc_test(){
	TEST_HEADER;

	rtc_init();
	rtc_set_freq(4);
}



/* Checkpoint 2 tests */
int ece391fs_test_read_exist_file() {
	TEST_HEADER;
    ece391fs_file_info_t finfo;
    if(-1 == read_dentry_by_name("frame1.txt", &finfo)) return FAIL;
	if(ece391fs_size(finfo.inode) == 174) {
		return PASS;
	} else {
		return FAIL;
	}
}

int ece391fs_test_read_nonexistent_file() {
	TEST_HEADER;
    ece391fs_file_info_t finfo;
    if(0 == read_dentry_by_name("404.not.found", &finfo)) return FAIL;
	return PASS;
}

int ece391fs_test_read_exist_idx() {
	TEST_HEADER;
	ece391fs_file_info_t finfo;
	if(-1 == read_dentry_by_index(0, &finfo)) return FAIL;
	if(ECE391FS_FILE_TYPE_FOLDER != finfo.type) return FAIL;
	if(0 != finfo.inode) return FAIL;
	return PASS;
}

int ece391fs_test_read_nonexistent_idx() {
	TEST_HEADER;
	ece391fs_file_info_t finfo;
	if(0 == read_dentry_by_index(100, &finfo)) return FAIL;
	return PASS;
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here

	// Checkpoint 1 - Added by jinghua3.
	//TEST_OUTPUT("Video Memory Paging Test", videoMem_paging_test());
	//TEST_OUTPUT("Kernel Memory Paging Test", kerMem_paging_test());
	//TEST_OUTPUT("Paging Structure Test", paging_struct_test());
	//dereferencing_null_test();
	//division_by_zero_test();
	//deref_nonexist_page_test();
	//rtc_test();

	// Checkpoint 2
	TEST_OUTPUT("ECE391FS Exist File", ece391fs_test_read_exist_file());
	TEST_OUTPUT("ECE391FS Nonexistent File", ece391fs_test_read_nonexistent_file());
	TEST_OUTPUT("ECE391FS Exist File", ece391fs_test_read_exist_idx());
	TEST_OUTPUT("ECE391FS Nonexistent File", ece391fs_test_read_nonexistent_idx());
	// Checkpoint 3
	// Checkpoint 4
	// Checkpoint 5
}
