#include "pci.h"
#include "qemu_vga.h"

int32_t pci_get_device(uint8_t bus, uint8_t device, uint8_t func, pci_device_t* ret) {
    if(NULL == ret) return FAIL;
    pci_addr_t addr = {0};
    addr.func_num = func;
    addr.device_num = device;
    addr.bus_num = bus;
    addr.enable = 1;
    for(addr.reg_num = 0; addr.reg_num < 0x10; addr.reg_num++) {
        outl(addr.val, PCI_REG_ADDR);
        ret->val[addr.reg_num] = inl(PCI_REG_DATA);
        if(addr.reg_num == 0 && (ret->vendor_id == 0xffff || ret->device_id == 0xffff)) {
            return FAIL;
        }

    }
    return SUCCESS;
}

void pci_register_device(uint8_t bus, uint8_t device, uint8_t func, pci_device_t* dev_info) {
    switch((uint32_t) dev_info->vendor_id << 16 | dev_info->device_id) {
        case 0x12341111:
            qemu_vga_addr = dev_info->bar[0];
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

void pci_init() {
    uint8_t bus, device, func;
    pci_device_t dev_info;
    for(bus = 0; bus < PCI_COUNT_BUS; bus++) {
        for(device = 0; device < PCI_COUNT_DEVICE; device++) {
            for(func = 0; func < PCI_COUNT_FUNC; func++) {
                if(FAIL == pci_get_device(bus, device, func, &dev_info)) continue;
                printf("PCI device %x:%x.%x: %x:%x\n", bus, device, func, dev_info.vendor_id, dev_info.device_id);
                pci_register_device(bus, device, func, &dev_info);
            }
            if(func == 0) continue;
        }
    }
    printf("PCI scan complete\n");
}
