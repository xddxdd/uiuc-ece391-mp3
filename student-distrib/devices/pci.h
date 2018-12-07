#ifndef _PCI_H_
#define _PCI_H_

#include "../lib/lib.h"

#define PCI_REG_ADDR 0xCF8
#define PCI_REG_DATA 0xCFC

#define PCI_COUNT_BUS 255
#define PCI_COUNT_DEVICE 32
#define PCI_COUNT_FUNC 8

#define PCI_MASK_BAR_MEMSPACE 0xfffffff0
#define PCI_MASK_BAR_IOSPACE 0xfffffffc

#define PCI_TYPE_STANDARD 0
#define PCI_TYPE_PCI_PCI 1
#define PCI_TYPE_CARDBUS 2

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
    uint32_t val[18];
    struct {
        // Basic information, shared among all 3 types.
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
        union {
            // For ordinary devices
            struct {
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
            } device;
            // For PCI-PCI bridge
            struct {
                uint32_t bar[2];
                uint8_t primary_bus_num;
                uint8_t secondary_bus_num;
                uint8_t subordinate_bus_num;
                uint8_t secondary_latency_timer;
                uint8_t io_base;
                uint8_t io_limit;
                uint16_t secondary_status;
                uint16_t memory_base;
                uint16_t memory_limit;
                uint16_t prefetchable_memory_base;
                uint16_t prefetchable_memory_limit;
                uint32_t prefetchable_base_upper;
                uint32_t prefetchable_limit_upper;
                uint16_t io_base_upper;
                uint16_t io_limit_upper;
                uint8_t capability_pointer;
                uint8_t reserved[3];
                uint32_t expansion_rom_base_addr;
                uint8_t interrupt_line;
                uint8_t interrupt_pin;
                uint16_t bridge_control;
            } pci_bridge;
            // For CardBus bridge
            struct {
                uint32_t cardbus_socket_base_addr;
                uint8_t offset_capabilities_list;
                uint8_t reserved;
                uint16_t secondary_status;
                uint8_t pci_bus_num;
                uint8_t cardbus_bus_num;
                uint8_t subordinate_bus_num;
                uint8_t cardbus_latency_timer;
                uint32_t mem_base_addr0;
                uint32_t mem_limit0;
                uint32_t mem_base_addr1;
                uint32_t mem_limit1;
                uint32_t io_base_addr0;
                uint32_t io_limit0;
                uint32_t io_base_addr1;
                uint32_t io_limit1;
                uint8_t interrupt_line;
                uint8_t interrupt_pin;
                uint16_t bridge_control;
                uint16_t subsystem_device_id;
                uint16_t subsystem_vendor_id;
                uint32_t legacy_mode_base_addr;
            } cardbus_bridge;
        };
    };
} pci_device_t;

int32_t pci_get_device(uint8_t bus, uint8_t device, uint8_t func, pci_device_t* ret);
void pci_register_device(uint8_t bus, uint8_t device, uint8_t func, pci_device_t* dev_info);
void pci_init();

#endif
