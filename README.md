# AFATSF
> This is a library that allows for reading and writing to disks formated as FAT32 in an asynchronous (non blocking) manner.

## Table of contents
* [General info](#general-info)
* [Screenshots](#screenshots)
* [Dependencies](#dependencies)
* [Setup](#setup)
* [Features](#features)
* [Status](#status)
* [Inspiration](#inspiration)
* [Contact](#contact)

## General info
This is a library that allows for reading and writing to disks formated as FAT32 in an asynchronous (non blocking) manner. Most of the file systems available online demand an write or read operation is finished by the time a function call returns. Depending on the case, an operation that access a file  can take multiple seconds. This is not a problem for applications with no time constraints or for those operating above a Real-Time Operating System (RTOS). However, for those trying to avoid the processing overhead and additional memory usage of an RTOS, there is not an option available. With that need in mind and the intent to learn a little about file systems in general, this project was created.

## Screenshots
No screenshots available.

## Dependencies
* Error codes returned by afatfs functions are declared inside "std_headers/stdstatus.h" file on the [utils repository](https://github.com/passoswell/utils).

## Setup
### 1. Edit "map_afatfs.h" file to add your disks.

Inside "map_afatfs.h", add items to the "DISK_Models_t" enumeration to reffer to the disks. If using two disks, you should have two items on the enum. This can be used on the main code to identify the disks by an name instead of an number.

```
/**
 * @brief Customize the list of disks used.
 */
typedef enum
{
  DISK1 = 0, /*!< Use any name you find easier for understanding your code */
  DISK2 = 1, /*!< Use any name you find easier for understanding your code */
  DISK3 = 2, /*!< Use any name you find easier for understanding your code */
}DISK_Models_t;`
```

### 2. Edit "map_afatfs.c" file to add your disks.

Inside "map_afatfs.c", you must include the header files for the disks and fill "Disk_List" variable with pointers to the disks' functions. The functions that must be supplied are:
 - xxxx_IntHwInit, a funtion that configures the hardware that is internal to the microcontroller (eg. I2C, SPI, UART, GPIO)
```
/**
 * @brief  Initializes internal peripherals.
 * @retval EStatus_t
 * @note This routine only initializes the internal hardware and is not enough
 *       for full device usage; the SDCARD_ExtHwConfig has to be called as
 *       well.
 */
EStatus_t SDCARD_IntHwInit( void );
```

 - xxxx_ExtDevConfig, a function that comunicates with the disk to perform any kind of configuration required
```
/**
 * @brief  Initializes external peripheral.
 * @retval EStatus_t
 *@note  This routine only configures the device hardware and is not enough
 *       for full device usage; the SDCARD_IntHwInit must be called first.
 */
EStatus_t SDCARD_ExtHwConfig( void );
```
 - xxxx_Read, a funcion that reads sectors from the disk
```
/**
 * @brief  This routine reads data from memory.
 * @param  DataBuffer : Variable where data read will be stored.
 * @param  Sector : Sector within the memory.
 * @param  NumberOfSectors : Number of sectors to read.
 * @retval EStatus_t
 */
EStatus_t SDCARD_Read(uint8_t *DataBuffer, uint32_t Sector,
    uint32_t NumberOfSectors);
```
 - xxxx_Write, a funcion that writes data to sectors on the disk
```
/**
 * @brief  This routine writes data to memory.
 * @param  DataBuffer : Variable where data to write is stored.
 * @param  Sector : Sector within the memory.
 * @param  NumberOfSectors : Number of sectors to read.
 * @retval EStatus_t
 */
EStatus_t SDCARD_Write(uint8_t *DataBuffer, uint32_t Sector,
    uint32_t NumberOfSectors);
```
 - xxxx_ReadSpecs, a funcion that fills a structure with disk specs (not implemented, for future purposes only)
```
```
 
 The variable should, for the three disks on the example, look similar to the code below.
```
#include "map_afatfs.h"
#include "disk1.h"
#include "disk2.h"
#include "disk3.h"

DiskIO_t Disk_List[] = {
    {DISK1_IntHwInit, DISK1_ExtHwConfig, DISK1_Read, DISK1_Write, 0},
    {DISK2_IntHwInit, DISK2_ExtHwConfig, DISK2_Read, DISK2_Write, 0},
    {DISK3_IntHwInit, DISK3_ExtHwConfig, DISK3_Read, DISK3_Write, 0},
};

uint32_t DIsk_ListSize = sizeof(Disk_List) / sizeof(DiskIO_t);

```

## Code Examples

```
#include "afatfs.h"

void main(void)
{
  enum{DISK_INIT = 0, OPEN_FILE, READ_FILE, WRITE_FILE, READ_FILE_2, NOP };
  EStatus_t returncode;
  static uint8_t state = DISK_INIT;
  static uint8_t fileHandle;
  static uint8_t Buffer[4100];
  static uint32_t DataRead;
  static uint16_t counter = 0;

  while(1)
  {

    switch(state)
    {
    /* Configuring DISK1 and enabling its access */
    case DISK_INIT:
      returncode = AFATFS_Mount(DISK1);
      if(returncode == ANSWERED_REQUEST){
        state = OPEN_FILE;
      }else if(returncode >= RETURN_ERROR_VALUE){
          /* An error occurred */
      }
      break;

    case OPEN_FILE:
    /* Opening an existing file on DISK1 called "ASCII.TXT" */
      returncode = AFATFS_Open(DISK1, 0, "ASCII.TXT", 0, &fileHandle);
      if(returncode == ANSWERED_REQUEST){
        AFATFS_Seek(fileHandle, 5000); /* Setting cursor to position 5000 on file */
        state = READ_FILE;
      }else if(returncode >= RETURN_ERROR_VALUE){
          /* An error occurred */
      }
      break;

    case READ_FILE:
    /* Reading 4095 bytes from "ASCII.TXT" file */
      returncode = AFATFS_Read(fileHandle, Buffer, 4095, &DataRead);
      if(returncode == ANSWERED_REQUEST){
        AFATFS_Seek(fileHandle, 510); /* Setting cursor to position 510 on file */
        state = WRITE_FILE;
      }else if(returncode >= RETURN_ERROR_VALUE){
          /* An error occurred */
      }
      break;

    case WRITE_FILE:
    /* Writing the value of a counter to the file multiple times */
      sprintf((char *)Buffer, "%5d\r\n", counter);
      returncode = AFATFS_Write(fileHandle, Buffer, 7);
      if(returncode == ANSWERED_REQUEST){
        counter++;
        if(counter > 10){
          AFATFS_Seek(0, 510); /* Setting cursor to position 510 on file again */
          state = READ_FILE_2;
        }
      }else if(returncode >= RETURN_ERROR_VALUE){
          /* An error occurred */
      }
      break;

    case READ_FILE_2:
    /* Reading the bytes we just wrote (and a little bit more */
      returncode = AFATFS_Read(fileHandle, Buffer, 512, &DataRead);
      if(returncode == ANSWERED_REQUEST){
        state = NOP;
      }else if(returncode >= RETURN_ERROR_VALUE){
          /* An error occurred */
      }
      break;

    case NOP:
    /* End of test */
      break;

    default:
      state = DISK_INIT;
      break;
    }

  }

}
```


## Features and limitations
List of features ready and limitations
* Open and read files alread existing in the root directory, subfolders not implemented
* Write to files alread existing in the root directory
* Can read and edit only the first 16 entries (one sector) of the root directory
* Can read only the first cluster of a file, multicluster read not implemented
* Can write only to the first cluster of a file, multicluster write not implemented
* Number of files opened simultaneously, number of disks and other parameters are configurable through macros in "afatfs.h"
* Map files (header and source) used to add disks so the library can use then

To-do list:
* Expand the number of entries read from root directory from one sector to one cluster
* Add functions to create new files, and root directories
* Add support to multi-cluster sized files
* Implement the extended name size for files and folders. Currently limited to 8 characters for the name and 3 for the extension (8.3)
* Implement access to files inside subfolders

## Status
Project is: _in progress_. Functions are still being implemented.

## Inspiration

For some implementations:
* The first inspiration is a widely used library is [FatFs - Generic FAT Filesystem Module by Chan](http://elm-chan.org/fsw/ff/00index_e.html), which does a great job at allowing embedded systems of all sizes to read from and write to FAT formatted disks, and stil being updated.
* An rare option, probably the only implementation publicly available on the internet, of a non-blocking, asynchronous FAT library [AsyncFatFS](https://github.com/thenickdude/asyncfatfs). This implementatio has been used, for instance, on the [Cleanflight / Betaflight's "blackbox" logging system](https://github.com/betaflight/betaflight), and seems to be maintained uo to this commit.

For some tutorialson the workings of FAT file systems:
* [Wyoos operating system](http://wyoos.org/) and [the series of Youtube videos](https://www.youtube.com/watch?v=IGpUdIX-Z5A&list=PLHh55M_Kq4OApWScZyPl5HhgsTJS9MZ6M&index=31) about its implementation
* [Code and Life](https://codeandlife.com/2012/04/02/simple-fat-and-sd-tutorial-part-1/) web tutorial on interfacing FAT16 SDCards and the tutorials it references too.
* [Paul's 8051 Code Library: Understanding the FAT32 Filesystem](https://www.pjrc.com/tech/8051/ide/fat32.html) web tutorial.

The template for this readme file was created by [@flynerdpl](https://www.flynerd.pl/).

## Contact
Created by [@passoswell](https://github.com/passoswell).
