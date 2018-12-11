#include "rng.h"
#include "cpuid.h"
#include "cmos.h"

uint32_t rand_num = 0;


// GNU Assembler in devel VM is too old to recognize RDRAND instruction,
//   so this is the machine code for two instructions:
// 0f c7 f0     RDRAND %eax
// c3           RET
//   This machine code can be easily regenerated with an up to date GCC and/or GAS.
char rng_x86_instructions[] = {0x0f, 0xc7, 0xf0, 0xc3};

/* void rng_init()
 * @output: RNG initialized with current time
 * @description: seed the RNG with CMOS time
 */
void rng_init() {
    datetime_t time = cmos_datetime();
    rand_num = time.second
             + time.minute * RNG_SEC_IN_MIN
             + time.hour * RNG_SEC_IN_HOUR
             + time.day * RNG_SEC_IN_DAY
             + time.month * RNG_SEC_IN_MONTH
             + time.year * RNG_SEC_IN_YEAR;
}

/* uint32_t rng_generate()
 * @output: next random number in sequence
 * @description: generates a random number with LCG algorithm
 *     described at https://en.wikipedia.org/wiki/Linear_congruential_generator
 *   Calls should be CLI/STIed, or multiple processes may get the same number.
 *   Optionally, if CPU supports it, uses x86 RDRAND instruction to generate a
 *     more random number. (However unsupported by GAS)
 */
uint32_t rng_generate() {
    uint32_t d = 0;
    if(cpu_info.features_ext.rdrnd) {
        // GAS in devel VM is too old to recognize RDRAND instruction,
        // so I put machine code in array rng_x86_instructions on top of this file.
        // C won't let me directly call an array, so I use an assembly call.
        asm volatile("call rng_x86_instructions" : "=a"(d) : : "memory");
        if(d) return d;
    }
    return rand_num = RNG_MASK & (RNG_MULTIPLIER * rand_num + RNG_INCREMENT);
}

// Unified FS interface for RTC.
unified_fs_interface_t rng_if = {
    .open = rng_open,
    .read = rng_read,
    .write = NULL,
    .ioctl = NULL,
    .close = rng_close
};

/* int32_t rng_open(int32_t* inode, char* filename)
 * @input: all ignored
 * @output: 0 (SUCCESS)
 * @description: open a RNG handle.
 */
int32_t rng_open(int32_t* inode, char* filename) {
    return 0;
}

/* int32_t rng_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len)
 * @input: buf, len - ptr and size of buffer
 * @output: 0 (SUCCESS)
 * @description: write a random number into the buffer.
 */
int32_t rng_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len) {
    if(len < sizeof(uint32_t)) return FAIL;
    *(uint32_t*) buf = rng_generate();
    return sizeof(uint32_t);
}

/* int32_t rng_close(int32_t* inode)
 * @input: inode - ignored
 * @output: 0 (SUCCESS)
 * @description: close RNG, currently does nothing
 */
int32_t rng_close(int32_t* inode) {
    return 0;
}
