#ifndef _CMOS_H_
#define _CMOS_H_

#include "../lib/lib.h"
#include "../fs/unified_fs.h"

#define CMOS_PORT_INDEX 0x70
#define CMOS_PORT_DATA 0x71

#define CMOS_REG_YEAR 0x09
#define CMOS_REG_MONTH 0x08
#define CMOS_REG_DAY 0x07
#define CMOS_REG_HOUR 0x04
#define CMOS_REG_MINUTE 0x02
#define CMOS_REG_SECOND 0x00
#define CMOS_REG_TIME_UPDATING 0x0a
#define CMOS_REG_TIME_UPDATING_MASK 0x80

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} datetime_t;

uint8_t cmos_reg_read(uint8_t index);
void cmos_reg_write(uint8_t index, uint8_t data);
datetime_t cmos_datetime();

extern unified_fs_interface_t cmos_if;

int32_t cmos_open(int32_t* inode, char* filename);
int32_t cmos_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len);
int32_t cmos_close(int32_t* inode);

#endif
