#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "devices/rtc.h"	// Added by jinghua3.
#include "fs/ece391fs.h"

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

	//int* ptr = (int*)(0xB8000 - 4); // test mem addr in first 4MB but not in video mem.
	//int* ptr = (int*)(0x800000 - 4); // test mem addr in first 4MB but not in video mem.
	//int* ptr = (int*)(0x800000 + 8);
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

/* int ece391fs_loaded()
 * @output: PASS / FAIL
 * @description: test if ece391fs is properly initialized.
 */
int ece391fs_loaded() {
	TEST_HEADER;
	if(-1 == ece391fs_is_initialized()) return FAIL;
	return PASS;
}

/* int ece391fs_read_existent_file()
 * @output: PASS / FAIL
 * @description: test getting file entry of "frame1.txt".
 *     and verify that its size is 174 bytes.
 *     If the file got modified, this test should be modified too.
 */
int ece391fs_read_existent_file() {
	TEST_HEADER;
    ece391fs_file_info_t finfo;
    if(-1 == read_dentry_by_name("frame1.txt", &finfo)) return FAIL;
	if(ece391fs_size(finfo.inode) == 174) {
		return PASS;
	} else {
		return FAIL;
	}
}

/* int ece391fs_read_nonexistent_file()
 * @output: PASS / FAIL
 * @description: test getting file entry of a nonexistent file.
 *     The FS call should return -1 (Fail).
 */
int ece391fs_read_nonexistent_file() {
	TEST_HEADER;
    ece391fs_file_info_t finfo;
    if(0 == read_dentry_by_name("404.not.found", &finfo)) return FAIL;
	return PASS;
}

/* int ece391fs_read_exist_idx()
 * @output: PASS / FAIL
 * @description: test getting the first file entry, the directory,
 *     and verify some of its attributes.
 */
int ece391fs_read_existent_idx() {
	TEST_HEADER;
	ece391fs_file_info_t finfo;
	if(-1 == read_dentry_by_index(0, &finfo)) return FAIL;
	if(ECE391FS_FILE_TYPE_FOLDER != finfo.type) return FAIL;
	if(0 != finfo.inode) return FAIL;
	return PASS;
}

/* int ece391fs_read_nonexistent_idx()
 * @output: PASS / FAIL
 * @description: test getting file entry of a nonexistent index.
 *     The FS call should return -1 (Fail).
 */
int ece391fs_read_nonexistent_idx() {
	TEST_HEADER;
	ece391fs_file_info_t finfo;
	if(0 == read_dentry_by_index(100, &finfo)) return FAIL;
	return PASS;
}

/* int ece391fs_large_file()
 * @output: PASS / FAIL
 * @description: tests getting information of large file with long name,
 *     verify size (5277 bytes), and verify some segments of data.
 */
int ece391fs_large_file() {
	TEST_HEADER;
	ece391fs_file_info_t finfo;
	if(-1 == read_dentry_by_name("verylargetextwithverylongname.txt", &finfo)) return FAIL;
	if(ece391fs_size(finfo.inode) != 5277) return FAIL;
	char buf[33];
	buf[32] = '\0';
	char std1[] = "very large text file with a very";
	char std2[] = " long name\n123456789012345678901";
	char std3[] = "jklmnopqrstuvwxyzabcdefghijklmno";
	char std4[] = "_+`1234567890-=[]\\{}|;\':\",./<>?\n";
	char std5[] = ",./<>?\n";
	if(32 != read_data(finfo.inode, 0, buf, 32)) return FAIL;		// Test reading at beginning
	if(0 != strncmp(buf, std1, 32)) return FAIL;
	if(32 != read_data(finfo.inode, 32, buf, 32)) return FAIL;		// Test reading with offset
	if(0 != strncmp(buf, std2, 32)) return FAIL;
	if(32 != read_data(finfo.inode, 4090, buf, 32)) return FAIL;	// Test reading across block
	if(0 != strncmp(buf, std3, 32)) return FAIL;
	if(32 != read_data(finfo.inode, 5245, buf, 32)) return FAIL;	// Test reading at end
	if(0 != strncmp(buf, std4, 32)) return FAIL;
	if(7 != read_data(finfo.inode, 5270, buf, 32)) return FAIL;		// Test reading over end
	if(0 != strncmp(buf, std5, 7)) return FAIL;						// should only read 7 bytes
	if(7 != read_data(finfo.inode, 5270, buf, 8000)) return FAIL;	// Test reading well over end
	if(0 != strncmp(buf, std5, 7)) return FAIL;						// should only read 7 bytes
	if(0 != read_data(finfo.inode, 5280, buf, 32)) return FAIL;		// Test reading after end
	return PASS;
}

int ece391fs_list_dir() {
	TEST_HEADER;
	char buf[ECE391FS_MAX_FILENAME_LEN + 2];
	int32_t ret;
	int i;
	for(i = 0; i < ECE391FS_MAX_FILE_COUNT; i++) {
		ret = read_dir(i, buf, ECE391FS_MAX_FILENAME_LEN);
		if(ret == 0) break;
		if(ret < 0) return FAIL;
		buf[ret] = '\n';
		buf[ret + 1] = '\0';
		printf(buf);
	}
	return PASS;
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	clear();
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
<<<<<<< HEAD
	/*TEST_OUTPUT("ECE391FS Loaded", ece391fs_loaded());
=======
	
	TEST_OUTPUT("ECE391FS Loaded", ece391fs_loaded());
>>>>>>> master
	TEST_OUTPUT("ECE391FS Existent File", ece391fs_read_existent_file());
	TEST_OUTPUT("ECE391FS Nonexistent File", ece391fs_read_nonexistent_file());
	TEST_OUTPUT("ECE391FS Existent File", ece391fs_read_existent_idx());
	TEST_OUTPUT("ECE391FS Nonexistent File", ece391fs_read_nonexistent_idx());
<<<<<<< HEAD
	TEST_OUTPUT("ECE391FS Large File", ece391fs_large_file());*/
	TEST_OUTPUT("ECE391FS List Directory", ece391fs_list_dir());
=======
	TEST_OUTPUT("ECE391FS Large File", ece391fs_large_file());

>>>>>>> master
	// Checkpoint 3
	// Checkpoint 4
	// Checkpoint 5
}
