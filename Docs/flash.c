/************************************************************************/
/*                                                                      */
/*  AMD Flash Memory Drivers                                            */
/*  File name: FLASH.C                                                  */
/*  Revision:  1.2  1/07/99                                             */
/*                                                                      */
/* Copyright (c) 1999 ADVANCED MICRO DEVICES, INC. All Rights Reserved. */
/* This software is unpublished and contains the trade secrets and      */
/* confidential proprietary information of AMD. Unless otherwise        */
/* provided in the Software Agreement associated herewith, it is        */
/* licensed in confidence "AS IS" and is not to be reproduced in whole  */
/* or part by any means except for backup. Use, duplication, or         */
/* disclosure by the Government is subject to the restrictions in       */
/* paragraph (b) (3) (B) of the Rights in Technical Data and Computer   */
/* Software clause in DFAR 52.227-7013 (a) (Oct 1988).                  */
/* Software owned by                                                    */
/* Advanced Micro Devices, Inc.,                                        */
/* One AMD Place,                                                       */
/* P.O. Box 3453                                                        */
/* Sunnyvale, CA 94088-3453.                                            */
/************************************************************************/
/*  This software constitutes a basic shell of source code for          */
/*  programming all AMD Flash components. AMD                           */
/*  will not be responsible for misuse or illegal use of this           */
/*  software for devices not supported herein. AMD is providing         */
/*  this source code "AS IS" and will not be responsible for            */
/*  issues arising from incorrect user implementation of the            */
/*  source code herein. It is the user's responsibility to              */
/*  properly design-in this source code.                                */
/*                                                                      */ 
/************************************************************************/                                                                        
#include "flash.h"

/*********************************************************************/
/* The purpose of get_flash_memptr() is to return a memory pointer   */
/* which points to the beginning of memory space allocated for the   */
/* flash.  All function pointers are then referenced from this       */
/* pointer. 							     */
/*                                                                   */
/* Different systems will implement this in different ways:          */
/* possibilities include:                                            */
/*  - A direct memory pointer                                        */
/*  - A pointer to a memory map                                      */
/*  - A pointer to a hardware port from which the linear             */
/*    address is translated                                          */
/*  - Output of an MMU function / service                            */
/*                                                                   */
/* Also note that this function expects the pointer to a specific    */
/* sector of the device.  This can be provided by dereferencing      */
/* the pointer from a translated offset of the sector from a         */
/* global base pointer (e.g. flashptr = base_pointer + sector_offset)*/
/*                                                                   */
/* Important: Many AMD flash devices need both bank and or sector    */
/* address bits to be correctly set (bank address bits are A18-A16,  */
/* and sector address bits are A18-A12, or A12-A15).  Flash parts    */
/* which do not need these bits will ignore them, so it is safe to   */
/* assume that every part will require these bits to be set.         */
/*********************************************************************/

extern unsigned char FAR *get_flash_memptr(int sector);

struct flashinfo *meminfo; /* A pointer into a specific field defined
                              in memdesc below */
/*********************************************************************/
/* Memdesc is a global storage array for sector and bank parameters. */
/* This can be found and inputted directly from any AMD specification*/
/* sheet.  If minimal static data storage is required, all other     */
/* declarations can be removed except for the flash component being  */
/* used in the application.                                          */
/* Four examples are provided of common flash devices                */
/* Also note the specifics to bank / sector locations when dealing   */
/* with DL flash parts.                                              */
/*********************************************************************/

