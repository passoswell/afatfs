/**
 * @file  map_afatfs.h
 * @date  05-January-2021
 * @brief .
 *
 * @author
 * @author
 */


#ifndef MAP_AFATFS_H
#define MAP_AFATFS_H

#include <stdint.h>
#include "stdstatus.h"

/**
 * @brief Customize the list of disks used.
 */
typedef enum
{
  SDCARD = 0,
  NAND,
}DISK_Models_t;


/**
 * @brief Pointers to functions that give access to the disk.
 */
typedef struct
{
  EStatus_t (*IntHwInit)(void);

  EStatus_t (*ExtDevConfig)(void);

  EStatus_t (*Read)(uint8_t *Buffer, uint32_t Sector, uint32_t Count);

  EStatus_t (*Write)(uint8_t *Buffer, uint32_t Sector, uint32_t Count);

  EStatus_t (*ReadSpecs)(void);
}DiskIO_t;


extern DiskIO_t Disk_List[];
extern uint32_t DIsk_ListSize;



#endif /* MAP_AFATFS_H */
