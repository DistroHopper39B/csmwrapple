#include "csmwrapple.h"

/* 
 * E820 memory map definitions
 * Based on the legacy BIOS E820 memory mapping interface
 */

/* 
 * Convert UEFI memory types to E820 types 
 */
static uint32_t convert_memory_type(efi_memory_type_t type)
{
    switch (type) {
        case EfiACPIReclaimMemory:
            return E820_ACPI;
        case EfiRuntimeServicesCode:
        case EfiRuntimeServicesData:
        case EfiMemoryMappedIO:
        case EfiMemoryMappedIOPortSpace:
        case EfiPalCode:
            return E820_RESERVED;
        case EfiLoaderCode:
        case EfiLoaderData:
        case EfiBootServicesCode:
        case EfiBootServicesData:
        case EfiConventionalMemory:
            return E820_RAM;
        case EfiACPIMemoryNVS:
            return E820_NVS;
        case EfiUnusableMemory:
            return E820_UNUSABLE;
        case EfiReservedMemoryType:
            return E820_RESERVED;
        default:
            printf("we should not get here\n");
            return E820_RESERVED;
    }
}

/*
 * Build E820 memory map based on UEFI GetMemoryMap
 * Return the number of entries in the E820 map
 */
int build_e820_map(struct csmwrap_priv *priv)
{
    efi_memory_descriptor_t *memory_map = (efi_memory_descriptor_t *) gBA->efi_mem_map_ptr;
    efi_memory_descriptor_t *memory_map_end = (efi_memory_descriptor_t *) gBA->efi_mem_map_ptr + gBA->efi_mem_map_size;
    efi_memory_descriptor_t *memory_map_ptr;
    struct e820_entry *e820_map;
    uintn_t memory_map_size = gBA->efi_mem_map_size;
    uintn_t descriptor_size = gBA->efi_mem_desc_size;
    uint32_t e820_entries = 0;
    
    /* Initialize the E820 map in the low_stub */
    e820_map = priv->low_stub->e820_map;
    memory_map_end = (efi_memory_descriptor_t *)((uint8_t *)memory_map + memory_map_size);
    
    /* Process each memory descriptor and convert to E820 format */
    for (memory_map_ptr = memory_map; 
         memory_map_ptr < memory_map_end && e820_entries < E820_MAX_ENTRIES; 
         memory_map_ptr = NextMemoryDescriptor(memory_map_ptr, descriptor_size)) {
        
        uint64_t start = memory_map_ptr->PhysicalStart;
        uint64_t end = start + (memory_map_ptr->NumberOfPages * EFI_PAGE_SIZE);
        uint32_t type = convert_memory_type(memory_map_ptr->Type);
        
        /* Skip zero-length regions */
        if (start == end)
            continue;
        
        /* Try to merge with previous entry if possible */
        if (e820_entries > 0 && 
            e820_map[e820_entries - 1].addr + e820_map[e820_entries - 1].size == start &&
            e820_map[e820_entries - 1].type == type) {
            /* Extend the previous entry */
            e820_map[e820_entries - 1].size += (end - start);
        } else {
            /* Create a new entry */
            e820_map[e820_entries].addr = start;
            e820_map[e820_entries].size = end - start;
            e820_map[e820_entries].type = type;
            e820_entries++;
        }
    }
    
    /* Save the number of entries in the low_stub */
    priv->low_stub->e820_entries = e820_entries;

#if 1
    printf("E820 memory map created with %d entries\n", e820_entries);
    
    /* Print the E820 map entries for debugging */
    for (int i = 0; i < e820_entries; i++) {
        printf("E820: [%x-%x] type %d\n",
               (unsigned int) e820_map[i].addr,
               (unsigned int) (e820_map[i].addr + e820_map[i].size - 1),
               e820_map[i].type);
    }
#endif
    
    return e820_entries;
}
