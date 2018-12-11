#ifndef _RNG_H_
#define _RNG_H_

#include "../lib/lib.h"
#include "../fs/unified_fs.h"

#define RNG_SEC_IN_MIN 60
#define RNG_SEC_IN_HOUR 3600
#define RNG_SEC_IN_DAY 86400
#define RNG_SEC_IN_MONTH (86400 * 30)
#define RNG_SEC_IN_YEAR (86400 * 30 * 365)

#define RNG_MULTIPLIER 1103515245
#define RNG_INCREMENT 12345
#define RNG_MASK 0x7fffffff

void rng_init();
uint32_t rng_generate();

extern unified_fs_interface_t rng_if;
int32_t rng_open(int32_t* inode, char* filename);
int32_t rng_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len);
int32_t rng_close(int32_t* inode);

#endif
