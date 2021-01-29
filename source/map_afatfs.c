#include "map_afatfs.h"
#include "sdcard.h"
/* #include "nand.h */

DiskIO_t Disk_List[] = {
    {SDCARD_IntHwInit, SDCARD_ExtHwConfig, SDCARD_Read, SDCARD_Write, 0},
};

uint32_t Disk_ListSize = sizeof(Disk_List) / sizeof(DiskIO_t);
