/**
 * @file  afatfs.h
 * @date  05-January-2021
 * @brief .
 *
 * @author
 * @author
 */


#ifndef AFATFS_H
#define AFATFS_H


#include <stdint.h>
#include "stdstatus.h"


/**
 * @brief Max number of disks allowed.
 */
#ifndef AFATS_MAX_DISKS
#define AFATS_MAX_DISKS                                                        1
#endif


/**
 * @brief Max number of partitions per disk allowed.
 */
#ifndef AFATS_MAX_PARTITIONS
#define AFATS_MAX_PARTITIONS                                                   4
#endif

/**
 * @brief Minimum allowed sector size.
 */
#ifndef AFATFS_MIN_SECTOR_SIZE
#define AFATFS_MIN_SECTOR_SIZE                                               512
#endif

/**
 * @brief Maximum allowed sector size (defines the size of some buffers).
 */
#ifndef AFATFS_MAX_SECTOR_SIZE
#define AFATFS_MAX_SECTOR_SIZE                                               512
#endif


/**
 * @brief Max number of files opened simultaneously per disk.
 */
#ifndef AFATS_MAX_FILES
#define AFATS_MAX_FILES                                                        4
#endif


/**
 * @brief Internal file buffer size as a multiple of sector size.
 */
#ifndef AFATFS_FILEBUFFER_SIZE
#define AFATFS_FILEBUFFER_SIZE                                                 4
#endif



#if AFATFS_MIN_SECTOR_SIZE > AFATFS_MAX_SECTOR_SIZE
#error AFATFS_MAX_SECTOR_SIZE smaller than AFATFS_MIN_SECTOR_SIZE.
#endif


/**
 * @brief Test function.
 */
void AFATFS_Test(void);


/**
 * @brief  This routine configures a specified disk.
 * @param  Disk : A number that will identify the disk.
 * @param  DiskIO : Pointers to functions that give access to the disk.
 * @retval EStatus_t
 */
EStatus_t AFATFS_Mount(uint8_t Disk);


/**
 * @brief  This routine opens a file.
 * @param  Disk : A number that will identify the disk.
 * @param  Partition : A number that will identify a partition.
 * @param  FileName : A string containing the file name.
 * @param  Mode : The mode in wich the file will be opened.
 * @param  FileHandle : A value returned by the function to identify the file.
 * @retval EStatus_t
 */
EStatus_t AFATFS_Open(uint8_t Disk, uint8_t Partition, char *FileName,
    uint8_t Mode, uint8_t *FileHandle);


/**
 * @brief  This routine reads data from a file.
 * @param  FileHandle : A handle to the file.
 * @param  Buffer : Buffer where data will be stores.
 * @param  Size : Number of bytes desired.
 * @param  BytesRead : Number of bytes efectively read.
 * @retval EStatus_t
 */
EStatus_t AFATFS_Read(uint8_t FileHandle, uint8_t *Buffer, uint8_t Size,
    uint8_t *BytesRead);



#endif /* AFATFS_H */
