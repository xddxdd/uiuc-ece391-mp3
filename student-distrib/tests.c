#include "tests.h"
#include "x86_desc.h"
#include "lib/lib.h"
#include "fs/ece391fs.h"
#include "fs/unified_fs.h"
#include "devices/rtc.h"	// Added by jinghua3.
#include "devices/sb16.h"
#include "devices/keyboard.h"
#include "interrupts/sys_calls.h"
#include "interrupts/multiprocessing.h"

#define SCANCODE_ENTER 0x1C

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result == PASS) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}

/* int fd_array_wrapper(int (*func)(fd_array_t))
 * @input: *func - function for test to be executed
 * @output: run the test with generated fd_array
 * @description: generate a real fd_array to use with test,
 *     to avoid bugs with RTC, etc
 */
int test_fdarray_wrapper(int (*func)(fd_array_t*)) {
	// Get a PCB along with its fd_array
	cli();
    int32_t pid = process_allocate();
	sti();
    if(-1 == pid) return FAIL;
    process_t* process = process_get_pcb(pid);
    if(FAIL == unified_init(process->fd_array)) return FAIL;

	int ret = (*func)(process->fd_array);

	// Close all files
	int i;
	for(i = 0; i < MAX_NUM_FD_ENTRY; i++) {
		unified_close(process->fd_array, i);
	}

	// Release the pid
	process->present = 0;
	return ret;
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
 * Not working anymore due to API changes - yuhuixu2
 *
 * Enable RTC.
 * Input: None.
 * Output: Should print "tick" on screen in a particular rate.
 * Side Effects: none
 * Coverage: RTC
 * Files: rtc.c
 */
/*void rtc_test(){
	TEST_HEADER;

	rtc_set_freq(4);
}*/



/* Checkpoint 2 tests */

/* int ece391fs_loaded()
 * @output: PASS / FAIL
 * @description: test if ece391fs is properly initialized.
 */
int ece391fs_loaded() {
	TEST_HEADER;
	if(FAIL == ece391fs_is_initialized()) return FAIL;
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
    if(FAIL == read_dentry_by_name("frame1.txt", &finfo)) return FAIL;
	if(ece391fs_size(finfo.inode) != 174) return FAIL;
	char buf[200];
	if(174 != read_data(finfo.inode, 0, buf, 200)) return FAIL;
	int i;
	for(i = 0; i < 174; i++) putc(buf[i]);
	return PASS;
}

/* int ece391fs_read_nonexistent_file()
 * @output: PASS / FAIL
 * @description: test getting file entry of a nonexistent file.
 *     The FS call should return FAIL.
 */
int ece391fs_read_nonexistent_file() {
	TEST_HEADER;
    ece391fs_file_info_t finfo;
    if(SUCCESS == read_dentry_by_name("404.not.found", &finfo)) return FAIL;
	return PASS;
}

/* int ece391fs_read_toolong_file()
 * @output: PASS / FAIL
 * @description: test getting file entry of a file with name > 32 chars.
 *     The FS call should return FAIL.
 */
int ece391fs_read_toolong_file() {
	TEST_HEADER;
    ece391fs_file_info_t finfo;
    if(SUCCESS == read_dentry_by_name("verylargetextwithverylongname.txt", &finfo)) return FAIL;
	return PASS;
}

/* int ece391fs_read_existent_idx()
 * @output: PASS / FAIL
 * @description: test getting the first file entry, the directory,
 *     and verify some of its attributes.
 */
int ece391fs_read_existent_idx() {
	TEST_HEADER;
	ece391fs_file_info_t finfo;
	if(FAIL == read_dentry_by_index(0, &finfo)) return FAIL;
	if(ECE391FS_FILE_TYPE_FOLDER != finfo.type) return FAIL;
	if(0 != finfo.inode) return FAIL;
	return PASS;
}

/* int ece391fs_read_nonexistent_idx()
 * @output: PASS / FAIL
 * @description: test getting file entry of a nonexistent index.
 *     The FS call should return FAIL.
 */
int ece391fs_read_nonexistent_idx() {
	TEST_HEADER;
	ece391fs_file_info_t finfo;
	if(SUCCESS == read_dentry_by_index(100, &finfo)) return FAIL;
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
	if(FAIL == read_dentry_by_name("verylargetextwithverylongname.tx", &finfo)) return FAIL;
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

/* int ece391fs_list_dir()
 * @output: PASS / FAIL
 * @description: Lists every file in ECE391FS
 */
int ece391fs_list_dir() {
	TEST_HEADER;
	char buf[ECE391FS_MAX_FILENAME_LEN + 1];
	int32_t ret;
	int i;
	for(i = 0; i < ECE391FS_MAX_FILE_COUNT; i++) {
		ret = read_dir(i, buf, ECE391FS_MAX_FILENAME_LEN);
		if(ret == 0) break;
		if(ret > ECE391FS_MAX_FILENAME_LEN) return FAIL;
		if(ret < 0) return FAIL;
		buf[ret] = '\0';
		printf(buf);
		printf(", ");
	}
	return PASS;
}

/* int fs_read_existent_file()
 * @output: PASS / FAIL
 * @description: test reading 174 bytes out of frame1.txt.
 *     Uses generic filesystem functions instead of ECE391FS specific ones.
 *     174 is the exact file size of frame1.txt.
 *     If the file got modified, this test should be modified too.
 */
int ece391fs_interface_read_existent_file() {
	TEST_HEADER;
	int32_t fd;
	uint32_t offset = 0;
	char buf[200];
	if(FAIL == file_open(&fd, "frame1.txt")) return FAIL;
	if(FAIL == file_read(&fd, &offset, buf, 200)) return FAIL;
	if(offset != 174) return FAIL;
	int i;
	for(i = 0; i < offset; i++) putc(buf[i]);
	if(FAIL == file_close(&fd)) return FAIL;
	if(0 != fd) return FAIL;
	return PASS;
}

/* int fs_read_nonexistent_file()
 * @output: PASS / FAIL
 * @description: test getting file entry of a nonexistent file.
 *     Uses generic filesystem functions instead of ECE391FS specific ones.
 *     The FS call should return -1 (Fail).
 */
int ece391fs_interface_read_nonexistent_file() {
	TEST_HEADER;
	int32_t fd = 0;
    if(SUCCESS == file_open(&fd, "404.not.found")) return FAIL;
	if(0 != fd) return FAIL;
	return PASS;
}

/* int fs_read_existent_dir()
 * @output: PASS / FAIL
 * @description: test listing files in root folder.
 *     Uses generic filesystem functions instead of ECE391FS specific ones.
 */
int ece391fs_interface_read_existent_dir() {
	TEST_HEADER;
	int32_t fd = 0;
	char buf[ECE391FS_MAX_FILENAME_LEN + 1];
	if(FAIL == dir_open(&fd, ".")) return FAIL;

	int32_t ret;
	uint32_t offset = 0;
	while(1) {
		ret = dir_read(&fd, &offset, buf, ECE391FS_MAX_FILENAME_LEN);
		if(ret == 0) break;
		if(ret > ECE391FS_MAX_FILENAME_LEN) return FAIL;
		if(ret < 0) return FAIL;
		buf[ret] = '\0';
		printf(buf);
		printf(", ");
	}
	if(FAIL == dir_close(&fd)) return FAIL;
	if(fd != 0) return FAIL;
	return PASS;
}

/* int fs_read_nonexistent_dir()
 * @output: PASS / FAIL
 * @description: test reading from an nonexistent dir.
 *     Uses generic filesystem functions instead of ECE391FS specific ones.
 *     The FS call should return -1 (Fail).
 */
int ece391fs_interface_read_nonexistent_dir() {
	TEST_HEADER;
	int32_t fd = 0;
    if(SUCCESS == dir_open(&fd, "404.not.found")) return FAIL;
	if(0 != fd) return FAIL;
	return PASS;
}

/* int rtc_read_write_test()
 * Not working anymore due to API changes - yuhuixu2
 *
 * @output: PASS / FAIL
 * @description: Tests setting frequency of RTC.
 *     and reading from RTC to wait until the next tick.
 *     Test starts from 2 Hz, doubles frequency every second,
 *     and ends after 1024 Hz.
 */
// int rtc_read_write_test()
// {
// 	TEST_HEADER;
// 	// new frequency set to the RTC
// 	uint16_t freq = 2;
// 	rtc_open(NULL, NULL);
// 	while (freq <= 1024) {
// 		int i;
// 		for(i = 0; i < freq; i++) {
// 			rtc_read(NULL, NULL, NULL, 0);
// 			printf("%d ", freq);
// 		}
// 		freq *= 2;
// 		rtc_write(NULL, NULL, (void*) &freq, sizeof(uint16_t));
// 		printf("\n");
// 	}
// 	rtc_close(NULL);
// 	return PASS;
// }

/* int terminal_driver_test()
 * @output: PASS / FAIL
 * @description: Tests reading/writing from/to terminal driver.
 *     Repeats the name entered.
 */
int terminal_driver_test()
{
	TEST_HEADER;
	int32_t read_retval, write_retval;
	terminal_open(NULL, NULL);
	char buf[128];
	printf("Hi, what's your name? ");
	read_retval = terminal_read(NULL, NULL, buf, 128);
	printf("Hello, ");
	write_retval = terminal_write(NULL, NULL, buf, 128);
	printf("Read %d characters from terminal, and write %d characters to terminal.\n",
	       read_retval, write_retval);
	return PASS;
}

/* Checkpoint 3 tests */

/* int unified_fs_read_file(fd_array_t* fd_array)
 * @input: fd_array - file descriptor array
 * @output: PASS / FAIL
 * @description: Tests reading contents of a file using Unified FS interface.
 *     Covers file open, file pos storage, over-end handling, etc.
 */
int unified_fs_read_file(fd_array_t* fd_array) {
	TEST_HEADER;

	int32_t fd;
	int i;
	char buf[100];
	if(FAIL == (fd = unified_open(fd_array, "frame1.txt"))) return FAIL;

	if(FAIL == unified_read(fd_array, fd, buf, 100)) return FAIL;
	if(fd_array[fd].pos != 100) return FAIL;
	for(i = 0; i < 100; i++) putc(buf[i]);

	if(FAIL == unified_read(fd_array, fd, buf, 100)) return FAIL;
	if(fd_array[fd].pos != 174) return FAIL;
	for(i = 0; i < 74; i++) putc(buf[i]);

	if(FAIL == unified_close(fd_array, fd)) return FAIL;
	if(NULL != fd_array[fd].interface) return FAIL;
	return PASS;
}

/* int unified_fs_read_dir(fd_array_t* fd_array)
 * @input: fd_array - file descriptor array
 * @output: PASS / FAIL
 * @description: Tests listing directory using Unified FS interface.
 */
int unified_fs_read_dir(fd_array_t* fd_array) {
	TEST_HEADER;

	int32_t fd;
	int ret;
	char buf[ECE391FS_MAX_FILENAME_LEN + 1];
	if(FAIL == (fd = unified_open(fd_array, "."))) return FAIL;

	while(1) {
		ret = unified_read(fd_array, fd, buf, ECE391FS_MAX_FILENAME_LEN);
		if(ret == 0) break;
		if(ret > ECE391FS_MAX_FILENAME_LEN) return FAIL;
		if(ret < 0) return FAIL;
		buf[ret] = '\0';
		printf(buf);
		printf(", ");
	}

	if(FAIL == unified_close(fd_array, fd)) return FAIL;
	if(NULL != fd_array[fd].interface) return FAIL;
	return PASS;
}

/* int unified_fs_read_nonexistent(fd_array_t* fd_array)
 * @input: fd_array - file descriptor array
 * @output: PASS / FAIL
 * @description: Tests opening a nonexistent file using Unified FS interface.
 */
int unified_fs_read_nonexistent(fd_array_t* fd_array) {
	TEST_HEADER;
	if(FAIL != unified_open(fd_array, "404.not.found")) return FAIL;
	return PASS;
}

/* int unified_fs_invalid_fd(fd_array_t* fd_array)
 * @input: fd_array - file descriptor array
 * @output: PASS / FAIL
 * @description: Tests Unified FS interface handing of invalid file descriptor.
 */
int unified_fs_invalid_fd(fd_array_t* fd_array) {
	TEST_HEADER;
	int32_t fd = 7;	// There's no fd 7, we haven't opened anything!
	char buf[100];
	if(FAIL != unified_read(fd_array, fd, buf, 100)) return FAIL;
	if(FAIL != unified_write(fd_array, fd, buf, 100)) return FAIL;
	if(FAIL != unified_close(fd_array, fd)) return FAIL;
	return PASS;
}

/* unified_fs_rtc_read_write(fd_array_t* fd_array)
 * @input: fd_array - file descriptor array
 * @output: PASS / FAIL
 * @description: Tests setting frequency of RTC with Unified FS interface,
 *     and reading from RTC to wait until the next tick.
 *     Test starts from 2 Hz, doubles frequency every second,
 *     and ends after 1024 Hz.
 */
int unified_fs_rtc_read_write(fd_array_t* fd_array) {
	TEST_HEADER;

	int32_t fd;
	if(FAIL == (fd = unified_open(fd_array, "rtc"))) return FAIL;

	// new frequency set to the RTC
	uint32_t freq = 2;
	while (freq <= 1024)
	{
		int i;
		for(i = 0; i < freq; i++) {
			if(FAIL == unified_read(fd_array, fd, NULL, 0)) return FAIL;
			printf("%d ", freq);
		}
		freq *= 2;
		if(FAIL == unified_write(fd_array, fd, &freq, sizeof(uint32_t))) return FAIL;
		printf("\n");
	}
	if(FAIL == unified_close(fd_array, fd));
	return PASS;
}

/* int unified_fs_stdio(fd_array_t* fd_array)
 * No longer works due to API changes - yuhuixu2
 *
 * @input: fd_array - file descriptor array
 * @output: PASS / FAIL
 * @description: Tests reading/writing from/to terminal driver with Unified FS interface.
 *     Repeats the name entered.
 */
// int unified_fs_stdio(fd_array_t* fd_array) {
// 	TEST_HEADER;
//
// 	char buf[128];
// 	if(FAIL == unified_write(fd_array, 1, "Hi, what's your name? ", 22)) return FAIL;
// 	if(FAIL == unified_read(fd_array, 0, buf, 128)) return FAIL;
// 	if(FAIL == unified_write(fd_array, 1, "Hello, ", 7)) return FAIL;
// 	if(FAIL == unified_write(fd_array, 1, buf, 128)) return FAIL;
// 	return PASS;
// }

/* syscall_execute_test */
int syscall_execute_test()
{
	TEST_HEADER;
	// to be added
	char* command = "testprint";
	printf("command is %s\n", command);
	syscall_execute ((uint8_t*)command);
	return PASS;
}

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* Extra feature tests */
/* int unified_fs_tux_read()
 * @output: PASS / FAIL
 * @description: Tests reading tux's button state using Unified FS interface.
 */
int unified_fs_tux_read(fd_array_t* fd_array) {
	TEST_HEADER;

	int32_t fd;
	if(FAIL == (fd = unified_open(fd_array, "tux"))) return FAIL;

	// List of buttons, see function interface of tux_read()
	char buttons[8][6] = {"START", "A", "B", "C", "Up", "Down", "Left", "Right"};

	uint8_t prev_buttons = 0;
	uint8_t curr_buttons = 0;
	uint8_t quit_count = 0;
	printf("Press START 3 times to quit\n");
	while(1) {
		if(FAIL == unified_read(fd_array, fd, &curr_buttons, sizeof(uint8_t))) return FAIL;
		int i;
		for(i = 0; i < 8; i++) {
			if(!(prev_buttons & (1 << i)) && (curr_buttons & (1 << i))) {
				// A button has been pressed
				printf("Press ");
				printf(buttons[i]);
				printf("\n");
				// Add counter for quit test every time START is pressed
				if(i == 0) {
					quit_count++;
				} else {
					quit_count = 0;
				}
			} else if((prev_buttons & (1 << i)) && !(curr_buttons & (1 << i))) {
				// A button has been released
				printf("Release ");
				printf(buttons[i]);
				printf("\n");
			}
		}
		prev_buttons = curr_buttons;

		// If START pressed more than 3 times in a row, quit
		if(quit_count >= 3) break;
	}

	if(FAIL == unified_close(fd_array, fd)) return FAIL;
	return PASS;
}

/* int unified_fs_tux_write()
 * @output: PASS / FAIL
 * @description: Tests setting tux's LED using Unified FS interface.
 */
int unified_fs_tux_write(fd_array_t* fd_array) {
	TEST_HEADER;

	// Tries to open both Tux and RTC
	int32_t tux_fd;
	if(FAIL == (tux_fd = unified_open(fd_array, "tux"))) return FAIL;
	int32_t rtc_fd;
	if(FAIL == (rtc_fd = unified_open(fd_array, "rtc"))) return FAIL;
	uint32_t freq = 2;
	if(FAIL == unified_write(fd_array, rtc_fd, &freq, sizeof(uint32_t))) return FAIL;

	// sequence of strings to be displayed on Tux
	char seq[16][6] = {
		"T",
		"TE",
		"TEA",
		"TEAM",
		{'T', 'E', 'A', 'M', 0x01, 0},
		{'T', 'E', 'A', 'M', 0x02, 0},
		{'T', 'E', 'A', 'M', 0x04, 0},
		{'T', 'E', 'A', 'M', 0x08, 0},
		"N",
		"NU",
		"NUL",
		"NULL",
		{'N', 'U', 'L', 'L', 0x01, 0},
		{'N', 'U', 'L', 'L', 0x02, 0},
		{'N', 'U', 'L', 'L', 0x04, 0},
		{'N', 'U', 'L', 'L', 0x08, 0}
	};

	// Display the strings, 3 loops total
	int i = 0;
	while(i < 16 * 3) {
		if(FAIL == unified_write(fd_array, tux_fd, seq[i % 16], strlen(seq[i % 16]))) return FAIL;
		// Wait 0.5s for each operation
		if(FAIL == unified_read(fd_array, rtc_fd, NULL, 0)) return FAIL;
		i++;
	}

	// Closes everything
	if(FAIL == unified_close(fd_array, tux_fd)) return FAIL;
	if(FAIL == unified_close(fd_array, rtc_fd)) return FAIL;
	return PASS;
}

/* int sb16_play_music()
 * Deprecated, use play command in shell - yuhuixu2
 * @output: PASS / FAIL
 * @description: Tests reading music from filesystem and playing it.
 */
#define TEST_SB16_CHUNK_SIZE 0x2000
#define TEST_SB16_SAMPLING_RATE 22050
char sb16_test_buf[TEST_SB16_CHUNK_SIZE];
int sb16_play_music(fd_array_t* fd_array) {
	TEST_HEADER;

	int32_t wav_fd;
	if(FAIL == (wav_fd = unified_open(fd_array, "halloffame.wav"))) return FAIL;
	int32_t aux_fd;
	if(FAIL == (aux_fd = unified_open(fd_array, "aux"))) return FAIL;

	// Copy over first chunk of music
	int32_t size;
	size = unified_read(fd_array, wav_fd, sb16_test_buf, TEST_SB16_CHUNK_SIZE);
	if(size <= 0) return FAIL;
	memset(sb16_test_buf + size, 0, TEST_SB16_CHUNK_SIZE - size);
	if(size != unified_write(fd_array, aux_fd, sb16_test_buf, size)) return FAIL;

	// Copy over second chunk
	size = unified_read(fd_array, wav_fd, sb16_test_buf, TEST_SB16_CHUNK_SIZE);
	if(size < 0) return FAIL;
	memset(sb16_test_buf + size, 0, TEST_SB16_CHUNK_SIZE - size);
	if(size > 0) {
		if(size != unified_write(fd_array, aux_fd, sb16_test_buf, size)) return FAIL;
	}

	// Initialize SB16, note that we have to send command manually using ioctl
	if(FAIL == unified_ioctl(fd_array, aux_fd, SB16_CMD_SAMPLING_RATE)) return FAIL;
	if(FAIL == unified_ioctl(fd_array, aux_fd, (TEST_SB16_SAMPLING_RATE >> 8) & 0xff)) return FAIL;
	if(FAIL == unified_ioctl(fd_array, aux_fd, TEST_SB16_SAMPLING_RATE & 0xff)) return FAIL;
	if(FAIL == unified_ioctl(fd_array, aux_fd, SB16_CMD_PLAY)) return FAIL;
	if(FAIL == unified_ioctl(fd_array, aux_fd, SB16_MODE_STEREO | SB16_MODE_UNSIGNED)) return FAIL;
	if(FAIL == unified_ioctl(fd_array, aux_fd, (TEST_SB16_CHUNK_SIZE - 1) & 0xff)) return FAIL;
	if(FAIL == unified_ioctl(fd_array, aux_fd, ((TEST_SB16_CHUNK_SIZE - 1) >> 8) & 0xff)) return FAIL;

	// Continue playing the following blocks
	while(1) {
		// Wait until one block finished
		if(FAIL == unified_read(fd_array, aux_fd, NULL, 0)) return FAIL;
		// Tell SB16 to stop temporarily after this block, to handle slow FS operations better
		if(FAIL == unified_ioctl(fd_array, aux_fd, SB16_CMD_EXIT_AFTER_BLOCK)) return FAIL;

		// Read the next chunk of data, copy into block correspondingly
		size = unified_read(fd_array, wav_fd, sb16_test_buf, TEST_SB16_CHUNK_SIZE);
		if(size < 0) return FAIL;	// FS error occured
		memset(sb16_test_buf + size, 0, TEST_SB16_CHUNK_SIZE - size);
		if(size == 0) break;		// Music has finished
		if(size != unified_write(fd_array, aux_fd, sb16_test_buf, size)) return FAIL;
		if(FAIL == unified_ioctl(fd_array, aux_fd, SB16_CMD_CONTINUE)) return FAIL;
		if(size < TEST_SB16_CHUNK_SIZE) {
			// Wait until second last block finished
			if(FAIL == unified_read(fd_array, aux_fd, NULL, 0)) return FAIL;
			// The remaining data isn't sufficient for one block
			// Finish after this block
			if(FAIL == unified_ioctl(fd_array, aux_fd, SB16_CMD_EXIT_AFTER_BLOCK)) return FAIL;
			break;
		}
	}
	if(FAIL == unified_close(fd_array, wav_fd)) return FAIL;
	if(FAIL == unified_close(fd_array, aux_fd)) return FAIL;

	return PASS;
}

/* Test suite entry point */
void launch_tests(){
	clear();
	//TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here

	// Checkpoint 1 - Added by jinghua3.
	// TEST_OUTPUT("Video Memory Paging Test", videoMem_paging_test());
	// TEST_OUTPUT("Kernel Memory Paging Test", kerMem_paging_test());
	// TEST_OUTPUT("Paging Structure Test", paging_struct_test());
	// dereferencing_null_test();
	// division_by_zero_test();
	// deref_nonexist_page_test();

	// Checkpoint 2
	// TEST_OUTPUT("ECE391FS Loaded", ece391fs_loaded());
	// TEST_OUTPUT("ECE391FS Existent File", ece391fs_read_existent_file());
	// TEST_OUTPUT("ECE391FS Nonexistent File", ece391fs_read_nonexistent_file());
	// TEST_OUTPUT("ECE391FS Existent File", ece391fs_read_existent_idx());
	// TEST_OUTPUT("ECE391FS Nonexistent File", ece391fs_read_nonexistent_idx());
	// TEST_OUTPUT("ECE391FS Toolong File", ece391fs_read_toolong_file());
	// TEST_OUTPUT("ECE391FS Large File", ece391fs_large_file());
	// TEST_OUTPUT("ECE391FS List Directory", ece391fs_list_dir());
	// TEST_OUTPUT("ECE391FS Interface Existent File", ece391fs_interface_read_existent_file());
	// TEST_OUTPUT("ECE391FS Interface Nonexistent File", ece391fs_interface_read_nonexistent_file());
	// TEST_OUTPUT("ECE391FS Interface Existent Directory", ece391fs_interface_read_existent_dir());
	// TEST_OUTPUT("ECE391FS Interface Nonexistent Directory", ece391fs_interface_read_nonexistent_dir());
	// TEST_OUTPUT("RTC Driver Read/Write Test", rtc_read_write_test());
	// TEST_OUTPUT("Terminal Driver Write Test", terminal_driver_test());

	// Checkpoint 3
	// TEST_OUTPUT("Unified FS File", test_fdarray_wrapper(unified_fs_read_file));
	// TEST_OUTPUT("Unified FS Dir", test_fdarray_wrapper(unified_fs_read_dir));
	// TEST_OUTPUT("Unified FS Nonexistent File", test_fdarray_wrapper(unified_fs_read_nonexistent));
	// TEST_OUTPUT("Unified FS Invalid FD", test_fdarray_wrapper(unified_fs_invalid_fd));
	// TEST_OUTPUT("Unified FS RTC Read/Write", test_fdarray_wrapper(unified_fs_rtc_read_write));
	// Checkpoint 4
	// Checkpoint 5

	// Extra features
	// TEST_OUTPUT("Tux Controller Read", test_fdarray_wrapper(unified_fs_tux_read));
	// TEST_OUTPUT("Tux Controller Write", test_fdarray_wrapper(unified_fs_tux_write));

	// Deprecated / No longer works
	// rtc_test();
	// TEST_OUTPUT("Unified FS STDIO", test_fdarray_wrapper(unified_fs_stdio));
	// TEST_OUTPUT("SB16 Play Music", test_fdarray_wrapper(sb16_play_music));
}
