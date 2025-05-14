/*
 * Copyright (C) 2025 Sylas Hollander.
 * Copyright (C) 2020-2025 Jiaxun Yang.
 * PURPOSE: CSMWrapple main header file.
 * SPDX-License-Identifier: MIT
*/

#pragma once

#define PACKED

#include "edk2/Acpi10.h"
#include "edk2/Acpi20.h"
#include "edk2/E820.h"
#include "edk2/LegacyBios.h"

typedef struct {
    uint32_t    Data1;
    uint16_t    Data2;
    uint16_t    Data3;
    uint8_t     Data4[8];
} efi_guid_t;

/* This code is based on POSIX-UEFI's uefi.h. https://gitlab.com/bztsrc/posix-uefi
 * Fixed to work on IA32, theoretically (don't use like that)
 */
#define EFIAPI

typedef uint64_t efi_status_t;
typedef uint64_t efi_physical_address_t;
typedef uint64_t efi_virtual_address_t;
typedef void     *efi_handle_t;

#define ACPI_TABLE_GUID                 { 0xeb9d2d30, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }
#define ACPI_20_TABLE_GUID              { 0x8868e871, 0xe4f1, 0x11d3, {0xbc, 0x22, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} }
#define SMBIOS_TABLE_GUID               { 0xeb9d2d31, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }
#define SMBIOS3_TABLE_GUID              { 0xf2fd1544, 0x9794, 0x4a2c, {0x99, 0x2e,0xe5, 0xbb, 0xcf, 0x20, 0xe3, 0x94} }

#define EFI_SIGNATURE_16(A,B)             ((A) | (B<<8))
#define EFI_SIGNATURE_32(A,B,C,D)         (EFI_SIGNATURE_16(A,B)     | (EFI_SIGNATURE_16(C,D)     << 16))
#define EFI_SIGNATURE_64(A,B,C,D,E,F,G,H) (EFI_SIGNATURE_32(A,B,C,D) | ((uint64_t)(EFI_SIGNATURE_32(E,F,G,H)) << 32))

#define SIGNATURE_16(A,B)             EFI_SIGNATURE_16(A,B)
#define SIGNATURE_32(A,B,C,D)         EFI_SIGNATURE_32(A,B,C,D)
#define SIGNATURE_64(A,B,C,D,E,F,G,H) EFI_SIGNATURE_64(A,B,C,D,E,F,G,H)

typedef struct {
    uint32_t                Type;
    uint32_t                Pad;
    efi_physical_address_t  PhysicalStart;
    efi_virtual_address_t   VirtualStart;
    uint64_t                NumberOfPages;
    uint64_t                Attribute;
} efi_memory_descriptor_t;

typedef struct {
    uint64_t    Signature;
    uint32_t    Revision;
    uint32_t    HeaderSize;
    uint32_t    CRC32;
    uint32_t    Reserved;
} efi_table_header_t;

typedef enum {
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType
} efi_allocate_type_t;

typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiUnacceptedMemoryType,
    EfiMaxMemoryType
} efi_memory_type_t;

#define NextMemoryDescriptor(Ptr,Size)  ((efi_memory_descriptor_t *) (((uint8_t *) Ptr) + Size))

#define EFI_PAGE_SIZE           4096
#define EFI_PAGE_MASK           0xFFF
#define EFI_PAGE_SHIFT          12

#define EFI_SIZE_TO_PAGES(a)    ( ((a) >> EFI_PAGE_SHIFT) + ((a) & EFI_PAGE_MASK ? 1 : 0) )

typedef enum {
    EfiResetCold,
    EfiResetWarm,
    EfiResetShutdown
} efi_reset_type_t;

/*** Runtime Services ***/
typedef struct {
    uint16_t    Year;       /* 1998 - 2XXX */
    uint8_t     Month;      /* 1 - 12 */
    uint8_t     Day;        /* 1 - 31 */
    uint8_t     Hour;       /* 0 - 23 */
    uint8_t     Minute;     /* 0 - 59 */
    uint8_t     Second;     /* 0 - 59 */
    uint8_t     Pad1;
    uint32_t    Nanosecond; /* 0 - 999,999,999 */
    int16_t     TimeZone;   /* -1440 to 1440 or 2047 */
    uint8_t     Daylight;
    uint8_t     Pad2;
} efi_time_t;

typedef struct {
    uint32_t Resolution;
    uint32_t Accuracy;
    boolean_t SetsToZero;
} efi_time_capabilities_t;

