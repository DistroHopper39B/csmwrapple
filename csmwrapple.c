/*
 * Copyright (C) 2025 Sylas Hollander.
 * Copyright (C) 2020-2025 Jiaxun Yang.
 * PURPOSE: CSMWrapple main file.
 * SPDX-License-Identifier: LGPL-2.1-only
*/

#include "csmwrapple.h"
#include "edk2/LegacyBios.h"

// Generated by: xxd -i Csm16.bin >> Csm16.h
#include "bins/Csm16.h"
// Generated by: xxd -i vgabios.bin >> vgabios.h
#include "bins/vgabios.h"

#define BIOS_ROM_BASE  0xc0000

struct csmwrap_priv priv = {
        .csm_bin = Csm16_bin,
        .vgabios_bin = vgabios_bin
};

mach_boot_args_t *gBA;

static inline void
outb(int port, uint8_t data)
{
    asm volatile("outb %0,%w1" : : "a" (data), "d" (port));
}

static
int test_bios_region_rw()
{
    uint32_t backup;
    uint32_t *bios_region = (uint32_t *)BIOSROM_START;
    uint32_t *bios_region_end = (uint32_t *)BIOSROM_END;
    uint32_t *ptr = bios_region;

    while (ptr < bios_region_end) {
        backup = *ptr;
        *ptr = 0xdeadbeef;
        if (*ptr != 0xdeadbeef) {
            printf("Unable to write to BIOS region\n");
            return -1;
        }
        *ptr = backup;
        ptr++;
    }

    printf("Success\n");

    return 0;
}

static void *find_table(uint32_t signature, uint8_t *csm_bin_base, size_t size)
{
    boolean_t Done;
    uint8_t *Ptr;
    void *Table;

    Done  = false;
    Table = NULL;
    for (Ptr = csm_bin_base; Ptr < (csm_bin_base + size) && !Done; Ptr += 0x10) {
        if (*(uint32_t *)Ptr == signature) {
            Table  = Ptr;
            // FIXME: Get checksum?
            Done = true;
        }
    }

    return Table;
}

static int set_smbios_table()
{
    int i;
    efi_guid_t smbiosGuid = SMBIOS_TABLE_GUID;
    efi_guid_t smbios3Guid = SMBIOS3_TABLE_GUID;
    boolean_t found = false;
    uintptr_t table_addr = 0;

    efi_system_table_t *sys_tbl = (efi_system_table_t *) gBA->efi_sys_tbl;

    for (i = 0; i < sys_tbl->NumberOfTableEntries; i++) {
        efi_configuration_table_t *table;
        table = sys_tbl->ConfigurationTable + i;

        if (!efi_guidcmp(table->VendorGuid, smbiosGuid)) {
            printf("Found SMBIOS Table at %lx\n", (uintptr_t)table->VendorTable);
            table_addr = (uintptr_t)table->VendorTable;
            found = true;
            break;
        }

        if (!efi_guidcmp(table->VendorGuid, smbios3Guid)) {
            printf("Found SMBIOS 3.0 Table at %lx\n", (uintptr_t)table->VendorTable);
            table_addr = (uintptr_t)table->VendorTable;
            found = true;
            break;
        }
    }

    if (found) {
        if (table_addr > 0xffffffff) {
            printf("SMBIOS table address too high\n");
            return -1;
        }
        priv.low_stub->boot_table.SmbiosTable = table_addr;
        return 0;
    }

    printf("No SMBIOS table found\n");

    return -1;
}

static uintptr_t find_HiPmm(void)
{
    uintptr_t HiPmm = 0x0;

    struct e820_entry *e820_map = priv.low_stub->e820_map;
    uint32_t e820_entries = priv.low_stub->e820_entries;

    for (int i = 0; i < e820_entries; i++)
    {
        if (e820_map[i].addr > HiPmm &&
            e820_map[i].size > HIPMM_SIZE &&
            e820_map[i].type == E820_RAM)
        {
            HiPmm = (e820_map[i].addr + e820_map[i].size - 1) - HIPMM_SIZE;
        }
    }

    return HiPmm;
}

