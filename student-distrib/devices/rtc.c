#include "rtc.h"
#include "../interrupts/multiprocessing.h"

// Unified FS interface for RTC.
unified_fs_interface_t rtc_if = {
    .open = rtc_open,
    .read = rtc_read,
    .write = rtc_write,
    .close = rtc_close
};

uint8_t rtc_init() {
    cli();
    // Initialization code, from https://wiki.osdev.org/RTC
    outb(RTC_REG_B, RTC_PORT_CMD);	    // select register B, and disable NMI
    char prev = inb(RTC_PORT_DATA);	    // read the current value of register B
    outb(RTC_REG_B, RTC_PORT_CMD);	    // set the index again (a read will reset the index to register D)
    outb(prev | 0x40, RTC_PORT_DATA);   // write the previous value ORed with 0x40. This turns on bit 6 of register B
    enable_irq(RTC_IRQ);

    // set the RTC frequency to 1024 Hz
    outb(RTC_REG_A, RTC_PORT_CMD);
    outb(RTC_FREQ_1024, RTC_PORT_DATA);

    sti();
    return 0;
}

/* void rtc_interrupt()
 * @description: function to be called when RTC generates an interrupt.
 *     Decreases offset for all RTC handles, as offsets are counters for timing.
 */
void rtc_interrupt() {
    // For all processes, find RTC handles and update their offset
    int pid;
    for(pid = 0; pid < MAX_PROGRAMS_NUM; pid++) {
        process_t* process = process_get_pcb(pid);
        if(!process->present) continue;
        int fd;
        for(fd = 0; fd < MAX_NUM_FD_ENTRY; fd++) {
            if(process->fd_array[fd].interface != &rtc_if) continue;

            // This is an RTC handle
            if(process->fd_array[fd].pos > 0) {
                process->fd_array[fd].pos--;
            }
        }
    }

    // Read from RTC register C, so it can keep sending interrupts
    outb(RTC_REG_C, RTC_PORT_CMD); // select register C
    inb(RTC_PORT_DATA);		       // just throw away contents

    send_eoi(RTC_IRQ);  // And we're done
}

/* int32_t rtc_open(int32_t* inode, char* filename)
 * @input: all ignored
 * @output: 0 (SUCCESS)
 * @description: initialize RTC, set freq to 2 Hz.
 *     using *inode to record the interval of rtc calls.
 */
int32_t rtc_open(int32_t* inode, char* filename) {
    *inode = RTC_FREQ_BASE / RTC_FREQ_DEFAULT;
    return 0;
}

/* int32_t rtc_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len)
 * @input: all ignored
 * @output: 0 (SUCCESS)
 * @description: wait until the next RTC tick.
 *     using *offset to record how many ticks passed.
 *     *offset will be updated by RTC interrupt.
 */
int32_t rtc_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len)
{
    *offset = *inode;
    while(*offset > 0);
    return 0;
}

/* int32_t rtc_write(int32_t* inode, uint32_t* offset, const char* buf, uint32_t len)
 * @input: buf - value of new frequency for RTC
 *         len - length of buf, should be sizeof(uint16_t)
 * @output: 0 (SUCCESS) / -1 (FAIL)
 * @description: set the frequency of interrupts by RTC
 */
int32_t rtc_write(int32_t* inode, uint32_t* offset, const char* buf, uint32_t len) {
  // invalid input
  //if(len != sizeof(uint16_t)) return -1;  // Removed, otherwise fish won't work
  if(buf == NULL) return -1;
  // change the frequency of RTC
  uint32_t freq = *(uint32_t *)buf;
  *inode = RTC_FREQ_BASE / freq;
  // quit critical section
  return 0;
}

/* int32_t rtc_close(int32_t* inode)
 * @input: inode - ignored
 * @output: 0 (SUCCESS)
 * @description: close RTC, currently does nothing
 */
int32_t rtc_close(int32_t* inode) {
    *inode = 0;
    return 0;
}
