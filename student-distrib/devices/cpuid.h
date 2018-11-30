#ifndef _CPUID_H_
#define _CPUID_H_

#include "../lib/lib.h"
#include "../fs/unified_fs.h"

typedef struct {
    int32_t eax;
    int32_t ebx;
    int32_t ecx;
    int32_t edx;
} cpuid_t;

#define CPUID_BRAND_NAME_LEN 12
#define CPUID_BRAND_STR_LEN (4 * 4 * 3)
#define CPUID_CACHE_MAX_LOOP 4
#define CPUID_CACHE_INFO_LEN (15 * CPUID_CACHE_MAX_LOOP)
#define CPUID_FS_BUF 128

typedef struct {
    int32_t max_id_basic;
    int32_t max_id_extended;

    char brand[CPUID_BRAND_NAME_LEN + 1];
    char brand_str[CPUID_BRAND_STR_LEN + 1];

    // cpuid(1).eax
    union {
        uint32_t val;
        struct __attribute__((packed)) {
            uint8_t stepping_id     : 4;
            uint8_t model_id        : 4;
            uint8_t family_id       : 4;
            uint8_t processor_type  : 2;
            uint8_t reserved0       : 2;
            uint8_t ext_model_id    : 4;
            uint8_t ext_family_id   : 8;
            uint8_t reserved1       : 4;
        };
    } version;

    // cpuid(1).ebx
    union {
        uint32_t val;
        struct __attribute__((packed)) {
            uint8_t brand_index;
            uint8_t clflush_line_size;
            uint8_t logical_processors;
            uint8_t apic_id;
        };
    } topology;

    // cpuid(1).ecx
    union {
        uint32_t val;
        struct __attribute__((packed)) {
            uint8_t sse3            : 1;
            uint8_t reserved0       : 2;
            uint8_t monitor         : 1;
            uint8_t ds_cpl          : 1;
            uint8_t reserved1       : 2;
            uint8_t est             : 1;
            uint8_t tm2             : 1;
            uint8_t reserved2       : 1;
            uint8_t cnxt_id         : 1;
            uint32_t reserved3      : 21;
        };
    } features_ext;

    union {
        uint32_t val;
        struct __attribute__((packed)) {
            uint8_t fpu             : 1;
            uint8_t vme             : 1;
            uint8_t de              : 1;
            uint8_t pse             : 1;
            uint8_t tsc             : 1;
            uint8_t msr             : 1;
            uint8_t pae             : 1;
            uint8_t mce             : 1;
            uint8_t cx8             : 1;
            uint8_t apic            : 1;
            uint8_t reserved0       : 1;
            uint8_t sep             : 1;
            uint8_t reserved1       : 1;
            uint8_t pge             : 1;
            uint8_t mca             : 1;
            uint8_t cmov            : 1;
            uint8_t pat             : 1;
            uint8_t pse36           : 1;
            uint8_t psn             : 1;
            uint8_t clfsh           : 1;
            uint8_t reserved2       : 1;
            uint8_t ds              : 1;
            uint8_t acpi            : 1;
            uint8_t mmx             : 1;
            uint8_t fxsr            : 1;
            uint8_t sse             : 1;
            uint8_t sse2            : 1;
            uint8_t ss              : 1;
            uint8_t htt             : 1;
            uint8_t tm              : 1;
            uint8_t reserved3       : 1;
            uint8_t pbe             : 1;
        };
    } features;

    uint8_t cache[CPUID_CACHE_INFO_LEN];

    union {
        uint32_t val;
        struct __attribute__((packed)) {
            uint8_t cache_type          : 5;
            uint8_t cache_level         : 3;
            uint8_t self_init_level     : 1;
            uint8_t fully_assoc_cache   : 1;
            uint8_t reserved0           : 4;
            uint16_t num_threads_share  : 12;
            uint8_t num_cores           : 6;
        };
    } cache_detail;

    union {
        uint32_t val;
        struct __attribute__((packed)) {
            uint16_t sys_coherency_line_size: 12;
            uint16_t physical_line_partition: 10;
            uint16_t ways_of_associativity  : 10;
        };
    } topology_detail;

    uint32_t num_of_sets;
    uint16_t min_mon_line_size;
    uint16_t max_mon_line_size;
} cpu_t;

extern cpu_t cpu_info;
extern unified_fs_interface_t cpuid_if;

cpuid_t cpuid(int32_t eax);
void cpuid_init();

int32_t cpuid_open(int32_t* inode, char* filename);
int32_t cpuid_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len);
int32_t cpuid_write(int32_t* inode, uint32_t* offset, const char* buf, uint32_t len);
int32_t cpuid_close(int32_t* inode);

#endif