struct flashinfo memdesc[] = {
	 /* DL800T */
	{"Am29DL800T", 0L, 0, 22, 14, 0 ,{
	{65536,     0x00000,    2},
	{65536,     0x10000,    2},
	{65536,     0x20000,    2},
	{65536,     0x30000,    2},
	{65536,     0x40000,    2},
	{65536,     0x50000,    2},
	{65536,     0x60000,    2},
	{65536,     0x70000,    2},
	{65536,     0x80000,    2},
	{65536,     0x90000,    2},
	{65536,     0xA0000,    2},
	{65536,     0xB0000,    2},
	{65536,     0xC0000,    2},
	{65536,     0xD0000,    2},
	{16384,     0xE0000,    1},
	{32768,     0xE4000,    1},
	{8192,      0xEC000,    1},
	{8192,      0xEE000,    1},
	{8192,      0xF0000,    1},
	{8192,      0xF2000,    1},
	{32768,     0xF4000,    1},
	{16384,     0xFC000,    1}
	}
	 },
	 /* DL800B */
	{"Am29DL800B", 0L, 0, 22, 0, 8 ,{
	{16384,     0x00000,    1},
	{32768,     0x04000,    1},
	{8192,      0x0C000,    1},
	{8192,      0x0E000,    1},
	{8192,      0x10000,    1},
	{8192,      0x12000,    1},
	{32768,     0x14000,    1},
	{16384,     0x1C000,    1},
	{65536,     0x20000,    2},
	{65536,     0x30000,    2},
	{65536,     0x40000,    2},
	{65536,     0x50000,    2},
	{65536,     0x60000,    2},
	{65536,     0x70000,    2},
	{65536,     0x80000,    2},
	{65536,     0x90000,    2},
	{65536,     0xA0000,    2},
	{65536,     0xB0000,    2},
	{65536,     0xC0000,    2},
	{65536,     0xD0000,    2},
	{65536,     0xE0000,    2},
	{65536,     0xF0000,    2}
	}
	},
	 /* LV800T */
	{"Am29LV800T", 0L, 0, 19, 0, 4 ,{
	{65536,     0x00000,    1},
	{65536,     0x10000,    1},
	{65536,     0x20000,    1},
	{65536,     0x30000,    1},
	{65536,     0x40000,    1},
	{65536,     0x50000,    1},
	{65536,     0x60000,    1},
	{65536,     0x70000,    1},
	{65536,     0x80000,    1},
	{65536,     0x90000,    1},
	{65536,     0xA0000,    1},
	{65536,     0xB0000,    1},
	{65536,     0xC0000,    1},
	{65536,     0xD0000,    1},
	{65536,     0xE0000,    1},
	{32768,     0xF0000,    1},
	{8192,      0xF8000,    1},
	{8192,      0xFA000,    1},
	{16384,     0xFC000,    1}
	}
	},
	 /* LV800B */
	{"Am29LV800B", 0L, 0, 19, 0, 4 ,{
	{16384,     0x00000,    1},
	{8192,      0x04000,    1},
	{8192,      0x06000,    1},
	{32768,     0x08000,    1},
	{65536,     0x10000,    1},
	{65536,     0x20000,    1},
	{65536,     0x30000,    1},
	{65536,     0x40000,    1},
	{65536,     0x50000,    1},
	{65536,     0x60000,    1},
	{65536,     0x70000,    1},
	{65536,     0x90000,    1},
	{65536,     0xA0000,    1},
	{65536,     0xB0000,    1},
	{65536,     0xC0000,    1},
	{65536,     0xD0000,    1},
	{65536,     0xE0000,    1},
	{65536,     0xF0000,    1}
	}
	},
	/* LV160B - same as LV800B, but limited to 2MB addressing window*/
	{"Am29LV160B", 0L, 0, 19, 0, 0 ,{
	{16384,     0x00000,    1},
	{8192,      0x04000,    1},
	{8192,      0x06000,    1},
	{32768,     0x08000,    1},
	{65536,     0x10000,    1},
	{65536,     0x20000,    1},
	{65536,     0x30000,    1},
	{65536,     0x40000,    1},
	{65536,     0x50000,    1},
	{65536,     0x60000,    1},
	{65536,     0x70000,    1},
	{65536,     0x80000,    1},
	{65536,     0x90000,    1},
	{65536,     0xA0000,    1},
	{65536,     0xB0000,    1},
	{65536,     0xC0000,    1},
	{65536,     0xD0000,    1},
	{65536,     0xE0000,    1},
	{65536,     0xF0000,    1}
	}
	},
	/* LV400B */
	{"Am29LV400B", 0L, 0, 11, 0, 0 ,{
	{16384,     0x00000,    1},
	{8192,      0x04000,    1},
	{8192,      0x06000,    1},
	{32768,     0x08000,    1},
	{65536,     0x10000,    1},
	{65536,     0x20000,    1},
	{65536,     0x30000,    1},
	{65536,     0x40000,    1},
	{65536,     0x50000,    1},
	{65536,     0x60000,    1},
	{65536,     0x70000,    1},
	}
	}
};

