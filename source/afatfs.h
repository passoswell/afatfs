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
 * @brief Internal file buffer size as a multiple of sector size.
 */
#ifndef AFATFS_FILEBUFFER_SIZE
#define AFATFS_FILEBUFFER_SIZE                                                 8
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


#endif /* AFATFS_H */
