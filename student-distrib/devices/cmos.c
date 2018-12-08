#include "cmos.h"
#include "acpi.h"

/* uint8_t cmod_reg_read(uint8_t index)
 * @input: index - index of data in CMOS
 * @output: ret val - data read from CMOS
 * @description: read data from specified CMOS index.
 */
uint8_t cmos_reg_read(uint8_t index) {
    outb(index, CMOS_PORT_INDEX);
    return inb(CMOS_PORT_DATA);
}

/* void cmos_reg_write(uint8_t index, uint8_t data)
 * @input: index - index of data in CMOS
 *         data - data to be written into CMOS
 * @description: set data on a specified index in CMOS
 */
void cmos_reg_write(uint8_t index, uint8_t data) {
    outb(index, CMOS_PORT_INDEX);
    outb(data, CMOS_PORT_DATA);
}

/* datetime_t cmos_datetime()
 * @output: ret val - structure of current datetime
 * @description: queries CMOS for date time information
 */
datetime_t cmos_datetime() {
    while(cmos_reg_read(CMOS_REG_TIME_UPDATING) & CMOS_REG_TIME_UPDATING_MASK);
    uint16_t year = cmos_reg_read(CMOS_REG_YEAR);
    uint8_t month = cmos_reg_read(CMOS_REG_MONTH);
    uint8_t day = cmos_reg_read(CMOS_REG_DAY);
    uint8_t hour = cmos_reg_read(CMOS_REG_HOUR);
    uint8_t min = cmos_reg_read(CMOS_REG_MINUTE);
    uint8_t sec = cmos_reg_read(CMOS_REG_SECOND);
    uint8_t century = (0 != fadt->Century) ? (cmos_reg_read(fadt->Century)) : 0;

    // Convert hex representation of time into decimal
    year = (year >> 4) * 10 + (year & 0x0f);
    month = (month >> 4) * 10 + (month & 0x0f);
    day = (day >> 4) * 10 + (day & 0x0f);
    hour = (hour >> 4) * 10 + (hour & 0x0f);
    min = (min >> 4) * 10 + (min & 0x0f);
    sec = (sec >> 4) * 10 + (sec & 0x0f);
    century = (century >> 4) * 10 + (century & 0x0f);
    if(0 == century) century = 20;
    year += century * 100;

    datetime_t ret = {year, month, day, hour, min, sec};
    return ret;
}

unified_fs_interface_t cmos_if = {
    .open = cmos_open,
    .read = cmos_read,
    .write = NULL,
    .ioctl = NULL,
    .close = cmos_close
};

/* int32_t cmos_open(int32_t* inode, char* filename)
 * @input: all ignored
 * @output: success
 * @description: does nothing.
 */
int32_t cmos_open(int32_t* inode, char* filename) {
    return 0;
}

/* int32_t cmos_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len)
 * @input: offset - whether we've outputted before
 *         buf - buffer to write to
 *         len - max number of characters to write into
 * @output: buf - written with datetime information
 *          ret val - 0 (SUCCESS) / -1 (FAIL)
 * @description: formats date time and writes them into buffer.
 */
int32_t cmos_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len) {
    if(NULL == buf || NULL == offset) return FAIL;
    char tmp[] = "0000-00-00 00:00:00\n";
    if(len < strlen(tmp)) return FAIL;
    if(0 != *offset) return 0;

    *offset = 1;    // Mark that we've outputted the date time
    datetime_t datetime = cmos_datetime();
    // Concatenating each segment of date time
    // Each number represents offset in output string, whose format is defined above
    itoa(datetime.year, (int8_t*) (tmp), 10);
    tmp[4] = '-';
    itoa(datetime.month, (int8_t*) (tmp + ((datetime.month < 10) ? 6 : 5)), 10);
    tmp[7] = '-';
    itoa(datetime.day, (int8_t*) (tmp + ((datetime.day < 10) ? 9 : 8)), 10);
    tmp[10] = ' ';
    itoa(datetime.hour, (int8_t*) (tmp + ((datetime.hour < 10) ? 12 : 11)), 10);
    tmp[13] = ':';
    itoa(datetime.minute, (int8_t*) (tmp + ((datetime.minute < 10) ? 15 : 14)), 10);
    tmp[16] = ':';
    itoa(datetime.second, (int8_t*) (tmp + ((datetime.second < 10) ? 18 : 17)), 10);
    tmp[19] = '\n';

    // Do the output
    memcpy(buf, tmp, strlen(tmp));
    return strlen(tmp);
}

/* int32_t cmos_close(int32_t* inode)
 * @input: inode - ignored
 * @output: 0 (SUCCESS)
 * @description: close cmos, currently does nothing
 */
int32_t cmos_close(int32_t* inode) {
    return SUCCESS;
}
