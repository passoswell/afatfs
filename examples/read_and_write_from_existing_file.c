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