typedef struct {
    efi_guid_t  CapsuleGuid;
    uint32_t    HeaderSize;
    uint32_t    Flags;
    uint32_t    CapsuleImageSize;
} efi_capsule_header_t;

#ifndef DataBlock
#define DataBlock ContinuationPointer
#endif
typedef struct {
    uint64_t                Length;
    efi_physical_address_t  ContinuationPointer;
} efi_capsule_block_descriptor_t;

typedef efi_status_t (EFIAPI *efi_get_time_t)(efi_time_t *Time, efi_time_capabilities_t *Capabilities);
typedef efi_status_t (EFIAPI *efi_set_time_t)(efi_time_t *Time);
typedef efi_status_t (EFIAPI *efi_get_wakeup_time_t)(boolean_t *Enable, boolean_t *Pending, efi_time_t *Time);
typedef efi_status_t (EFIAPI *efi_set_wakeup_time_t)(boolean_t Enable, efi_time_t *Time);
typedef efi_status_t (EFIAPI *efi_set_virtual_address_map_t)(uintn_t MemoryMapSize, uintn_t DescriptorSize,
uint32_t DescriptorVersion, efi_memory_descriptor_t *VirtualMap);
typedef efi_status_t (EFIAPI *efi_convert_pointer_t)(uintn_t DebugDisposition, void **Address);
typedef efi_status_t (EFIAPI *efi_get_variable_t)(wchar_t *VariableName, efi_guid_t *VendorGuid, uint32_t *Attributes,
        uintn_t *DataSize, void *Data);
typedef efi_status_t (EFIAPI *efi_get_next_variable_name_t)(uintn_t *VariableNameSize, wchar_t *VariableName,
efi_guid_t *VendorGuid);
typedef efi_status_t (EFIAPI *efi_set_variable_t)(wchar_t *VariableName, efi_guid_t *VendorGuid, uint32_t Attributes,
        uintn_t DataSize, void *Data);
typedef efi_status_t (EFIAPI *efi_get_next_high_mono_t)(uint64_t *Count);
typedef efi_status_t (EFIAPI *efi_reset_system_t)(efi_reset_type_t ResetType, efi_status_t ResetStatus, uintn_t DataSize,
        wchar_t *ResetData);
typedef efi_status_t (EFIAPI *efi_update_capsule_t)(efi_capsule_header_t **CapsuleHeaderArray, uintn_t CapsuleCount,
efi_physical_address_t ScatterGatherList);
typedef efi_status_t (EFIAPI *efi_query_capsule_capabilities_t)(efi_capsule_header_t **CapsuleHeaderArray, uintn_t CapsuleCount,
uint64_t *MaximumCapsuleSize, efi_reset_type_t *ResetType);
typedef efi_status_t (EFIAPI *efi_query_variable_info_t)(uint32_t Attributes, uint64_t *MaximumVariableStorageSize,
uint64_t *RemainingVariableStorageSize, uint64_t *MaximumVariableSize);

typedef struct {
    efi_table_header_t              Hdr;

    efi_get_time_t                  GetTime;
    efi_set_time_t                  SetTime;
    efi_get_wakeup_time_t           GetWakeupTime;
    efi_set_wakeup_time_t           SetWakeupTime;

    efi_set_virtual_address_map_t   SetVirtualAddressMap;
    efi_convert_pointer_t           ConvertPointer;

    efi_get_variable_t              GetVariable;
    efi_get_next_variable_name_t    GetNextVariableName;
    efi_set_variable_t              SetVariable;

    efi_get_next_high_mono_t        GetNextHighMonotonicCount;
    efi_reset_system_t              ResetSystem;

    efi_update_capsule_t            UpdateCapsule;
    efi_query_capsule_capabilities_t QueryCapsuleCapabilities;
    efi_query_variable_info_t       QueryVariableInfo;
} efi_runtime_services_t;
extern efi_runtime_services_t *RT;

/*** System Table ***/
typedef struct {
    efi_guid_t  VendorGuid;
    void        *VendorTable;
} efi_configuration_table_t;

typedef struct {
    efi_table_header_t              Hdr;

    wchar_t                         *FirmwareVendor;
    uint32_t                        FirmwareRevision;

    efi_handle_t                    ConsoleInHandle;
    void                            *ConIn;

    efi_handle_t                    ConsoleOutHandle;
    void                            *ConOut;

    efi_handle_t                    ConsoleErrorHandle;
    void                            *StdErr;

    efi_runtime_services_t          *RuntimeServices;
    void                            *BootServices;

    uintn_t                         NumberOfTableEntries;
    efi_configuration_table_t       *ConfigurationTable;
} efi_system_table_t;