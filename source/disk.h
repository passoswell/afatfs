/**
 * @file  disk.h
 * @date  06-January-2021
 * @brief Wrapper header file for storage unities.
 *
 * @author
 * @author
 */


#ifndef DISK_H
#define DISK_H

#include <stdint.h>
#include "stdstatus.h"

EStatus_t DISK_IntHwInit(uint8_t Disk);

EStatus_t DISK_ExtHwConfig(uint8_t Disk);

EStatus_t DISK_Read(uint8_t Disk, uint8_t *Buffer, uint32_t Sector, uint32_t Count);

EStatus_t DISK_Write(uint8_t Disk, uint8_t *Buffer, uint32_t Sector, uint32_t Count);

EStatus_t DISK_ReadSpecs(uint8_t Disk);

#endif /* DISK_H */
