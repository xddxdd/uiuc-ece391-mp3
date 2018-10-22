#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"	// Added by jinghua3.

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

// add more tests here

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
 * Coverage: 
 * Files: 
 */
int paging_struct_test(){
	TEST_HEADER;
	
	


	return FAIL;

	//return PASS;
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
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here

	// Checkpoint 1 - Added by jinghua3.
	TEST_OUTPUT("Video Memory Paging Test", videoMem_paging_test());
	TEST_OUTPUT("Kernel Memory Paging Test", kerMem_paging_test());
	TEST_OUTPUT("Paging Structure Test", paging_struct_test());
	//dereferencing_null_test();
	//division_by_zero_test();
	//deref_nonexist_page_test();
	//rtc_test();

	// Checkpoint 2
	// Checkpoint 3
	// Checkpoint 4
	// Checkpoint 5
}
