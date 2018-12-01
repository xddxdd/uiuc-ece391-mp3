#ifndef _ACPI_H_
#define _ACPI_H_

#include "../lib/lib.h"

#define ACPI_SEARCH_START 0xe0000
#define ACPI_SEARCH_END 0x100000
#define ACPI_SEARCH_TARGET "RSD PTR "
#define ACPI_SEARCH_TARGET_LEN 8
#define ACPI_SEARCH_UNIT 16
#define ACPI_RSDT_MAX_TABLES 16
#define ACPI_SLP_SHUTDOWN (1 << 13)

#define RESET_8042_PORT 0x64
#define RESET_8042_MASK 0x02
#define RESET_8042_CMD 0xfe

// Copied from https://wiki.osdev.org/RSDP
typedef struct {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;

    // // ACPI 2.0 specific
    // uint32_t Length;
    // uint32_t XsdtAddress_low;
    // uint32_t XsdtAddress_high;
    // uint8_t ExtendedChecksum;
    // uint8_t reserved[3];
} acpi_rsdp_descriptor_t;

typedef struct {
  uint8_t AddressSpace;
  uint8_t BitWidth;
  uint8_t BitOffset;
  uint8_t AccessSize;
  uint32_t Address_low;
  uint32_t Address_high;
} acpi_generic_addr_t;

// Copied from https://wiki.osdev.org/RSDP
typedef struct {
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
} acpi_sdt_header_t;

typedef struct {
    acpi_sdt_header_t header;
    uint32_t ptr_to_other_sdt[ACPI_RSDT_MAX_TABLES];
} acpi_rsdt_t;

typedef struct {
    acpi_sdt_header_t header;
    uint32_t FirmwareCtrl;
    uint32_t Dsdt;

    // field used in ACPI 1.0; no longer in use, for compatibility only
    uint8_t  Reserved;

    uint8_t  PreferredPowerManagementProfile;
    uint16_t SCI_Interrupt;
    uint32_t SMI_CommandPort;
    uint8_t  AcpiEnable;
    uint8_t  AcpiDisable;
    uint8_t  S4BIOS_REQ;
    uint8_t  PSTATE_Control;
    uint32_t PM1aEventBlock;
    uint32_t PM1bEventBlock;
    uint32_t PM1aControlBlock;
    uint32_t PM1bControlBlock;
    uint32_t PM2ControlBlock;
    uint32_t PMTimerBlock;
    uint32_t GPE0Block;
    uint32_t GPE1Block;
    uint8_t  PM1EventLength;
    uint8_t  PM1ControlLength;
    uint8_t  PM2ControlLength;
    uint8_t  PMTimerLength;
    uint8_t  GPE0Length;
    uint8_t  GPE1Length;
    uint8_t  GPE1Base;
    uint8_t  CStateControl;
    uint16_t WorstC2Latency;
    uint16_t WorstC3Latency;
    uint16_t FlushSize;
    uint16_t FlushStride;
    uint8_t  DutyOffset;
    uint8_t  DutyWidth;
    uint8_t  DayAlarm;
    uint8_t  MonthAlarm;
    uint8_t  Century;

    // reserved in ACPI 1.0; used since ACPI 2.0+
    uint16_t BootArchitectureFlags;

    uint8_t  Reserved2;
    uint32_t Flags;

    // 12 byte structure; see below for details
    acpi_generic_addr_t ResetReg;

    uint8_t  ResetValue;
    uint8_t  Reserved3[3];

    // // 64bit pointers - Available on ACPI 2.0+
    // uint64_t                X_FirmwareControl;
    // uint64_t                X_Dsdt;
    //
    // acpi_generic_addr_t X_PM1aEventBlock;
    // acpi_generic_addr_t X_PM1bEventBlock;
    // acpi_generic_addr_t X_PM1aControlBlock;
    // acpi_generic_addr_t X_PM1bControlBlock;
    // acpi_generic_addr_t X_PM2ControlBlock;
    // acpi_generic_addr_t X_PMTimerBlock;
    // acpi_generic_addr_t X_GPE0Block;
    // acpi_generic_addr_t X_GPE1Block;
} acpi_fadt_t;

typedef struct {
    uint8_t s5_signature[4];
    uint8_t package_op;
    uint8_t package_len;
    uint8_t num_elements;
    uint8_t slp_typa_prefix;
    uint8_t slp_typa;
    uint8_t slp_typb_prefix;
    uint8_t slp_typb;
    uint8_t reserved[4];
} acpi_dsdt_s5_t;

extern acpi_rsdp_descriptor_t* rsdp_descriptor;
extern acpi_rsdt_t* rsdt;
extern acpi_fadt_t* fadt;
extern acpi_dsdt_s5_t* dsdt_s5;

void acpi_init();
int32_t acpi_shutdown();
int32_t acpi_reboot();

#endif
