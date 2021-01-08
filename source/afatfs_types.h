/**
 * @file  afatfs_types.h
 * @date  07-January-2021
 * @brief .
 *
 * @author
 * @author
 */


#ifndef AFATFS_TYPES_H
#define AFATFS_TYPES_H


#include "afatfs.h"


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
 * @brief List of file attributes recorded on disk.
 */
typedef enum
{
  READ_ONLY    = 0x01, /*!< O.S. should not allow opening for modification */
  HIDDEN       = 0x02, /*!< File or directory removed from normal view */
  SYSTEM       = 0x04, /*!< Indicates the file belongs to the system */
  VOLUME_LABEL = 0x08, /*!< Indicates a volume label */
  SUBDIRECTORY = 0x10, /*!< Subdirectory */
  ARCHIVE      = 0x20, /*!< It is used by backup programs */
}FileAttributes;


/**
 * @brief Complete structure of a primary partition table record.
 */
typedef struct __attribute__((packed))
{
  uint8_t   isBootable;
  uint8_t   StartCHS[3];
  FatType_t FatType;
  uint8_t   EndCHS[3];
  uint32_t  StartLBA;   /*!< Start sector for the partition */
  uint32_t  LengthLBA;  /*!< Size in sectors of the partition */
}PartitionTable_t;


/**
 * @brief Complete (0x5A bytes) BIOS parameter block for FAT32
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
}PartitionParameterTable_t;





/**
 * @brief Complete list of file parameters from file allocation table.
 */
typedef struct __attribute__((packed))
{
  uint8_t Name[8];           /*!< File name (8 chars, padded with spaces) */
  uint8_t Ext[3];            /*!< File extension (3 char, padded with spaces */
  uint8_t Attributes;        /*!< Combination of FAT32 file attributes */
  uint8_t Reserved;
  uint8_t cTimeTenth;
  uint16_t cTime;
  uint16_t cDate;
  uint16_t aTime;
  uint16_t FirstClusterHi;
  uint16_t wTime;
  uint16_t wDate;
  uint16_t FirstClusterLow;
  uint32_t Size;
}DirectoryEntryFat32_t;



/**
 * @brief Simplified structure of a boot sector for FAT32.
 */
typedef struct
{
  FatType_t FatType[AFATS_MAX_PARTITIONS];   /*!< Partitions type */
  uint32_t  StartLBA[AFATS_MAX_PARTITIONS];  /*!< Start sector for the
                                                  partitions */
  uint32_t  LengthLBA[AFATS_MAX_PARTITIONS]; /*!< Size in sectors of the
                                                  partitions */
  uint16_t  Signature;                       /*!< Valid if equal to 0xAA55 */
}ReducedMasterBootRecord_t;



/**
 * @brief Simplified partition parameter block for FAT32.
 */
typedef struct
{
  uint32_t FatStartSector[AFATS_MAX_PARTITIONS];
  uint32_t FatSize[AFATS_MAX_PARTITIONS];
  uint32_t DataStartSector[AFATS_MAX_PARTITIONS];
  uint32_t SectorPerCluster[AFATS_MAX_PARTITIONS];
  uint32_t RootSector[AFATS_MAX_PARTITIONS];
}ReducedPartitionParameterTable_t;

#endif /* AFATFS_TYPES_H */
