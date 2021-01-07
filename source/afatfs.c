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
 * @brief Complete structure of a primary partition record for FAT32, sector 0.
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
 * @brief Complete (0x5A bytes) BIOS parameter block FAT32, sector 0.
 */
typedef struct __attribute__((packed))
{
  uint8_t jump[3];
  uint8_t softName[8];
  uint16_t bytesPerSector;
  uint8_t sectorsPerCluster;
  uint16_t reservedSectors;
  uint8_t fatCopies;
  uint16_t rootDirEntries;
  uint16_t totalSectors;
  uint8_t mediaType;
  uint16_t fatSectorCount;
  uint16_t sectorsPerTrack;
  uint16_t headCount;
  uint32_t hiddenSectors;
  uint32_t totalSectorCount;

  uint32_t tableSize;
  uint16_t extFlags;
  uint16_t fatVersion;
  uint32_t rootCluster;
  uint16_t fatInfo;
  uint16_t backupSector;
  uint8_t reserved0[12];
  uint8_t driveNumber;
  uint8_t reserved;
  uint8_t bootSignature;
  uint32_t volumeId;
  uint8_t volumeLabel[11];
  uint8_t fatTypeLabel[8];
}PartitionParameterRecord_t;



typedef struct __attribute__((packed))
{
  uint8_t name[8];
  uint8_t ext[3];
  uint8_t attributes;
  uint8_t reserved;
  uint8_t cTimeTenth;
  uint16_t cTime;
  uint16_t cDate;
  uint16_t aTime;
  uint16_t firstClusterHi;
  uint16_t wTime;
  uint16_t wDate;
  uint16_t firstClusterLow;
  uint32_t size;
}DirectoryEntryFat32_t;



/**
 * @brief Simplified structure of a boot sector for FAT32.
 */
typedef struct
{
  FatType_t FatType[4];    /*!< Partitions type */
  uint32_t  StartLBA[4];   /*!< Start sector for the partitions */
  uint32_t  LengthLBA[4];  /*!< Size in sectors of the partitions */
  uint16_t  Signature;
}ReducedMasterBootRecord_t;

/**
 * @brief Simplified partition parameter block for FAT32.
 */
typedef struct
{
  uint32_t FatStartSector[4];
  uint32_t FatSize[4];
  uint32_t DataStartSector[4];
  uint32_t RootSector[4];
}ReducedPartitionParameterRecord_t;


struct
{
  ReducedMasterBootRecord_t MBR;
  uint32_t                  RootSector[4];
  DirectoryEntryFat32_t     RootDir[16];
}FatDisk[AFATS_MAX_DISKS];



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
      memcpy(&FatDisk[Disk].MBR.Signature,
          &buffer[FAT_SIGNATURE_OFFSET], 2);
      /* Verifying the FAT boot sector signature */
      if(FatDisk[Disk].MBR.Signature == FAT_BOOT_SIGNATURE)
      {
        /* Saving the FAT type, the partition's start sector and length */
        for(i = 0; i < FAT_PARTITION_QTY; i++)
        {
          FatDisk[Disk].MBR.FatType[i] =
              buffer[FAT_PARTITION_RECORD0_OFFSET +
                     (FAT_PARTITION_RECORD_SIZE * i) +
                     FAT_TYPE_OF_PARTITION_OFFSET];
          memcpy(&FatDisk[Disk].MBR.StartLBA[i],
                 &buffer[FAT_PARTITION_RECORD0_OFFSET +
                        (FAT_PARTITION_RECORD_SIZE * i) +
                        FAT_START_LBA_OFFSET], 4);
          memcpy(&FatDisk[Disk].MBR.LengthLBA[i],
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



EStatus_t AFATFS_ReadBiosParameter(uint8_t Disk)
{
  PartitionParameterRecord_t Parameters;
  EStatus_t returncode = OPERATION_RUNNING;
  static uint8_t buffer[512];
  uint32_t fatStart, fatSize, dataStart;

  if(Disk < AFATS_MAX_DISKS)
  {

    returncode = DISK_Read(Disk, buffer, FatDisk[Disk].MBR.StartLBA[0], 1);
    if(returncode == ANSWERED_REQUEST)
    {
      memcpy(&Parameters, buffer, sizeof(Parameters));

      fatStart = FatDisk[Disk].MBR.StartLBA[0] + Parameters.reservedSectors;
      fatSize = Parameters.tableSize;
      dataStart = fatStart + (fatSize * Parameters.fatCopies);
      FatDisk[Disk].RootSector[Disk] = dataStart +
          Parameters.sectorsPerCluster * (Parameters.rootCluster - 2);
    }

  }else{
    returncode = ERR_PARAM_ID;
  }

  return returncode;
}



EStatus_t AFATFS_ReadRootDirEntry(uint8_t Disk)
{
  EStatus_t returncode = OPERATION_RUNNING;
  static uint8_t buffer[512];

  if(Disk < AFATS_MAX_DISKS)
  {

    returncode = DISK_Read(Disk, buffer, FatDisk[Disk].RootSector[0], 1);
    if(returncode == ANSWERED_REQUEST)
    {
      memcpy(&FatDisk[Disk].RootDir, buffer, sizeof(FatDisk[Disk].RootDir));
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
      returncode = AFATFS_ReadBiosParameter(0);
      if(returncode == ANSWERED_REQUEST){
        state = 4;
      }
      break;

    case 4:
      returncode = AFATFS_ReadRootDirEntry(0);
      if(returncode == ANSWERED_REQUEST){
        state = 5;
      }
      break;

    case 5:
      break;

    default:
      state = 0;
      break;
    }

  }

}
