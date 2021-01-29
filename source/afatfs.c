#include <string.h>
#include <stdio.h>
#include "afatfs.h"
#include "disk.h"
#include "afatfs_types.h"
#include "map_afatfs.h"



/**
 * @brief File structure.
 */
typedef struct
{
  uint8_t Name[8]; /*!< 8 chars for name */

  uint8_t Extension[3]; /*!< 3 chars for extension, dot implied */

  uint32_t ClusterFirst; /*!< The first cluster number of the file */

  uint32_t ClusterPos; /*!< The current file cluster */

  uint32_t ClusterPrev; /*!< The previous file cluster */

  uint32_t SectorFirst;

  uint32_t SectorPos;

  uint32_t SectorPrev;

  uint32_t FilePos; /*!< The byte offset of the cursor within the file */

  uint32_t LogicalSize; /*!< File size in bytes of data written to the file. */

  uint32_t PhysicalSize; /*!< Size allocated in bytes, multiple of
                              cluster size*/

  uint8_t Mode; /*!< A combination of AFATFS_FILE_MODE_* flags */

  uint8_t Attrib; /*!< Combination of FAT_FILE_ATTRIBUTE_* flags for the
                       directory entry of this file */

  uint8_t Buffer[AFATFS_MAX_SECTOR_SIZE * AFATFS_FILEBUFFER_SIZE];

  uint8_t *pBuffer; /*!< Pointer to the buffer where data should be stored
                         when recovered */

  uint8_t Disk; /*!< Stores the disk from wich the file came */

  uint8_t Partition; /*!< Stores te partition from which the file came */

  uint32_t Entry; /*!< Entry position on root dir table */

  uint8_t isInUse; /*!< Flags if the structure represents a valid file */

} afatfsFile_t;



struct
{
  uint8_t                          isInitialized;
  ReducedMasterBootRecord_t        MBR;
  ReducedPartitionParameterTable_t PPR;
  DirectoryEntryFat32_t            RootDir[AFATS_MAX_PARTITIONS][16];
  /*afatfsFile_t                     File[AFATS_MAX_FILES];*/
  uint8_t                          Buffer[AFATFS_MAX_SECTOR_SIZE];
  /* DiskIO_t                         DiskIO; */
}FatDisk[AFATS_MAX_DISKS];


afatfsFile_t Fat32File[AFATS_MAX_FILES];




