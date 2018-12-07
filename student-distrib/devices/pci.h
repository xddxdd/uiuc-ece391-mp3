#ifndef _PCI_H_
#define _PCI_H_

#include "../lib/lib.h"

#define PCI_REG_ADDR 0xCF8
#define PCI_REG_DATA 0xCFC

typedef union {
    uint32_t val;
    struct __attribute__((packed)) {
        uint8_t reserved0   : 2;
        uint8_t reg_num     : 6;
        uint8_t func_num    : 3;
        uint8_t device_num  : 5;
        uint8_t bus_num     : 8;
        uint8_t reserved1   : 7;
        uint8_t enable      : 1;
    };
} pci_addr_t;

typedef union {
    uint32_t val[16];
    struct {
        uint16_t vendor_id;
        uint16_t device_id;
        uint16_t command;
        uint16_t status;
        uint8_t revision_id;
        uint8_t prog_if;
        uint8_t subclass;
        uint8_t class_code;
        uint8_t cache_line_size;
        uint8_t latency_timer;
        uint8_t header_type;
        uint8_t bist;
        uint32_t bar[6];
        uint32_t cardbus_cis_ptr;
        uint16_t subsystem_vendor_id;
        uint16_t subsystem_id;
        uint32_t expansion_rom_base_addr;
        uint8_t capabilities_pointer;
        uint8_t reserved[7];
        uint8_t interrupt_line;
        uint8_t interrupt_pin;
        uint8_t min_grant;
        uint8_t max_latency;
    };
} pci_device_t;

int32_t pci_get_device(uint8_t bus, uint8_t device, uint8_t func, pci_device_t* ret);
void pci_init();

#endif
