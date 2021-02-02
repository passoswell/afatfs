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
#include "setup.h"


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
#define AFATS_MAX_FILES                                                        2
#endif


/**
 * @brief Internal file buffer size as a multiple of sector size.
 */
#ifndef AFATFS_FILEBUFFER_SIZE
#define AFATFS_FILEBUFFER_SIZE                                                 8
#endif



#if AFATFS_MIN_SECTOR_SIZE > AFATFS_MAX_SECTOR_SIZE
#error AFATFS_MAX_SECTOR_SIZE smaller than AFATFS_MIN_SECTOR_SIZE.
#endif


/**
 * @brief  This routine configures a specified disk.
 * @param  Disk : A number that will identify the disk.
 * @param  DiskIO : Pointers to functions that give access to the disk.
 * @retval EStatus_t
 */
EStatus_t AFATFS_Mount(uint8_t Disk);


/**
 * @brief  This routine creates an empty file on root directory.
 * @param  Disk : A number that will identify the disk.
 * @param  Partition : A number that will identify a partition.
 * @param  FileName : A string containing the file name.
 * @param  Mode : The mode in wich the file will be created.
 * @param  FileHandle : A value returned by the function to identify the file.
 * @retval EStatus_t
 */
EStatus_t AFATFS_Create(uint8_t Disk, uint8_t Partition, char *FileName,
    uint8_t Mode, uint8_t *FileHandle);


/**
 * @brief  This routine opens a file from root directory.
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
 * @brief  This routine closes a file from root directory.
 * @param  Disk : A number that will identify the disk.
 * @param  Partition : A number that will identify a partition.
 * @param  FileHandle : A handle to the file.
 * @note   This function only free the memory used to handle the file. Any
 *         write operation is performed inside AFATFS_Write.
 * @retval EStatus_t
 */
EStatus_t AFATFS_Close(uint8_t Disk, uint8_t Partition, uint8_t *FileHandle);


/**
 * @brief  This moves a file pointer to the specified offset.
 * @param  FileHandle : A handle to the file.
 * @param  Offset : Number in bytes to move the file cursor from the begining of
 *         the file.
 * @retval EStatus_t
 */
EStatus_t AFATFS_Seek(uint8_t FileHandle, uint32_t Offset);


/**
 * @brief  This routine reads data from a file.
 * @param  FileHandle : A handle to the file.
 * @param  Buffer : Buffer where data will be stores.
 * @param  Size : Number of bytes desired.
 * @param  BytesRead : Number of bytes efectively read.
 * @retval EStatus_t
 * @note   The data is read from an offset set by a call to AFATFS_Write or
 *         to AFATFS_Seek
 */
EStatus_t AFATFS_Read(uint8_t FileHandle, uint8_t *Buffer, uint32_t Size,
    uint32_t *BytesRead);


/**
 * @brief  This routine writes data to a file.
 * @param  FileHandle : A handle to the file.
 * @param  Buffer : Buffer with data to write.
 * @param  Size : Number of bytes to write.
 * @retval EStatus_t
 * @note   The data is written to an offset set by a call to AFATFS_Read or
 *         to AFATFS_Seek
 */
EStatus_t AFATFS_Write(uint8_t FileHandle, uint8_t *Buffer, uint32_t Size);

#endif /* AFATFS_H */
