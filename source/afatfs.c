#include <string.h>
#include "afatfs.h"
#include "disk.h"
#include "afatfs_types.h"




/**
 * @brief File structure.
 */
typedef struct
{
  uint32_t FirstCluster; /*!< The first cluster number of the file */

  uint8_t Name[11]; /*!< 8 chars for name, 3 chars for extension, dot implied */

  uint32_t FilePos; /*!< The byte offset of the cursor within the file */

  uint32_t LogicalSize; /*!< File size in bytes of data written to the file. */

  uint32_t PhysicalSize; /*!< Size allocated in bytes, multiple of cluster size*/

  uint32_t ClusterPos; /*!< The current file cluster */

  uint32_t ClusterPrevPos; /*!< The previous file cluster */

  uint8_t Mode; /*!< A combination of AFATFS_FILE_MODE_* flags */

  uint8_t Attrib; /*!< Combination of FAT_FILE_ATTRIBUTE_* flags for the directory entry of this file */

  uint8_t Buffer[AFATFS_MAX_SECTOR_SIZE * AFATFS_FILEBUFFER_SIZE];

  uint8_t *pBuffer; /*!< Pointer to the buffer where data should be stored when recovered */

} afatfsFile_t;



struct
{
  ReducedMasterBootRecord_t        MBR;
  ReducedPartitionParameterTable_t PPR;
  DirectoryEntryFat32_t            RootDir[AFATS_MAX_PARTITIONS][16];
  uint8_t Buffer[AFATFS_MAX_SECTOR_SIZE];
}FatDisk[AFATS_MAX_DISKS];



EStatus_t AFATFS_ReadBootSector(uint8_t Disk)
{
  EStatus_t returncode = OPERATION_RUNNING;
  uint32_t i;

  if(Disk < AFATS_MAX_DISKS)
  {

    returncode = DISK_Read(Disk, FatDisk[Disk].Buffer, 0, 1);
    if(returncode == ANSWERED_REQUEST)
    {
      memcpy(&FatDisk[Disk].MBR.Signature,
          &FatDisk[Disk].Buffer[FAT_SIGNATURE_OFFSET], 2);
      /* Verifying the FAT boot sector signature */
      if(FatDisk[Disk].MBR.Signature == FAT_BOOT_SIGNATURE)
      {
        /* Saving the FAT type, the partition's start sector and length */
        for(i = 0; i < FAT_PARTITION_QTY; i++)
        {
          if(i < AFATS_MAX_PARTITIONS)
          {
          FatDisk[Disk].MBR.FatType[i] =
              FatDisk[Disk].Buffer[FAT_PARTITION_RECORD0_OFFSET +
                     (FAT_PARTITION_RECORD_SIZE * i) +
                     FAT_TYPE_OF_PARTITION_OFFSET];
          memcpy(&FatDisk[Disk].MBR.StartLBA[i],
                 &FatDisk[Disk].Buffer[FAT_PARTITION_RECORD0_OFFSET +
                        (FAT_PARTITION_RECORD_SIZE * i) +
                        FAT_START_LBA_OFFSET], 4);
          memcpy(&FatDisk[Disk].MBR.LengthLBA[i],
                 &FatDisk[Disk].Buffer[FAT_PARTITION_RECORD0_OFFSET +
                        (FAT_PARTITION_RECORD_SIZE * i) +
                        FAT_LENGTH_OFFSET], 4);
          }else{
            break;
          }
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



EStatus_t AFATFS_ReadBiosParameter(uint8_t Disk, uint8_t Partition)
{
  PartitionParameterTable_t Parameters;
  EStatus_t returncode = OPERATION_RUNNING;
  uint32_t fatStart, fatSize, dataStart;

  if(Disk < AFATS_MAX_DISKS && Partition < AFATS_MAX_PARTITIONS)
  {

    if(FatDisk[Disk].MBR.FatType[Partition] == FAT32_LBA)
    {

      returncode = DISK_Read(Disk, FatDisk[Disk].Buffer,
          FatDisk[Disk].MBR.StartLBA[Partition], 1);
      if(returncode == ANSWERED_REQUEST)
      {
        memcpy(&Parameters, FatDisk[Disk].Buffer, sizeof(Parameters));

        fatStart = FatDisk[Disk].MBR.StartLBA[Partition] +
            Parameters.reservedSectors;
        fatSize = Parameters.tableSize;
        dataStart = fatStart + (fatSize * Parameters.fatCopies);
        FatDisk[Disk].PPR.RootSector[Partition] = dataStart +
            Parameters.sectorsPerCluster * (Parameters.rootCluster - 2);
        /* The next two are important to determine file sectors */
        FatDisk[Disk].PPR.FatStartSector[Partition] = fatStart;
        FatDisk[Disk].PPR.FatSize[Partition] = fatSize;
        FatDisk[Disk].PPR.DataStartSector[Partition] = dataStart;
        FatDisk[Disk].PPR.SectorPerCluster[Partition] = Parameters.sectorsPerCluster;
      }

    }else{
      returncode = ERR_FAILED;
    }

  }else{
    returncode = ERR_PARAM_VALUE;
  }

  return returncode;
}



EStatus_t AFATFS_ReadRootDirEntry(uint8_t Disk, uint8_t Partition)
{
  EStatus_t returncode = OPERATION_RUNNING;

  if(Disk < AFATS_MAX_DISKS && Partition < AFATS_MAX_PARTITIONS)
  {

    if(FatDisk[Disk].MBR.FatType[Partition] == FAT32_LBA)
    {

      returncode = DISK_Read(Disk, FatDisk[Disk].Buffer,
          FatDisk[Disk].PPR.RootSector[Partition], 1);
      if(returncode == ANSWERED_REQUEST)
      {
        memcpy(&FatDisk[Disk].RootDir[Partition][0], FatDisk[Disk].Buffer,
            sizeof(FatDisk[Disk].RootDir[0]));
      }

    }else{
      returncode = ERR_FAILED;
    }

  }else{
    returncode = ERR_PARAM_VALUE;
  }

  return returncode;
}



EStatus_t AFATFS_ReadFile(uint8_t Disk, uint8_t Partition)
{
  EStatus_t returncode = OPERATION_RUNNING;


//  if(Disk < AFATS_MAX_DISKS && Partition < AFATS_MAX_PARTITIONS)
//  {
//
//    if(FatDisk[Disk].MBR.FatType[Partition] == FAT32_LBA)
//    {
//
//      returncode = DISK_Read(Disk, FatDisk[Disk].Buffer,
//          FatDisk[Disk].PPR.RootSector[Partition], 1);
//      if(returncode == ANSWERED_REQUEST)
//      {
//        memcpy(&FatDisk[Disk].RootDir[Partition][0], FatDisk[Disk].Buffer,
//            sizeof(FatDisk[Disk].RootDir[0]));
//      }
//
//    }else{
//      returncode = ERR_FAILED;
//    }
//
//  }else{
//    returncode = ERR_PARAM_VALUE;
//  }

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
      returncode = AFATFS_ReadBiosParameter(0, 0);
      if(returncode == ANSWERED_REQUEST){
        state = 4;
      }
      break;

    case 4:
      returncode = AFATFS_ReadRootDirEntry(0, 0);
      if(returncode == ANSWERED_REQUEST){
        state = 5;
      }
      break;

    case 5:
      returncode = AFATFS_ReadFile(0, 0);
      if(returncode == ANSWERED_REQUEST){
        state = 6;
      }
      break;

    case 6:
      break;

    default:
      state = 0;
      break;
    }

  }

}
