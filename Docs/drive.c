/************************************************************************/
/*                                                                      */
/*  AMD Flash Memory Drivers                                            */
/*  File name: DRIVE.C                                                  */
/*  Revision:  1.1  5/07/98                                             */
/*                                                                      */
/* Copyright (c) 1998 ADVANCED MICRO DEVICES, INC. All Rights Reserved. */
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

#include <stdio.h>
#include <stdlib.h>  /* For randomize function */
#include "flash.h"

void main(void)
{
  byte test;
  byte manuf_id;
  word device_id;
  byte far *str;
  word test2,i;
  dword sec_size=0;
  long count=0;

  printf("Beginning test.....\n\n");

  /* First, poke around the memory map to see what kind of
     flash is installed in the socket...assume a DL800B */

  /* The purpose of init_flash is to perform any system memory
     mapping required, and to set up pointers to those region(s).
     init_flash() also selects the proper sector organization
     table defined in flash.c 
     Note: init_flash() will need to be provided by users of
     the flash.c routines */

  if(!init_flash(AM29DL800B)) {
	 exit(1);
  }

  /* Verify the manufacturer code is indeed 0x01 for AMD */
  manuf_id = flash_get_manuf_code(0);

  switch(manuf_id)
  {
	case AMDPART: printf("AMD Flash found in socket...\n");
   		      break;
	default: printf("Non AMD part found in socket...exiting.\n");
		 exit(1);
		 break;
  }

  /* Poll the device id so that the proper sector layout table
     is used for the device in the socket */

  printf("Polling part for Device ID...");

  /* Retrieve the device ID for this AMD flash part.  All device
     id's are stored in flash.h */

  device_id = flash_get_device_id(0);

  switch(device_id)
  {
	case ID_AM29DL800T: 	printf("found an Am29DL800T\n");
				if(!init_flash(AM29DL800T)) exit(1);
				break;

	case ID_AM29DL800B:  	printf("found an Am29DL800B\n");
				if(!init_flash(AM29DL800B)) exit(1);
				break;

	case ID_AM29LV800T:  	printf("found an Am29LV800T\n");
				if(!init_flash(AM29LV800T)) exit(1);
				break;

	case ID_AM29LV800B:  	printf("found an Am29LV800B\n");
				if(!init_flash(AM29LV800B)) exit(1);
				break;

	case ID_AM29LV160B:  	printf(  "found an Am29LV160B\n");
				if(!init_flash(AM29LV160B)) exit(1);
				break;

	case ID_AM29LV400B:  	printf(  "found an Am29LV400B\n");
				if(!init_flash(AM29LV400B)) exit(1);
				break;

	default: printf("error reading Device ID...exiting.\n");
		 exit(1);
		 break;
  }

  randomize();

  /* flash_get_status uses DQ7, DQ5, and DQ2 polling to get the
     status of the flash.  All status codes are defined in flash.h
     Also note that for the DL parts, status is bank dependent */

  printf("Checking current flash status...flash is ");
  test = flash_get_status(0);
  switch(test)
  {
	 case STATUS_READY: 	printf("[Ready]\n");break;
	 case STATUS_BUSY: 	printf("[Busy]\n");break;
	 case STATUS_ERSUSP: 	printf("[Erase Suspended]\n");break;
	 case STATUS_TIMEOUT: 	printf("[Timed Out]\n");break;
	 default: 		printf("Error!\n"); exit(1);break;
  }

  printf("Performing API tests...\n\n");

  /* flash_sector_erase_int() is the function which erases a single
     sector.  It is different from flash_sector_erase() in that it
     'interrupts' execution of the program until the erase is completed.
     For erasing a sector without pausing use flash_sector_erase(). */

  flash_reset(); /* Quick safe check */

  printf("Erasing sector 8...");
  flash_sector_erase_int(8);
  printf("done.\n");

  printf("Verifying erase...");

  flash_get_sector_size(8, &sec_size); /* Get # of byte */

  /* A simple test which reads every word from the flash, and checks
     to see if every word contains the data 0xFFFF, which indicates
     an erased word. */

  for (count=0 ; count < (sec_size/2); count++) {
	 if(count%2048 == 0)
		printf("."); /*print out some dots to show the program hasn't frozen */
	 if (flash_read_word(8,count) != 0xFFFF) {
		printf("erase not completed sucessfully!\n");
		exit(1);
	 }
  }
  printf("erase successful.\n");

  /* flash_write_word() takes word data and programs it to the flash
     at the indicated offset.  Note that this data must be *word aligned*,
     or else programming errors can result.
     It is also good to check the word for 0xFFFF data before programming. */
  
  printf("Writing a single word [0xABCD]\n");
  flash_write_word(8,0,0xABCD);

  /* flash_read_word() returns a single word of data at the specified
     sector/offset .  Must also be word aligned */
  
  printf("After write(0xABCD): %4x\n", flash_read_word(8,0));

  str = (byte far *) calloc(0x7FFF, sizeof(byte));

  /* Randomize the string with random ASCII characters */
  for(i=0; i<0x7FFF; i++) {
	 str[i] = (byte) (41 + (rand() % 26));
  }

  printf("Erasing sector 9...");
  flash_sector_erase_int(9);
  printf("done.\n");

  /* flash_write_string() is a function to program bulk data from a C
     buffer.  It is a bit faster than looping techniques using
     flash_write_word() because function overhead is eliminated. */
  
  printf("Writing 32 kbyte string...");
  flash_write_string(9,0,str,0x7FFE);

  printf("done.\n\n");

  printf("Testing erase suspend\n");
  printf("Beginning erase...\n");

/* This is an example of flash_sector_erase().  Note that the program will
     simply issue the command, and execution will continue while the
     flash is erasing. */
  
  flash_sector_erase(10);

  /* flash_erase_suspend will suspend an erase in progress.  The application
     can then do any reading of data from that sector, or another sector. */
  
  printf("Suspending erase...");
  flash_erase_suspend(10);
  printf("done.\n");

  /* The current flash status should now be STATUS_ERSUSP */
  printf("Checking current flash status...flash is ");
  test = flash_get_status(10);
  switch(test)
  {
	 case STATUS_READY: printf("[Ready]\n");break;
	 case STATUS_BUSY: printf("[Busy]\n");break;
	 case STATUS_ERSUSP: printf("[Erase Suspended]\n");break;
	 case STATUS_TIMEOUT: printf("[Timed Out]\n");break;
	 default: printf("Error!\n"); exit(1);break;
  }

  /* Now we can resume the erase previously suspended */
  printf("Resuming erase after status check..");
  flash_erase_resume(10);
  printf("done.\n");

  /* Now for a test of unlock bypass mode */
  /* Unlock bypass allows for faster programming of flash data in that
     the number of required bus cycles is cut in half.  The most benefit
     can be realized when programming large amounts of data using
     flash_write_string_ub() */

  printf("Entering unlock bypass mode...\n");
  flash_sector_erase_int(11);
  flash_ub(11); /* Enter unlock bypass mode */

  printf("Programming a string in unlock bypass mode..");
  flash_write_string_ub(11,0,str,0x7FFE);
  printf("done.\n");

  printf("Exiting unlock bypass mode..\n");
  flash_reset_ub();

  flash_reset();

  /* Last thing is a quick loop through all the sectors 
     to check for sector protection. */

  printf("\nVerifying sector protection...\n");

  flash_get_numsectors(&test2);

  printf("This device contains %3i sectors: \n", test2);
  for(i=0; i < test2; i++) {
	 test = flash_sector_protect_verify(i);
	 flash_get_sector_size(i, &size);
	 printf("Verify sector #%2i, size [%-5li]: ",
	          i, size);
	 if (test == 0x01)
		printf("sector is protected[%2i].\n", test);
	 else
		printf("sector is not protected[%2i].\n", test);
	 flash_reset();
  }


  printf("Test drive done!\n");
  free(str);
  exit(0);
}