/*********************************************************************/
/* init_flash() performs any initial memory allocation, setting of   */
/* parameters, and also initializes the appropriate sector table     */
/* defined above.                                                    */
/* Memory mapping or allocation specifics will need to be provided   */
/* by the programmer of the driver, and will be highly dependant on  */
/* on the system being used.                                         */
/*********************************************************************/

unsigned int init_flash(unsigned int flashtype)
{
  /* Insert any memory initialization or memory mapping here */

  meminfo = &memdesc[flashtype];

  return(1);
}

/*********************************************************************/
/* Flash_command() is the main driver function.  It performs         */
/* every possible command available to AMD B revision                */
/* flash parts. Note that this command is not used directly, but     */
/* rather called through the API wrapper functions provided below.   */
/* This function can be called directly if desired (see wrapper      */
/* functions defined below.                                          */
/*********************************************************************/

void flash_command(int command, int sector, int offset,
						 unsigned int data)
{
	 static dword base;    /* base offset for our sector */
	 static word *selp;
	 static dword sechigh;
	 static word far *flashptr;  /* flash window (64K bytes) */
	 static int oldsector = -1;
	 int retry;
       
         /**************************************************************/
	 /* IMPORTANT: Note that flashptr is defined as a WORD pointer */
         /* If BYTE pointers are used, the command tables will have to */
         /* be remapped						       */
         /* Note 1: flashptr is declared far - if system does not      */
         /*         support far pointers, this will have to be changed */
         /* Note 2: flashptr is declared static to avoid calling       */
         /*         get_flash_memptr() on successive sector accesses   */
         /**************************************************************/

	 /******************************************************************/
	 /* On systems where bus glitching is prevalent, some long command */
  	 /* strings may be interrupted and cause the command to fail (this */
         /* is most probable on six cycle commands such as chip erase). In */
         /* order to ensure that flash_command executes the command        */
         /* properly, it may be necessary to issue the command more than   */
         /* once in order for it to be accepted by the flash device.  In   */
         /* these cases it is recommended that the retry number be made    */
         /* positive (such as 1 or 2), so that flash_command will try      */
         /* to issue the command more than once.  Keep in mind that this   */
         /* will only be attempted if the command fails in the first       */
         /* attempt.                                                       */
         /******************************************************************/
         
         static int retrycount[] = {0,0,0,0,0,0,0,0,0,0};

	 retry = retrycount[command];

	 if(sector != oldsector) {
		flashptr = (word far *) get_flash_memptr(sector);
	 }

again:

	 if (command == FLASH_SELECT) {
		return;
	 } else if (command == FLASH_RESET || command > FLASH_LASTCMD) {
		  flashptr[0] = 0xF0;   /* assume reset device to read mode */
	 } else if (command == FLASH_ESUSPEND) {
		  flashptr[0] = 0xB0;   /* suspend sector erase */
	 } else if (command == FLASH_ERESUME) {
		  flashptr[0] = 0x30;   /* resume suspended sector erase */
	 } else if (command == FLASH_UBPROG) {
		  flashptr[0] = 0xA0;
		  flashptr[((meminfo->sec[sector].base)+offset)/2] = data;
	 } else if (command == FLASH_UBRESET) {
		  flashptr[0] = 0x90;
		  flashptr[0] = 0x00;
	 }
	 else {
		  flashptr[0x555] = 0xAA;       /* unlock 1 */
		  flashptr[0x2AA] = 0x55;       /* unlock 2 */
		  switch (command) {
			 case FLASH_AUTOSEL:
				flashptr[0x555] = 0x90;
				break;
			 case FLASH_PROG:
				flashptr[0x555] = 0xA0;
				flashptr[((meminfo->sec[sector].base)+offset)/2] = data;
				break;
			 case FLASH_CERASE:
				flashptr[0x555] = 0x80;
				flashptr[0x555] = 0xAA;
				flashptr[0x2AA] = 0x55;
				flashptr[0x555] = 0x10;
				break;
			 case FLASH_SERASE:
				flashptr[0x555]  = 0x80;
				flashptr[0x555]  = 0xAA;
				flashptr[0x2AA]  = 0x55;
				flashptr[(meminfo->sec[sector].base)/2] = 0x30;
				break;
			 case FLASH_UB:
				flashptr[0x555] = 0x20;
				break;
		  }
	 }

	 if (retry-- > 0 && flash_status(flashptr) == STATUS_READY) {
		goto again;
	 }
}
/*********************************************************************/
/* Flash_write extends the functionality of flash_program() by       */
/* providing an faster way to program multiple data words, without   */
/* needing the function overhead of looping algorithms which         */
/* program word by word.  This function utilizes fast pointers       */
/* to quickly loop through bulk data.                                */
/*********************************************************************/
int flash_write(int sector, unsigned offset, byte *buf, 
                int nbytes, int ub)
{
	 word far *flashptr; /* flash window */
	 word far *src, *dst;
	 int stat;
	 int retry = 0, retried = 0;

	 flashptr = (word *)get_flash_memptr(sector);
	 dst = flashptr + offset/2;   /* (byte offset) */
	 src = (word *)buf;

	 if ((nbytes | offset) & 1) {
		return -1;
	 }

again:

	 /* Check to see if we're in unlock bypass mode */
         if (ub == FALSE)
		flashptr[0] = 0xF0;  /* reset device to read mode */

	 while ((stat = flash_status(flashptr)) == STATUS_BUSY) {}
	 if (stat != STATUS_READY) {
		return (char *)src - buf;
	 }

	 while (nbytes > 0) {
		if (ub == FALSE){
		  flashptr[0x555] = 0xAA;      /* unlock 1 */
		  flashptr[0x2AA] = 0x55;      /* unlock 2 */
		}
		flashptr[0x555] = 0xA0;
		*dst++ = *src++;
		while ((stat = flash_status(flashptr)) == STATUS_BUSY) {}
		if (stat != STATUS_READY) break;
		nbytes -= 2;
	 }

	 if (stat != STATUS_READY || nbytes != 0) {
		if (retry-- > 0) {
		  ++retried;
		  --dst, --src;   /* back up */
		  goto again;     /* and retry the last word */
		}
		if (ub == FALSE)
                  flash_command(FLASH_RESET,sector,0,0);
	 }

	 return (char *)src - buf;
}

