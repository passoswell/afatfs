#include <stdint.h>
#include "disk.h"

#include "sdcard.h"




EStatus_t DISK_IntHwInit(uint8_t Disk)
{
  EStatus_t returncode;
  returncode = SDCARD_IntHwInit();
  return returncode;
}


EStatus_t DISK_ExtHwConfig(uint8_t Disk)
{
  EStatus_t returncode;
  returncode = SDCARD_ExtHwConfig();
  return returncode;
}


EStatus_t DISK_Read(uint8_t Disk, uint8_t *Buffer, uint32_t Sector, uint32_t Count)
{
  EStatus_t returncode;
  returncode = SDCARD_Read(Buffer, Sector, Count);
  return returncode;
}


EStatus_t DISK_Write(uint8_t Disk, uint8_t *Buffer, uint32_t Sector, uint32_t Count)
{
  EStatus_t returncode;
  returncode = SDCARD_Write(Buffer, Sector, Count);
  return returncode;
}


EStatus_t DISK_ReadSpecs(uint8_t Disk)
{
  return ERR_NOT_IMPLEMENTED;
}


