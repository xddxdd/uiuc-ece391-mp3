#include "rtl8139.h"

uint32_t rtl_base_port = 0;
uint32_t rtl_interrupt_line = 0;
uint8_t rtl_mac[6];
uint8_t rtl_buf[RTL8139_BUF_SIZE];

void rtl8139_init(uint32_t port, uint32_t int_line) {
    rtl_base_port = port;
    rtl_interrupt_line = int_line;
    printf("RTL8139 on port %x int %d mac", port, int_line);

    // Read RTL8139's MAC address
    int i;
    for(i = 0; i < 6; i++) {
        rtl_mac[i] = inb(rtl_base_port + i);
        printf("%c%x", (i == 0) ? ' ' : ':', rtl_mac[i]);
    }
    printf("\n");

    outb(0x00, RTL8139_REG_CONF1);          // Power it on
    outb(0x10, RTL8139_REG_CMD);            // Reset
    while(0x10 & inb(RTL8139_REG_CMD));     // Wait until reset done
    memset(rtl_buf, 0, RTL8139_BUF_SIZE);   // Clear buf
    outl(rtl_buf, RTL8139_REG_RBSTART);     // Send ring buffer addr
}