/*********************************************************************/
/* Flash_status utilizes the DQ6, DQ5, and DQ3 polling algorithms    */
/* described in the flash data book.  It can quickly ascertain the   */
/* operational status of the flash device, and return an             */
/* appropriate status code (defined in flash.h)                      */
/*********************************************************************/
 
int flash_status(word far *fp)
{
	 unsigned char d, t;
	 int retry = 1;

again:

	 d = *fp;        /* read data */
	 t = d ^ *fp;    /* read it again and see what toggled */

	 if (t == 0) {           /* no toggles, nothing's happening */
		return STATUS_READY;
	 }
	 else if (t == 0x04) { /* erase-suspend */
		if (retry--) goto again;    /* may have been write completion */
		return STATUS_ERSUSP;
	 }
	 else if (t & 0x40) {
		if (d & 0x20) {     /* timeout */
		  return STATUS_TIMEOUT;
		}
		else {
		  return STATUS_BUSY;
		}
	 }

	 if (retry--) goto again;    /* may have been write completion */

	 return STATUS_ERROR;
}
/*********************************************************************/
/* BEGIN API WRAPPER FUNCTIONS                                       */
/*********************************************************************/
/* Flash_sector_erase() will erase a single sector dictated by the   */
/* sector parameter.                                                 */
/* Note: this function will merely BEGIN the erase program.  Code    */
/* execution will immediately return to the calling function         */
/*********************************************************************/

byte flash_sector_erase(byte sector)
{
	flash_command(FLASH_SERASE,sector,0,0);
	return(1);
}

/*********************************************************************/
/* Flash_sector_erase_int() is identical to flash_sector_erase(),    */
/* except it will wait until the erase is completed before returning */
/* control to the calling function.  This can be used in cases which */
/* require the program to hold until a sector is erased, without     */
/* adding the wait check external to this function.                  */
/*********************************************************************/

byte flash_sector_erase_int(byte sector)
{
	flash_command(FLASH_SERASE,sector,0,0);
	while (flash_status(get_flash_memptr(sector))
			 == STATUS_BUSY) { }
	return(1);
}

/*********************************************************************/
/* flash_reset() will reset the flash device to reading array data.  */
/* It is good practice to call this function after autoselect        */
/* sequences had been performed.                                     */
/*********************************************************************/

byte flash_reset(void)
{
  flash_command(FLASH_RESET,1,0,0);
  return(1);
}

/*********************************************************************/
/* flash_get_device_id() will perform an autoselect sequence on the  */
/* flash device, and return the device id of the component.          */
/* This function automatically resets to read mode.                  */
/*********************************************************************/

