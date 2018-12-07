#include "pci.h"
#include "qemu_vga.h"

/* int32_t pci_get_device(uint8_t bus, uint8_t device, uint8_t func, pci_device_t* ret)
 * @input: bus, device, func - index of the PCI slot
 *         ret - where to write the return value to
 * @output: ret val - SUCCESS / FAIL
 *          ret - written with description for this PCI device
 * @description: probes if there is PCI device connected to the slot, and if there is any,
 *     get its 64-to-72-byte information and write it into the return value.
 *     C doesn't support returning a 64-byte value on stack, so pointer is used.
 */
int32_t pci_get_device(uint8_t bus, uint8_t device, uint8_t func, pci_device_t* ret) {
    if(bus >= PCI_COUNT_BUS
        || device >= PCI_COUNT_DEVICE
        || func >= PCI_COUNT_FUNC
        || ret == NULL) return FAIL;
    // Compose the address for the query
    pci_addr_t addr = {0};
    addr.func_num = func;
    addr.device_num = device;
    addr.bus_num = bus;
    addr.enable = 1;
    // Register upper bound, 0x10 for standard devices, 0x12 for CardBus
    int reg_bound = 0x10;
    // Get each of the 16 segments of the 64-byte information
    for(addr.reg_num = 0; addr.reg_num < 0x10; addr.reg_num++) {
        outl(addr.val, PCI_REG_ADDR);
        ret->val[addr.reg_num] = inl(PCI_REG_DATA);
        // If this device doesn't exist (vendor_id and device_id are both 0xffff),
        // return FAIL.
        if(addr.reg_num == 0 && (ret->vendor_id == 0xffff || ret->device_id == 0xffff)) {
            return FAIL;
        }
        if(addr.reg_num == 3 && ret->header_type == PCI_TYPE_CARDBUS) {
            reg_bound = 0x12;
        }

    }
    return SUCCESS;
}

/* void pci_register_device(uint8_t bus, uint8_t device, uint8_t func, pci_device_t* dev_info)
 * @input: bus, device, func - index of the PCI slot
 *         dev_info - pointer to the description of the device
 * @output: if the device is recognized, then some drivers are configured according to PCI info
 * @description: configures some devices based on PCI info.
 *     currently only configures QEMU VGA device.
 */
void pci_register_device(uint8_t bus, uint8_t device, uint8_t func, pci_device_t* dev_info) {
    if(bus >= PCI_COUNT_BUS
        || device >= PCI_COUNT_DEVICE
        || func >= PCI_COUNT_FUNC
        || dev_info == NULL) return;
    // Device specific handler
    switch((uint32_t) dev_info->vendor_id << 16 | dev_info->device_id) {
        case 0x12341111:    // vendor device id pair for QEMU VGA device
            // Register bar0 address as the address for linear buffer
            qemu_vga_addr = dev_info->device.bar[0];
            if(qemu_vga_addr & 1) {
                // IO space BAR
                qemu_vga_addr &= PCI_MASK_BAR_IOSPACE;
            } else {
                // Memory space BAR
                qemu_vga_addr &= PCI_MASK_BAR_MEMSPACE;
            }
            printf("QEMU VGA Adapter detected, vram=%x\n", qemu_vga_addr);
            break;
    }
}

/* void pci_init()
 * @output: PCI devices get scanned and initialized
 * @description: scans every slot of the PCI bus, detects and initializes everything.
 */
void pci_init() {
    uint8_t bus, device, func;
    pci_device_t dev_info;

    // 32 * 8 = 256 bits, marks the presence of all PCI buses.
    // PCI bus num = array index * 8 + bit index
    uint8_t buses[32] = {0};
    buses[0] = 1;   // Mark bus 0 as present

    // Smart scan of the whole PCI bus.
    // Start with bus 0, and if a PCI bridge is detected, add that bridge into list.
    for(bus = 0; bus < PCI_COUNT_BUS; bus++) {
        if(!(buses[bus / 8] & (1 << (bus % 8)))) continue;
        printf("PCI scan on bus %d\n", bus);
        for(device = 0; device < PCI_COUNT_DEVICE; device++) {
            for(func = 0; func < PCI_COUNT_FUNC; func++) {
                if(FAIL == pci_get_device(bus, device, func, &dev_info)) continue;
                if(dev_info.header_type == PCI_TYPE_PCI_PCI) {
                    // Schedule the bus for scanning
                    int new_bus = dev_info.pci_bridge.secondary_bus_num;
                    printf("PCI bus %x:%x.%x: bus %x\n", bus, device, func, new_bus);
                    buses[new_bus / 8] |= (1 << (new_bus % 8));
                } else {
                    printf("PCI device %x:%x.%x: %x:%x\n", bus, device, func, dev_info.vendor_id, dev_info.device_id);
                    pci_register_device(bus, device, func, &dev_info);
                }
            }
            if(func == 0) continue;
        }
    }
    printf("PCI scan complete\n");
}