static EStatus_t AFATFS_ReadBootSector(uint8_t Disk)
{
  EStatus_t returncode = OPERATION_RUNNING;
  uint32_t i, counter;

  if(Disk < AFATS_MAX_DISKS)
  {

    returncode = Disk_List[Disk].Read(FatDisk[Disk].Buffer, 0, 1);
    if(returncode == ANSWERED_REQUEST)
    {
      memcpy(&FatDisk[Disk].MBR.Signature,
          &FatDisk[Disk].Buffer[FAT_SIGNATURE_OFFSET], 2);
      /* Verifying the FAT boot sector signature */
      if(FatDisk[Disk].MBR.Signature == FAT_BOOT_SIGNATURE)
      {
        counter = 0;
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
            if(FatDisk[Disk].MBR.FatType[i] == FAT32_LBA){
              counter++;
            }
          }else{
            break;
          }
        }
        if(counter == 0){
          returncode = ERR_INVALID_FILE_SYSTEM;
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



static EStatus_t AFATFS_ReadBiosParameter(uint8_t Disk, uint8_t Partition)
{
  PartitionParameterTable_t Parameters;
  EStatus_t returncode = OPERATION_RUNNING;
  uint32_t fatStart, fatSize, dataStart;

  if(Disk < AFATS_MAX_DISKS && Partition < AFATS_MAX_PARTITIONS)
  {

    if(FatDisk[Disk].MBR.FatType[Partition] == FAT32_LBA)
    {

      returncode = Disk_List[Disk].Read(FatDisk[Disk].Buffer,
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
        FatDisk[Disk].PPR.SectorPerCluster[Partition] =
            Parameters.sectorsPerCluster;
      }

    }else{
      returncode = ERR_INVALID_FILE_SYSTEM;
    }

  }else{
    returncode = ERR_PARAM_VALUE;
  }

  return returncode;
}



static EStatus_t AFATFS_ReadRootDirEntry(uint8_t Disk, uint8_t Partition)
{
  EStatus_t returncode = OPERATION_RUNNING;

  if(Disk < AFATS_MAX_DISKS && Partition < AFATS_MAX_PARTITIONS)
  {

    if(FatDisk[Disk].MBR.FatType[Partition] == FAT32_LBA)
    {

      returncode = Disk_List[Disk].Read(FatDisk[Disk].Buffer,
          FatDisk[Disk].PPR.RootSector[Partition], 1);
      if(returncode == ANSWERED_REQUEST)
      {
        memcpy(&FatDisk[Disk].RootDir[Partition][0], FatDisk[Disk].Buffer,
            sizeof(FatDisk[Disk].RootDir[0]));
      }

    }else{
      returncode = ERR_INVALID_FILE_SYSTEM;
    }

  }else{
    returncode = ERR_PARAM_VALUE;
  }

  return returncode;
}



static EStatus_t AFATFS_WriteRootDirEntry(uint8_t Disk, uint8_t Partition)
{
  EStatus_t returncode = OPERATION_RUNNING;

  if(Disk < AFATS_MAX_DISKS && Partition < AFATS_MAX_PARTITIONS)
  {

    if(FatDisk[Disk].MBR.FatType[Partition] == FAT32_LBA)
    {

      memcpy(FatDisk[Disk].Buffer, &FatDisk[Disk].RootDir[Partition][0],
          sizeof(FatDisk[Disk].RootDir[0]));

      returncode = Disk_List[Disk].Write(FatDisk[Disk].Buffer,
          FatDisk[Disk].PPR.RootSector[Partition], 1);

    }else{
      returncode = ERR_INVALID_FILE_SYSTEM;
    }

  }else{
    returncode = ERR_PARAM_VALUE;
  }

  return returncode;
}




EStatus_t AFATFS_Mount(uint8_t Disk)
{
  enum{INT_HW_INIT = 0, EXT_DEV_CONFIG, READ_BOOT, READ_BIOS, NOP};
  EStatus_t returncode = OPERATION_RUNNING;
  static uint8_t state[AFATS_MAX_DISKS];
  static uint8_t partCounter[AFATS_MAX_DISKS];
  static uint8_t errorCounter[AFATS_MAX_DISKS];

  if(Disk < AFATS_MAX_DISKS && Disk < Disk_ListSize)
  {
    if(Disk_List[Disk].IntHwInit != NULL &&
        Disk_List[Disk].ExtDevConfig != NULL &&
        Disk_List[Disk].Read != NULL && Disk_List[Disk].Write != NULL)
    {
      switch(state[Disk])
      {
      case INT_HW_INIT:
        /* Initializing internal hardware */
        returncode = Disk_List[Disk].IntHwInit();
        if( returncode == ANSWERED_REQUEST ){
          returncode = OPERATION_RUNNING;
          state[Disk] = EXT_DEV_CONFIG;
        }
        break;

      case EXT_DEV_CONFIG:
        /* Configuring device */
        returncode = Disk_List[Disk].ExtDevConfig();
        if( returncode == ANSWERED_REQUEST ){
          returncode = OPERATION_RUNNING;
          state[Disk] = READ_BOOT;
        }else if(returncode >= RETURN_ERROR_VALUE){
          state[Disk] = EXT_DEV_CONFIG;
        }
        break;

      case READ_BOOT:
        /* Reading boot sector (sector 0) */
        returncode = AFATFS_ReadBootSector(Disk);
        if(returncode == ANSWERED_REQUEST){
          returncode = OPERATION_RUNNING;
          state[Disk] = READ_BIOS;
        }else if(returncode >= RETURN_ERROR_VALUE){
          state[Disk] = EXT_DEV_CONFIG;
        }
        break;

      case READ_BIOS:
        /* Reading what seems to be an extension of the boot sector */
        returncode = AFATFS_ReadBiosParameter(Disk, partCounter[Disk]);
        if(returncode == ANSWERED_REQUEST){
          partCounter[Disk]++;
          if(partCounter[Disk] >= AFATS_MAX_PARTITIONS){
            /* All partitions were read, and at least one is valid */
            partCounter[Disk] = 0;
            errorCounter[Disk] = 0;
            partCounter[Disk] = 0;
            FatDisk[Disk].isInitialized = 1;
            state[Disk] = NOP;
          }else{
            returncode = OPERATION_RUNNING;
          }
        }else if(returncode == ERR_INVALID_FILE_SYSTEM){
          errorCounter[Disk]++;
          partCounter[Disk]++;
          if(partCounter[Disk] >= AFATS_MAX_PARTITIONS){
            partCounter[Disk] = 0;
            if(errorCounter[Disk] < AFATS_MAX_PARTITIONS){
              /* At least one valid partition was found */
              FatDisk[Disk].isInitialized = 1;
              state[Disk] = NOP;
              returncode = ANSWERED_REQUEST;
            }else{
              /* No valid partition was found */
              state[Disk] = EXT_DEV_CONFIG;
            }
            errorCounter[Disk] = 0;
            partCounter[Disk] = 0;
          }else{
            returncode = OPERATION_RUNNING;
          }
        } else if(returncode >= RETURN_ERROR_VALUE){
          state[Disk] = EXT_DEV_CONFIG;
        }
        break;

      case NOP:
        /* Will configure the disk again */
        FatDisk[Disk].isInitialized = 0;
        state[Disk] = EXT_DEV_CONFIG;
        break;

      default:
        /* What happened? Maybe cosmic rays */
        state[Disk] = INT_HW_INIT;
        break;
      }
    }
  }else{
    returncode = ERR_PARAM_VALUE;
  }

  return returncode;
}



EStatus_t AFATFS_Open(uint8_t Disk, uint8_t Partition, char *FileName,
    uint8_t Mode, uint8_t *FileHandle)
{
  enum{FETCH_NAME = 0, READ_ROOT_DIR, COPY_FILE_TO_BUFFER};
  EStatus_t returncode = OPERATION_RUNNING;
  static uint8_t state[AFATS_MAX_DISKS];
  uint32_t i;

  /* Variables used for file name's string manipulation */
  const char forbidenChar[] = {'/', ':'};
  char *p;
  uint32_t nameSize, extensionSize;

  if(Disk < AFATS_MAX_DISKS && Disk < Disk_ListSize)
  {
    if(FatDisk[Disk].isInitialized == 1)
    {
      /*
       * This piece of code performs the following:
       * 1 - Find a file structure not in use.
       * 2 - Fetch the file name. Verify if it is 8.3 or less, verify if there
       *     are no subfolders in the name.
       * 3 - Read root directory entry.
       * 4 - Search for the suplied file name in the root dir entry.
       * 5 - Save relevant data from file entry.
       */
      switch(state[Disk]){
      case FETCH_NAME:
        returncode = OPERATION_RUNNING;
        /* Verifying if there is a file structure free */
        for(i = 0; i < AFATS_MAX_FILES; i++){
          if(!Fat32File[i].isInUse){
            *FileHandle = i;
            break;
          }
        }
        if(i >= AFATS_MAX_FILES){
          returncode = ERR_RESOURCE_DEPLETED;
        }

        /* Validating file name
         * Subfolders not implemented, disk identifiers not allowed */
        if(returncode == OPERATION_RUNNING){
          for(i = 0; i < sizeof(forbidenChar); i++){
            p = strchr(FileName,forbidenChar[i]);
            if(p != NULL){
              returncode = ERR_PARAM_NAME;
              break;
            }
          }
        }

        /* Validating file name - sizing and saving to file structure */
        if(returncode == OPERATION_RUNNING){
          p = strchr(FileName,'.');
          if(p != NULL){
            /* File has extension */
            nameSize = p - FileName;
            extensionSize = strlen(FileName) - nameSize -1;
            if(nameSize <= 8 && extensionSize <= 3 ){
              /* Completing with spaces */
              memset(Fat32File[*FileHandle].Name, ' ', 8);
              memset(Fat32File[*FileHandle].Extension, ' ', 3);
              /* Copying the name */
              memcpy(Fat32File[*FileHandle].Name, FileName, nameSize);
              /* Copying the extension */
              memcpy(Fat32File[*FileHandle].Extension,
                  FileName + nameSize + 1 , extensionSize);
              Fat32File[*FileHandle].isInUse = 1;
              state[Disk] = READ_ROOT_DIR;
            }else{
              returncode = ERR_PARAM_NAME;
            }
          }else{
            /* File has no extension */
            nameSize = strlen(FileName);
            if(nameSize <= 8){
              /* Completing with spaces */
              memset(Fat32File[*FileHandle].Name, ' ', 8);
              memset(Fat32File[*FileHandle].Extension, ' ', 3);
              /* Copying the name */
              memcpy(Fat32File[*FileHandle].Name, FileName, nameSize);
              Fat32File[*FileHandle].isInUse = 1;
              state[Disk] = READ_ROOT_DIR;
            }else{
              returncode = ERR_PARAM_NAME;
            }
          }
        }
        break;

      case READ_ROOT_DIR:
        returncode = AFATFS_ReadRootDirEntry(Disk, Partition);
        if(returncode == ANSWERED_REQUEST){
          /*Searching for file name within root entries*/
          for(i = 0; i < 16; i++){
            if(!memcmp(FatDisk[Disk].RootDir[Partition][i].Name,
                Fat32File[*FileHandle].Name, 8) &&
                !memcmp(FatDisk[Disk].RootDir[Partition][i].Ext,
                    Fat32File[*FileHandle].Extension, 3))
            {
              break;
            }
          }
          if(i < 16){
            /* File was found */
            Fat32File[*FileHandle].Disk = Disk;
            Fat32File[*FileHandle].Partition = Partition;
            Fat32File[*FileHandle].Entry = i;
            Fat32File[*FileHandle].FilePos = 0; /*Start of file*/
            Fat32File[*FileHandle].LogicalSize =
                FatDisk[Disk].RootDir[Partition][i].Size;
            /* Considering a file is only one cluster in size */
            Fat32File[*FileHandle].PhysicalSize =
                512 * FatDisk[Disk].PPR.SectorPerCluster[Partition];

            Fat32File[*FileHandle].ClusterFirst =
                (uint32_t) (FatDisk[Disk].RootDir[Partition][i].FirstClusterHi
                    << 16) |
                    FatDisk[Disk].RootDir[Partition][i].FirstClusterLow;
            Fat32File[*FileHandle].ClusterPos =
                Fat32File[*FileHandle].ClusterFirst;
            Fat32File[*FileHandle].ClusterPrev = 0; /*Invalid value*/

            Fat32File[*FileHandle].SectorFirst =
                FatDisk[Disk].PPR.DataStartSector[Partition] +
                ( FatDisk[Disk].PPR.SectorPerCluster[Partition] *
                    (Fat32File[*FileHandle].ClusterFirst - 2) );
            Fat32File[*FileHandle].SectorPos =
                Fat32File[*FileHandle].SectorFirst;
            Fat32File[*FileHandle].SectorPrev = 0; /*Invalid value*/

            Fat32File[*FileHandle].isInUse = 1;
            returncode = ANSWERED_REQUEST;
            state[Disk] = FETCH_NAME;
          }else{
            /* File was not found */
            Fat32File[*FileHandle].isInUse = 0;
            returncode = ERR_FAILED;
            state[Disk] = FETCH_NAME;
          }
        }else if(returncode >= RETURN_ERROR_VALUE){
          state[Disk] = FETCH_NAME;
        }
        break;

      default:
        state[Disk] = FETCH_NAME;
        break;
      }
    }else{
      returncode = ERR_DISABLED;
    }
  }else{
    returncode = ERR_PARAM_VALUE;
  }

  return returncode;
}



EStatus_t AFATFS_Seek(uint8_t FileHandle, uint32_t Offset)
{
  EStatus_t returncode = OPERATION_RUNNING;

  if(Fat32File[FileHandle].isInUse == 1)
  {
    if(Offset < Fat32File[FileHandle].LogicalSize)
    {
      Fat32File[FileHandle].FilePos = Offset;
      returncode = ANSWERED_REQUEST;
    }else
    {
      /* Offset is bigger than the file itself */
      returncode = ERR_PARAM_OFFSET;
    }
  }else{
    returncode = ERR_DISABLED;
  }

  return returncode;
}



EStatus_t AFATFS_Read(uint8_t FileHandle, uint8_t *Buffer, uint32_t Size,
    uint32_t *BytesRead)
{
  EStatus_t returncode = OPERATION_RUNNING;
  uint32_t sectorFirst, sectorLast, nSectors, sectorOffset;
  uint8_t Disk;


  if(Fat32File[FileHandle].isInUse == 1)
  {
    /*
     * Steps:
     * 1 - Compute the starting sector and number of sectors where the data is
     *     based on cursor position on file, sector size, file size.
     * 2 - Read the data from the memory.
     * 3 - Copy the data requested tyo the supplied buffer.
     *
     * Notes:
     * 1 - Sector position is updated only after its memory content is read.
     *
     * TODO: reduce disk access if the data requested is already buffered, maybe
     * using SectorPos and SectorPrev values.
     */
    if(Size == 0){
      *BytesRead = 0;
      returncode = ANSWERED_REQUEST;
    }else if( Fat32File[FileHandle].FilePos >=
        Fat32File[FileHandle].LogicalSize )
    {
      returncode = ERR_FAILED;
    }else
    {
      Disk = Fat32File[FileHandle].Disk;
      /* First sector relative to beginning of file */
      sectorFirst = Fat32File[FileHandle].FilePos / 512;
      /* Cursor positon within the first sector*/
      sectorOffset = Fat32File[FileHandle].FilePos - (512 * sectorFirst);
      /* Last sector relative to beginning of file */
      if((Size + Fat32File[FileHandle].FilePos) <
          Fat32File[FileHandle].LogicalSize)
      {
        sectorLast = (Fat32File[FileHandle].FilePos + Size) / 512;
      }else
      {
        /* If the size requested is bigger than file size */
        sectorLast = (Fat32File[FileHandle].LogicalSize) / 512;
      }
      /* Computing number of sectors to read */
      nSectors = 1 + (sectorLast - sectorFirst);
      /* Transforming relative first sector into absolute value */
      sectorFirst += Fat32File[FileHandle].SectorFirst;

      if(nSectors <= AFATFS_FILEBUFFER_SIZE)
      {

        returncode = Disk_List[Disk].Read(Fat32File[FileHandle].Buffer,
            sectorFirst , nSectors);
        if(returncode == ANSWERED_REQUEST)
        {
          /* Copying requested data to supplied buffer */
          if((Size + Fat32File[FileHandle].FilePos) <
              Fat32File[FileHandle].LogicalSize)
          {
            *BytesRead = Size;
          }
          else
          {
            /* If the size requested is bigger than file size */
            *BytesRead = Fat32File[FileHandle].LogicalSize -
                Fat32File[FileHandle].FilePos;
          }
          memcpy(Buffer, Fat32File[FileHandle].Buffer + sectorOffset,
              *BytesRead);
          /* Updating file cursor position */
          Fat32File[FileHandle].FilePos += *BytesRead;
          /* Updating cluster position */
          Fat32File[FileHandle].ClusterPrev = Fat32File[FileHandle].ClusterPos;
          /* Updating sector positon */
          Fat32File[FileHandle].SectorPrev = Fat32File[FileHandle].SectorPos;
          Fat32File[FileHandle].SectorPos = sectorFirst;

        }

      }else{
        returncode = ERR_BUFFER_SIZE;
      }

    }

  }else{
    returncode = ERR_DISABLED;
  }

  return returncode;
}



EStatus_t AFATFS_Write(uint8_t FileHandle, uint8_t *Buffer, uint32_t Size)
{
  enum{READ_FIRST_SECTOR = 0, READ_LAST_SECTOR, WRITE_DATA, UPDATE_ENTRY};
  static uint8_t state[AFATS_MAX_DISKS];
  EStatus_t returncode = OPERATION_RUNNING;
  uint32_t sectorFirst, sectorLast, nSectors, sectorFOffset, sectorLOffset;
  uint8_t Disk, Partition;
  uint32_t Entry;

  if(Fat32File[FileHandle].isInUse == 1)
  {
    /*
     * Steps:
     * 1 - Read first sector from the disk
     * 2 - Copy data in the correct position of file buffer
     * 3 - Read last sector from the disk
     * 4 - Update file buffer with the new data supplyed
     * 5 - Write data back to the disk
     * 6 - Update root entry list with new file size
     *
     * Notes:
     * 1 - Sector position is updated only after its memory content is written
     *     and the root entry list is updated.
     * 2 - There are three cases:
     *     a - There is only one sector to write
     *     b - There are two or more sectors to write
     */
    if(Size == 0){
      returncode = ANSWERED_REQUEST;
    }else if( Fat32File[FileHandle].FilePos + Size >=
        Fat32File[FileHandle].PhysicalSize ){
      returncode = ERR_FAILED;
    }else
    {
      Disk = Fat32File[FileHandle].Disk;
      Partition = Fat32File[FileHandle].Partition;
      Entry = Fat32File[FileHandle].Entry;
      /* First sector relative to beginning of file */
      sectorFirst = Fat32File[FileHandle].FilePos / 512;
      /* Cursor positon within the first sector (remainder of division) */
      sectorFOffset = Fat32File[FileHandle].FilePos - (512 * sectorFirst);
      /* Last sector relative to beginning of file */
      sectorLast = (Fat32File[FileHandle].FilePos + Size) / 512;
      /* Cursor positon within the last sector  (remainder of division) */
      sectorLOffset = (Fat32File[FileHandle].FilePos + Size) -
          (512 * sectorLast);
      /* Computing number of sectors to read */
      nSectors = 1 + (sectorLast - sectorFirst);
      /* Transforming relative first sector into absolute value */
      sectorFirst += Fat32File[FileHandle].SectorFirst;
      sectorLast  += Fat32File[FileHandle].SectorFirst;

      if(nSectors <= AFATFS_FILEBUFFER_SIZE)
      {

        switch(state[Disk])
        {
        case READ_FIRST_SECTOR:
          /* 1 - Reading first sector from the disk */
          returncode = Disk_List[Disk].Read(Fat32File[FileHandle].Buffer,
              sectorFirst , 1);
          if(returncode == ANSWERED_REQUEST)
          {
            returncode = OPERATION_RUNNING;
            /* 2 - Copying data in the correct position of file buffer */
            /* Updating the first file sector with new data */
            if(nSectors == 1){
              /* If there is only one sector to write */
              memcpy(Fat32File[FileHandle].Buffer + sectorFOffset, Buffer,
                  sectorLOffset - sectorFOffset);
              state[Disk] = WRITE_DATA;
            }else{
              /* If there is more than one sector to write */
              /* (512 - sectorFOffset) is the qty of new data writen to the
               * 1st sector in this case*/
              memcpy(Fat32File[FileHandle].Buffer + sectorFOffset, Buffer,
                  512 - sectorFOffset);
              state[Disk] = READ_LAST_SECTOR;
            }
          }
          break;

        case READ_LAST_SECTOR:
          /* 3 - Reading last sector from the disk */
          returncode = Disk_List[Disk].Read(Fat32File[FileHandle].Buffer +
              ((nSectors - 1) * 512) , sectorLast , 1);
          if(returncode == ANSWERED_REQUEST)
          {
            returncode = OPERATION_RUNNING;
            /* 4 - Updating file buffer with the new data supplyed */
            /* Updating the remaining file sectors with new data */
            /* (512 - sectorFOffset) is the qty of new data writen to the
             * 1st sector in this case*/
            memcpy(Fat32File[FileHandle].Buffer + 512,
                Buffer + (512 - sectorFOffset), Size - (512 - sectorFOffset));
            state[Disk] = WRITE_DATA;
          }
          else if(returncode >= RETURN_ERROR_VALUE)
          {
            state[Disk] = READ_FIRST_SECTOR;
          }
          break;

        case WRITE_DATA:
          /* 5 - Writing data back to the disk */
          returncode = Disk_List[Disk].Write(Fat32File[FileHandle].Buffer,
              sectorFirst, nSectors);
          if(returncode == ANSWERED_REQUEST)
          {
            /* Computing if file size increased */
            if((Fat32File[FileHandle].FilePos + Size) >
            Fat32File[FileHandle].LogicalSize)
            {
              returncode = OPERATION_RUNNING;
              FatDisk[Disk].RootDir[Partition][Entry].Size =
                  Fat32File[FileHandle].FilePos + Size;
              state[Disk] = UPDATE_ENTRY;
            }
            else
            {
              Fat32File[FileHandle].FilePos += Size;
              state[Disk] = READ_FIRST_SECTOR;
            }
          }else if(returncode >= RETURN_ERROR_VALUE){
            state[Disk] = READ_FIRST_SECTOR;
          }
          break;

        case UPDATE_ENTRY:
          returncode = AFATFS_WriteRootDirEntry(Disk, Partition);
          if(returncode == ANSWERED_REQUEST){
            Fat32File[FileHandle].FilePos += Size;
            Fat32File[FileHandle].LogicalSize = Fat32File[FileHandle].FilePos;
            state[Disk] = READ_FIRST_SECTOR;
          }else if(returncode >= RETURN_ERROR_VALUE){
            state[Disk] = READ_FIRST_SECTOR;
          }
          break;

        default:
          state[Disk] = READ_FIRST_SECTOR;
          break;
        }


      }

      if(returncode == ANSWERED_REQUEST){
        /* Updating cluster position */
        Fat32File[FileHandle].ClusterPrev = Fat32File[FileHandle].ClusterPos;
        /* Updating sector positon */
        Fat32File[FileHandle].SectorPrev = Fat32File[FileHandle].SectorPos;
        Fat32File[FileHandle].SectorPos = sectorFirst;
      }

    }

  }else{
    returncode = ERR_DISABLED;
  }

  return returncode;

}
