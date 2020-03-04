/** @file
  RISC-V Flattened Device Tree DXE module

  Copyright (c) 2020, Hewlett Packard Enterprise Development LP. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <libfdt.h>

EFI_STATUS
EFIAPI
FixDtb (
  IN  VOID  *DtbBlob
  )
{
  fdt32_t Size;
  UINT32 ChosenOffset, Err;

  DEBUG ((DEBUG_ERROR, "Fixing up device tree with boot hart id.\n"));

  Size = fdt_totalsize(DtbBlob);
  Err  = fdt_open_into(DtbBlob, DtbBlob, Size + 32);
  if (Err < 0) {
    DEBUG ((DEBUG_ERROR, "Device Tree can't be expanded to accommodate new node\n"));
    return EFI_OUT_OF_RESOURCES;
  }
  ChosenOffset = fdt_path_offset(DtbBlob, "/chosen");
  fdt_setprop_u32(DtbBlob, ChosenOffset, "boot-hartid",
         PcdGet32(PcdBootHartId));

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
InstallFdtFromHob (
  VOID
  )
{
  EFI_STATUS         Status;
  EFI_HOB_GUID_TYPE *GuidHob;
  VOID              *DataInHob;
  UINTN              DataSize;

  GuidHob = GetFirstGuidHob (&gRiscVDtbHobGuid);
  if (GuidHob == NULL) {
    DEBUG ((DEBUG_ERROR, "Failed to find RISC-V DTB Hob\n",
      __FUNCTION__));
    return EFI_OUT_OF_RESOURCES;
  }
  DataInHob = GET_GUID_HOB_DATA (GuidHob);
  DataSize  = GET_GUID_HOB_DATA_SIZE (GuidHob);

  Status = FixDtb (DataInHob);
  ASSERT_EFI_ERROR (Status);

  Status = gBS->InstallConfigurationTable (&gFdtTableGuid, DataInHob);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: failed to install FDT configuration table\n",
      __FUNCTION__));
    return EFI_OUT_OF_RESOURCES;
  }
  return Status;
}

/**
  Install the FDT from the HOB into the EFI system configuration table.

  @param ImageHandle     Image handle this driver.
  @param SystemTable     Pointer to the System Table.

  @retval EFI_SUCCESS    FDT successfully installed into config table.

**/
EFI_STATUS
EFIAPI
InstallFdt (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
{
  EFI_STATUS  Status;

  Status = InstallFdtFromHob ();

  return Status;
}