word flash_get_device_id(byte sector)
{
	 word far *fwp; /* flash window */
	 word answer;

	 fwp = (word *)get_flash_memptr(sector);

	 flash_command(FLASH_AUTOSEL,sector,0,0);
	 answer = *(fwp++);
	 
         flash_command(FLASH_RESET,sector,0,0);   /* just to be safe */
	 return( (word) answer );
}

/*********************************************************************/
/* flash_get_manuf_code() will perform an autoselect sequence on the */
/* flash device, and return the manufacturer code of the component.  */
/* This function automatically resets to read mode.                  */
/*********************************************************************/

byte flash_get_manuf_code(byte sector)
{
	 word far *fwp; /* flash window */
	 word answer;

	 fwp = (word *)get_flash_memptr(sector);

	 flash_command(FLASH_AUTOSEL,sector,0,0);
         answer = *fwp;

         flash_command(FLASH_RESET,sector,0,0);   /* just to be safe */	
	 return( (byte) (answer & 0x00FF) );
}

/*********************************************************************/
/* flash_sector_protect_verify() performs an autoselect command      */
/* sequence which checks the status of the sector protect CAM        */
/* to check if the particular sector is protected.  Function will    */
/* return a '0' is the sector is unprotected, and a '1' if it is     */
/* protected.                                                        */
/*********************************************************************/

byte flash_sector_protect_verify(byte sector)
{
	 word far *fwp; /* flash window */
	 byte answer;

	 fwp = (word *)get_flash_memptr(sector);

	 flash_command(FLASH_AUTOSEL,sector,0,0);

	 fwp += ((meminfo->sec[sector].base)/2);
	 fwp += 2;

	 answer = (byte) (*fwp & 0x0001); /* Only need DQ0 to check */

	 flash_command(FLASH_RESET,sector,0,0);

	 return( (byte) answer );
}

/*********************************************************************/
/* flash_get_status() will return the current operational status of  */
/* the flash device.  A list of return codes is outlined in flash.h  */
/* Note: for DL parts, status will be bank dependent.                */
/*********************************************************************/

byte flash_get_status(byte sector)
{
	 word far *fwp;

	 fwp = (word *)get_flash_memptr(sector);
	 return flash_status(fwp);
}

/*********************************************************************/
/* flash_chip_erase() will perform a complete erasure of the flash   */
/* device.  							     */
/*********************************************************************/

byte flash_chip_erase(byte sector)
{
  flash_command(FLASH_CERASE,sector,0,0);
  return(1);
}

/*********************************************************************/
/* flash_write_word() will program a single word of data at the      */
/* specified offset from the beginning of the sector parameter.      */
/* Note: this offset must be word aligned, or else errors are        */
/* possible (this can happen when dealing with odd offsets due to    */
/* only partial programming.                                         */
/* Note: It is good practice to check the desired offset by first    */
/* reading the data, and checking to see if it contains 0xFFFF       */
/*********************************************************************/

byte flash_write_word(byte sector, word offset, word data)
{
  flash_command(FLASH_PROG, sector, offset, data);
  return (1);
}

/*********************************************************************/
/* flash_read_word() reads a single word of data from the specified  */
/* offset from the sector parameter.  This function will auto align  */
/* the offset to return word data.                                   */
/*********************************************************************/

word flash_read_word(byte sector, word offset)
{
  word far *fwp;

  flash_command(FLASH_SELECT,sector,0,0);
  fwp = (word *)get_flash_memptr(sector);
  if (offset & 0x0001){ /* Is odd? */
	 offset--;
  }
  fwp += offset;

  return( (word) *fwp );
}
/*********************************************************************/
/* flash_write_string() functions like flash_write_word(), except    */
/* that it accepts a pointer to a buffer to be programmed.  This     */
/* function will align the data to a word offset, then bulk program  */
/* the flash device with the provided data.                          */
/* The maximum buffer size is currently only limited to the data     */
/* size of the numbytes parameter (which in the test system is       */
/* 16 bits = 65535 words                                             */
/* Since the current maximum flash sector size is 64kbits, this      */
/* should not present a problem.                                     */
/*********************************************************************/

