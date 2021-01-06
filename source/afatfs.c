#include <string.h>
#include "afatfs.h"
#include "disk.h"



#define FAT_PARTITION_RECORD0_OFFSET                                         446
#define FAT_PARTITION_RECORD1_OFFSET                                         462
#define FAT_PARTITION_RECORD2_OFFSET                                         478
#define FAT_PARTITION_RECORD3_OFFSET                                         494
#define FAT_PARTITION_RECORD_SIZE                                             16
#define FAT_PARTITION_QTY                                                      4

#define FAT_BOOT_SECTOR_SIGNATURE_OFFSET                                     510
#define FAT_SIGNATURE_OFFSET                                                 510
#define FAT_BOOT_SIGNATURE                                                0xAA55

/** Inside primary partition section **/
#define FAT_ISBOOTABLE_OFFSET                                                  0
#define FAT_START_PARTIION_CHS_OFFSET                                          1
#define FAT_TYPE_OF_PARTITION_OFFSET                                           4
#define FAT_END_OF_PARTITION_CHS_OFFSET                                        5
#define FAT_START_LBA_OFFSET                                                   8
#define FAT_LENGTH_OFFSET                                                     12


/**
 * @brief Valid values for FAT type field on a FAT32's primary partition record.
 */
typedef enum
{
  UNUSED       = 0,  /*!< empty / unused */
  FAT12        = 1,  /*!< FAT12 */
  FAT16_1      = 4,  /*!< FAT16 for partitions <= 32 MiB */
  EXTENDED     = 5,  /*!< extended partition */
  FAT16_2      = 6,  /*!< FAT16 for partitions > 32 MiB */
  FAT32        = 11, /*!< FAT32 for partitions <= 2 GiB */
  FAT32_LBA    = 12, /*!< Same as type 11 (FAT32), but using LBA addressing,
                          which removes size constraints*/
  FAT16_LBA    = 14, /*!< Same as type 6 (FAT16), but using LBA addressing */
  EXTENDED_LBA = 15, /*!< Same as type 5, but using LBA addressing */
}FatType_t;


/**
 * @brief Complete structure of a primary partition record for FAT32.
 */
typedef struct __attribute__((packed))
{
  uint8_t   isBootable;
  uint8_t   StartCHS[3];
  FatType_t FatType;
  uint8_t   EndCHS[3];
  uint32_t  StartLBA;   /*!< Start sector for the partition */
  uint32_t  LengthLBA;  /*!< Size in sectors of the partition */
}PartitionTableEntry_t;


/**
 * @brief Simplified structure of a boot sector for FAT32.
 */
struct
{
  /* PartitionTableEntry_t PrimaryPartition[4]; */
  FatType_t FatType[4];    /*!< Partitions type */
  uint32_t  StartLBA[4];   /*!< Start sector for the partitions */
  uint32_t  LengthLBA[4];  /*!< Size in sectors of the partitions */
  uint16_t  Signature;
}MasterBootRecord[AFATS_MAX_DISKS];


EStatus_t AFATFS_ReadBootSector(uint8_t Disk)
{
  EStatus_t returncode = OPERATION_RUNNING;
  static uint8_t buffer[512];
  uint32_t i;

  if(Disk < AFATS_MAX_DISKS)
  {

    returncode = DISK_Read(Disk, buffer, 0, 1);
    if(returncode == ANSWERED_REQUEST)
    {
      memcpy(&MasterBootRecord[Disk].Signature,
          &buffer[FAT_SIGNATURE_OFFSET], 2);
      /* Verifying the FAT boot sector signature */
      if(MasterBootRecord[Disk].Signature == FAT_BOOT_SIGNATURE)
      {
        /* Saving the FAT type, the partition's start sector and length */
        for(i = 0; i < FAT_PARTITION_QTY; i++)
        {
          MasterBootRecord[Disk].FatType[i] =
              buffer[FAT_PARTITION_RECORD0_OFFSET +
                     (FAT_PARTITION_RECORD_SIZE * i) +
                     FAT_TYPE_OF_PARTITION_OFFSET];
          memcpy(&MasterBootRecord[Disk].StartLBA[i],
                 &buffer[FAT_PARTITION_RECORD0_OFFSET +
                        (FAT_PARTITION_RECORD_SIZE * i) +
                        FAT_START_LBA_OFFSET], 4);
          memcpy(&MasterBootRecord[Disk].LengthLBA[i],
                 &buffer[FAT_PARTITION_RECORD0_OFFSET +
                        (FAT_PARTITION_RECORD_SIZE * i) +
                        FAT_LENGTH_OFFSET], 4);
        }
      }else{
        /* Failed to find a FAT file system */
        returncode = ERR_FAILED;
      }
    }

  }else{
    returncode = ERR_PARAM_ID;
  }

  return returncode;
}



void AFATFS_Test(void)
{
  EStatus_t returncode;
  static uint8_t state = 0;

  while(1)
  {

    switch(state)
    {
    case 0:
      returncode = DISK_IntHwInit(0);
      if(returncode == ANSWERED_REQUEST){
        state = 1;
      }
      break;

    case 1:
      returncode = DISK_ExtHwConfig(0);
      if(returncode == ANSWERED_REQUEST){
        state = 2;
      }
      break;

    case 2:
      returncode = AFATFS_ReadBootSector(0);
      if(returncode == ANSWERED_REQUEST){
        state = 3;
      }
      break;

    case 3:
      break;

    default:
      state = 0;
      break;
    }

  }

}