noreturn void csmwrapple_init(mach_boot_args_t *ba)
{
    uintptr_t HiPmm;
    uintptr_t csm_bin_base;
    EFI_IA32_REGISTER_SET Regs;

    gBA = ba;

    cons_init(&ba->video, 0xFFFFFFFF, 0x00000000);

    boolean_t verbose = (ba->video.display_mode == DISPLAY_MODE_TEXT);
    if (verbose)
        cons_clear_screen(0x00000000);

    printf("CSMWrapple for Apple TV 1st Gen initializing...\n");

    csm_bin_base = (uintptr_t)BIOSROM_END - sizeof(Csm16_bin);
    priv.csm_bin_base = csm_bin_base;
    printf("csm_bin_base: 0x%lx\n", csm_bin_base);
    if (csm_bin_base < VGABIOS_END) {
        printf("Illegal csm_bin size \n");
        goto hang;
    }

    priv.csm_efi_table = find_table(EFI_COMPATIBILITY16_TABLE_SIGNATURE, Csm16_bin, sizeof(Csm16_bin));
    if (priv.csm_efi_table == NULL) {
        printf("EFI_COMPATIBILITY16_TABLE not found\n");
        goto hang;
    }

    priv.vga_table = find_table(CSM_VGA_TABLE_SIGNATURE, vgabios_bin, sizeof(vgabios_bin));
    if (priv.vga_table == NULL) {
        printf("VGA Table not found\n");
        goto hang;
    }

    // Set up ACPI
    copy_rsdt(&priv);
    // Set up video
    csmwrap_video_init(&priv);

    // Set up low stub.
    priv.low_stub = (struct low_stub *) LOW_STUB_BASE;
    memset((void *) LOW_STUB_BASE, 0, CONVEN_END - LOW_STUB_BASE);

    // Set up SMBIOS
    set_smbios_table();

    // Build E820 map
    build_e820_map(&priv);

    // Now we need to figure out the highest memory address.
    HiPmm = find_HiPmm();
    printf("HiPmm = %lx\n", HiPmm);

    uintptr_t e820_low = (uintptr_t)&priv.low_stub->e820_map;
    priv.csm_efi_table->E820Pointer = e820_low;
    priv.csm_efi_table->E820Length = sizeof(struct e820_entry) * priv.low_stub->e820_entries;
    priv.low_stub->boot_table.AcpiTable = priv.csm_efi_table->AcpiRsdPtrPointer;

    uintptr_t pmm_base = LegacyBiosInitializeThunkAndTable(LOW_STUB_BASE, sizeof(struct low_stub));
    pmm_base += LOW_STACK_SIZE;

    printf("Init Thunk pmm: %lx\n", (uintptr_t)pmm_base);

    priv.low_stub->init_table.BiosLessThan1MB = 0x00080000; // Whole EBDA
    priv.low_stub->init_table.ThunkStart = (uint32_t)(uintptr_t)priv.low_stub;
    priv.low_stub->init_table.ThunkSizeInBytes = sizeof(struct low_stub);
    priv.low_stub->init_table.LowPmmMemory = (uint32_t)pmm_base;
    priv.low_stub->init_table.LowPmmMemorySizeInBytes = (uint32_t)CONVEN_END - (uint32_t)pmm_base;
    priv.low_stub->init_table.HiPmmMemorySizeInBytes = HIPMM_SIZE;
    priv.low_stub->init_table.HiPmmMemory = HiPmm;

    priv.low_stub->vga_oprom_table.OpromSegment = EFI_SEGMENT(VGABIOS_START);
    /* NVIDIA card at 01:00.0 */
    priv.low_stub->vga_oprom_table.PciBus = 0x01;
    priv.low_stub->vga_oprom_table.PciDeviceFunction = (uint8_t)(0x00 << 3 | 0x0);

    printf("CALL16 %x:%x\n", priv.csm_efi_table->Compatibility16CallSegment,
           priv.csm_efi_table->Compatibility16CallOffset);

    /* Disable external interrupts */
    asm volatile ("cli");

    /* Disable 8259 PIC */
    outb(0x21, 0xff);
    outb(0xa1, 0xff);
    outb(0x43, 0x36);
    /* Program PIT to default */
    outb(0x40, 0x00);
    outb(0x40, 0x00);

    /* Copy ROM to location, as late as possible */
    memcpy((void*)csm_bin_base, Csm16_bin, sizeof(Csm16_bin));
    memcpy((void*)VGABIOS_START, vgabios_bin, sizeof(vgabios_bin));

    memset(&Regs, 0, sizeof(EFI_IA32_REGISTER_SET));
    Regs.X.AX = Legacy16InitializeYourself;
    Regs.X.ES = EFI_SEGMENT(&priv.low_stub->init_table);
    Regs.X.BX = EFI_OFFSET(&priv.low_stub->init_table);

    LegacyBiosFarCall86(priv.csm_efi_table->Compatibility16CallSegment,
                        priv.csm_efi_table->Compatibility16CallOffset,
                        &Regs,
                        NULL,
                        0);

    memset(&Regs, 0, sizeof(EFI_IA32_REGISTER_SET));
    Regs.X.AX = Legacy16DispatchOprom;
    Regs.X.ES = EFI_SEGMENT(&priv.low_stub->vga_oprom_table);
    Regs.X.BX = EFI_OFFSET(&priv.low_stub->vga_oprom_table);
    LegacyBiosFarCall86(priv.csm_efi_table->Compatibility16CallSegment,
                        priv.csm_efi_table->Compatibility16CallOffset,
                        &Regs,
                        NULL,
                        0);

    memset(&Regs, 0, sizeof(EFI_IA32_REGISTER_SET));
    Regs.X.AX = Legacy16PrepareToBoot;
    Regs.X.ES = EFI_SEGMENT(&priv.low_stub->boot_table);
    Regs.X.BX = EFI_OFFSET(&priv.low_stub->boot_table);

    LegacyBiosFarCall86(priv.csm_efi_table->Compatibility16CallSegment,
                        priv.csm_efi_table->Compatibility16CallOffset,
                        &Regs,
                        NULL,
                        0);

    memset(&Regs, 0, sizeof(EFI_IA32_REGISTER_SET));
    Regs.X.AX = Legacy16Boot;
    // No arguments?

    LegacyBiosFarCall86(priv.csm_efi_table->Compatibility16CallSegment,
                        priv.csm_efi_table->Compatibility16CallOffset,
                        &Regs,
                        NULL,
                        0);

hang:
    while (1);
}