#include "acpi.h"

acpi_rsdp_descriptor_t* rsdp_descriptor = NULL;
acpi_rsdt_t* rsdt;
acpi_fadt_t* fadt;
acpi_dsdt_s5_t* dsdt_s5;

int32_t acpi_verify(char* ptr, int32_t len) {
    uint8_t sum = 0;
    int32_t i;
    for(i = 0; i < len; i++) sum += *(ptr + i);
    return 0 == sum;
}

void acpi_init() {
    // Step 1: find the RSDP descriptor
    int32_t pos;
    for(pos = ACPI_SEARCH_START; pos < ACPI_SEARCH_END; pos += ACPI_SEARCH_UNIT) {
        if(0 == strncmp((char*) pos, ACPI_SEARCH_TARGET, ACPI_SEARCH_TARGET_LEN)) {
            rsdp_descriptor = (acpi_rsdp_descriptor_t*) pos;
            if(!acpi_verify((char*) rsdp_descriptor, sizeof(acpi_rsdp_descriptor_t))) {
                rsdp_descriptor = NULL;
                continue;
            }

            printf("acpi v%d found\n", (rsdp_descriptor->Revision == 2) ? 2 : 1);
            // Although we can tell if it's ACPI 1.0 or 2.0, qemu apparently uses only 1.0
            // so we'll not implement anything based on ACPI 2.0
            break;
        }
    }
    if(NULL == rsdp_descriptor) {
        printf("acpi not found\n");
        return;
    }

    // Step 2: Find RSDT
    rsdt = (acpi_rsdt_t*) rsdp_descriptor->RsdtAddress;
    if(!acpi_verify((char*) rsdt, rsdt->header.Length)) {
        rsdt = NULL;
        printf("acpi rsdp is invalid\n");
        return;
    }

    // Step 3: Find other tables
    int num_tables = (rsdt->header.Length - sizeof(acpi_sdt_header_t)) / sizeof(uint32_t);
    if(num_tables > ACPI_RSDT_MAX_TABLES) num_tables = ACPI_RSDT_MAX_TABLES;
    printf("acpi: %d tables to walk through\n", num_tables);

    int32_t i;
    for(i = 0; i < num_tables; i++) {
        acpi_sdt_header_t* header = (acpi_sdt_header_t*) rsdt->ptr_to_other_sdt[i];
        if(!acpi_verify((char*) header, header->Length)) {
            printf("acpi: table #%d invalid\n", i);
            continue;
        } else {
            char table_name[5];
            memset(table_name, 0, 5);
            memcpy(table_name, header->Signature, 4);
            printf("acpi: found table %s\n", table_name);
            if(0 == strncmp("FACP", header->Signature, 4)) {
                // We found the FADT (no typo here)
                fadt = (acpi_fadt_t*) header;
                // Now find DSDT in FADT
                acpi_sdt_header_t* dsdt_header = (acpi_sdt_header_t*) fadt->Dsdt;
                if(!acpi_verify((char*) dsdt_header, dsdt_header->Length)) {
                    printf("acpi: table DSDT invalid\n");
                    continue;
                }
                int32_t j;
                for(j = 0; j < dsdt_header->Length - sizeof(acpi_dsdt_s5_t); j++) {
                    acpi_dsdt_s5_t* s5_tmp = (acpi_dsdt_s5_t*) ((char*) dsdt_header + j);
                    if(0 == strncmp((char*) "_S5_", (char*) s5_tmp->s5_signature, 4)) {
                        printf("acpi: found table DSDT S5\n");
                        dsdt_s5 = s5_tmp;
                        break;
                    }
                }
                if(NULL == dsdt_s5) printf("acpi: table DSDT S5 not found\n");
            }
        }
    }

    // Step 4: Send the ACPI enable command
    if(fadt->AcpiEnable != 0 && fadt->SMI_CommandPort != 0) {
        outb(fadt->AcpiEnable, fadt->SMI_CommandPort);
    }
}

int32_t acpi_shutdown() {
    if(NULL == fadt || NULL == dsdt_s5) return FAIL;
    if(0 != fadt->PM1aControlBlock) {
        outw(dsdt_s5->slp_typa | ACPI_SLP_SHUTDOWN, fadt->PM1aControlBlock);
    }
    if(0 != fadt->PM1bControlBlock) {
        outw(dsdt_s5->slp_typb | ACPI_SLP_SHUTDOWN, fadt->PM1aControlBlock);
    }
    return SUCCESS;
}

int32_t acpi_reboot() {
    while(inb(RESET_8042_PORT) & RESET_8042_MASK);
    outb(RESET_8042_CMD, RESET_8042_PORT);
    return SUCCESS;
}