byte flash_write_string(byte sector, word offset, 
                        byte *buffer, word numbytes)
{
  word value=0;

  if(numbytes == 0)
	 value = (word) strlen(buffer);
  else
	 value = numbytes;

  if (value & 0x0001)
	 value--; /* Need to make sure we don't overrun buffer */

  flash_write(sector, offset, buffer, value,FALSE);
  return (1);
}

/*********************************************************************/
/* flash_erase_suspend() will suspend an erase process in the        */
/* specified sector.  Array data can then be read from other sectors */
/* (or any other sectors in other banks), and the erase can be       */
/* resumed using the flash_erase_resume function.                    */
/* Note: multiple sectors can be queued for erasure, so long as the  */
/* 80 uS erase suspend window has not terminated (see AMD data sheet */
/* concerning erase_suspend restrictions).                           */
/*********************************************************************/

byte flash_erase_suspend(byte sector)
{
  flash_command(FLASH_ESUSPEND, sector, 0, 0);
  return (1);
}

/*********************************************************************/
/* flash_erase_resume() will resume all pending erases in the bank   */
/* in which the sector parameter is located.                         */
/*********************************************************************/

byte flash_erase_resume(byte sector)
{
  flash_command(FLASH_ERESUME, sector, 0, 0);
  return (1);
}

/*********************************************************************/
/* flash_get_sector_size() is provided for cases in which the size   */
/* of a sector is required by a host application.  The sector size   */
/* (in bytes) is returned in the data location pointed to by the     */
/* 'size' parameter.                                                 */
/*********************************************************************/

byte flash_get_sector_size(byte sector, dword *size)
{
  *size = meminfo->sec[sector].size;
  return(1);
}

/*********************************************************************/
/* UNLOCK BYPASS FUNCTIONS                                           */
/*********************************************************************/
/* Unlock bypass mode is useful whenever the calling application     */
/* wished to program large amounts of data in minimal time.  Unlock  */
/* bypass mode remove half of the bus overhead required to program   */
/* a single word, from 4 cycles down to 2 cycles.  Programming of    */
/* individual bytes does not gain measurable benefit from unlock     */
/* bypass mode, but programming large strings can see a significant  */
/* decrease in required programming time.                            */
/*********************************************************************/

/*********************************************************************/
/* flash_ub() places the flash into unlock bypass mode.  This        */
/* is REQUIRED to be called before any of the other unlock bypass    */
/* commands will become valid (most will be ignored without first    */
/* calling this function.                                            */
/*********************************************************************/
 
byte flash_ub(byte sector)
{
  flash_command(FLASH_UB, sector, 0, 0);
  return(1);
}

/*********************************************************************/
/* flash_write_word_ub() programs a single word using unlock bypass  */
/* mode.  Note that the calling application will see little benefit  */
/* from programming single words using this mode (outlined above)    */
/*********************************************************************/

byte flash_write_word_ub(byte sector, word offset, word data)
{
  flash_command(FLASH_UBPROG, sector, offset, data);
  return (1);
}

/*********************************************************************/
/* flash_write_string_ub() behaves in the exact same manner as       */
/* flash_write_string() (outlined above), expect that it utilizes    */
/* the unlock bypass mode of the flash device.  This can remove      */
/* significant overhead from the bulk programming operation, and     */
/* when programming bulk data a sizeable performance increase can be */
/* observed.                                                         */
/*********************************************************************/

byte flash_write_string_ub(byte sector, word offset,
   			   byte *buffer, word numbytes)
{
  word value=0;

  if(numbytes == 0)
	 value = (word) strlen(buffer);
  else
	 value = numbytes;

  if (value & 0x0001)
	 value--; /* Need to make sure we don't overrun buffer */

  flash_write(sector, offset, buffer, value,TRUE);
  return (1);
}

/*********************************************************************/
/* flash_reset_ub() is required to remove the flash from unlock      */
/* bypass mode.  This is important, as other flash commands will be  */
/* ignored while the flash is in unlock bypass mode.                 */
/*********************************************************************/

byte flash_reset_ub(void)
{
  flash_command(FLASH_UBRESET,1,0,0);
  return(1);
}

/*********************************************************************/
/* Usefull funtion to return the number of sectors in the device.    */
/* Can be used for functions which need to loop among all the        */
/* sectors, or wish to know the number of the last sector.           */
/*********************************************************************/

void flash_get_numsectors(int *num)
{
  *num = meminfo->nsect;
}

/*********************************************************************/