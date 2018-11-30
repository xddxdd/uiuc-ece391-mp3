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

// cpuid(1).edx
// defined in https://en.wikipedia.org/wiki/CPUID
typedef union {
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
} cpuid_features_t;

// cpuid(1).ecx
// defined in https://en.wikipedia.org/wiki/CPUID
typedef union {
    uint32_t val;
    struct __attribute__((packed)) {
        uint8_t sse3            : 1;
        uint8_t pclmulqdq       : 1;
        uint8_t dtes64          : 1;
        uint8_t monitor         : 1;
        uint8_t ds_cpl          : 1;
        uint8_t vmx             : 1;
        uint8_t smx             : 1;
        uint8_t est             : 1;
        uint8_t tm2             : 1;
        uint8_t ssse3           : 1;
        uint8_t cnxt_id         : 1;
        uint8_t sdbg            : 1;
        uint8_t fma             : 1;
        uint8_t cx16            : 1;
        uint8_t xtpr            : 1;
        uint8_t pdcm            : 1;
        uint8_t reserved0       : 1;
        uint8_t pcid            : 1;
        uint8_t dca             : 1;
        uint8_t sse41           : 1;
        uint8_t sse42           : 1;
        uint8_t x2apic          : 1;
        uint8_t movbe           : 1;
        uint8_t popcnt          : 1;
        uint8_t tsc_deadline    : 1;
        uint8_t aes             : 1;
        uint8_t xsave           : 1;
        uint8_t osxsave         : 1;
        uint8_t avx             : 1;
        uint8_t f16c            : 1;
        uint8_t rdrnd           : 1;
        uint8_t hypervisor      : 1;
    };
} cpuid_features_ext_t;

// cpuid(7).ebx
// defined in https://en.wikipedia.org/wiki/CPUID
typedef union {
    uint32_t val;
    struct __attribute__((packed)) {
        uint8_t fsgsbase        : 1;
        uint8_t ia32_tsc_adjust : 1;
        uint8_t sgx             : 1;
        uint8_t bmi1            : 1;
        uint8_t hle             : 1;
        uint8_t avx2            : 1;
        uint8_t reserved0       : 1;
        uint8_t smep            : 1;
        uint8_t bmi2            : 1;
        uint8_t erms            : 1;
        uint8_t invpcid         : 1;
        uint8_t rtm             : 1;
        uint8_t pqm             : 1;
        uint8_t fpu_cs_ds       : 1;
        uint8_t mpx             : 1;
        uint8_t pqe             : 1;
        uint8_t avx512f         : 1;
        uint8_t avx512dq        : 1;
        uint8_t rdseed          : 1;
        uint8_t adx             : 1;
        uint8_t smap            : 1;
        uint8_t avx512ifma      : 1;
        uint8_t pcommit         : 1;
        uint8_t clflushopt      : 1;
        uint8_t clwb            : 1;
        uint8_t intel_pt        : 1;
        uint8_t avx512pf        : 1;
        uint8_t avx512er        : 1;
        uint8_t avx512cd        : 1;
        uint8_t sha             : 1;
        uint8_t avx512bw        : 1;
        uint8_t avx512vl        : 1;
    };
} cpuid_features_ext2_ebx_t;

// cpuid(7).ecx
// defined in https://en.wikipedia.org/wiki/CPUID
typedef union {
    uint32_t val;
    struct __attribute__((packed)) {
        uint8_t prefetchwt1     : 1;
        uint8_t avx512vbmi      : 1;
        uint8_t umip            : 1;
        uint8_t pku             : 1;
        uint8_t ospke           : 1;
        uint8_t reserved0       : 1;
        uint8_t avx512vmbi2     : 1;
        uint8_t reserved1       : 1;
        uint8_t gfni            : 1;
        uint8_t vaes            : 1;
        uint8_t vpclmulqdq      : 1;
        uint8_t avx512vnni      : 1;
        uint8_t avx512bitalg    : 1;
        uint8_t reserved2       : 1;
        uint8_t avx512vpopcntdq : 1;
        uint8_t reserved3       : 1;
        uint8_t reserved4       : 1;
        uint8_t mawau           : 5;
        uint8_t rdpid           : 1;
        uint8_t reserved5       : 7;
        uint8_t sgx_lc          : 1;
        uint8_t reserved6       : 1;
    };
} cpuid_features_ext2_ecx_t;

// cpuid(7).edx
// defined in https://en.wikipedia.org/wiki/CPUID
typedef union {
    uint32_t val;
    struct __attribute__((packed)) {
        uint8_t reserved0       : 2;
        uint8_t avx512_4vnniw   : 1;
        uint8_t avx512_4fmaps   : 1;
        uint16_t reserved1      : 14;
        uint8_t pconfig         : 1;
        uint8_t reserved2       : 7;
        uint8_t spec_ctrl       : 1;
        uint8_t ibc             : 1;
        uint8_t reserved3       : 1;
        uint8_t msr             : 1;
        uint8_t reserved4       : 1;
        uint8_t ssbd            : 1;
    };
} cpuid_features_ext2_edx_t;

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

    cpuid_features_t features;                      // cpuid(1).edx
    cpuid_features_ext_t features_ext;              // cpuid(1).ecx
    cpuid_features_ext2_ebx_t features_ext2_ebx;    // cpuid(7).ebx
    cpuid_features_ext2_ecx_t features_ext2_ecx;    // cpuid(7).ecx
    cpuid_features_ext2_edx_t features_ext2_edx;    // cpuid(7).edx

    uint8_t cache[CPUID_CACHE_INFO_LEN];
    uint8_t cache_len;

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
