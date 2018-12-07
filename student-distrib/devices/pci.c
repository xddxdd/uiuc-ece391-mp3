#include "pci.h"

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

void pci_init() {
    uint8_t bus, device, func;
    pci_device_t dev_info;
    for(bus = 0; bus < 255; bus++) {
        for(device = 0; device < 32; device++) {
            for(func = 0; func < 8; func++) {
                if(FAIL == pci_get_device(bus, device, func, &dev_info)) continue;
                printf("PCI device %x:%x.%x: %x:%x\n", bus, device, func, dev_info.vendor_id, dev_info.device_id);
            }
            if(func == 0) continue;
        }
    }
    printf("PCI scan complete\n");
    infinite_loop();
}
