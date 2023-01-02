/************************************************************************/
/*                                                                      */
/*  AMD Flash Memory Drivers                                            */
/*  File name: PROTECT.C                                                */
/*  Revision:  1.0  6/10/99                                             */
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
/*  protecting AMD word Flash components. AMD                           */
/*  will not be responsible for misuse or illegal use of this           */
/*  software for devices not supported herein. AMD is providing         */
/*  this source code "AS IS" and will not be responsible for            */
/*  issues arising from incorrect user implementation of the            */
/*  source code herein. It is the user's responsibility to              */
/*  properly design-in this source code.                                */
/************************************************************************/ 

/************************************************************************/ 
/* Note: for both functions, fp is declared as a global word pointer to */
/* the beginning (word offset 0) of the flash device in the memory map. */
/*                                                                      */
/* DWORD = typedef unsigned long                                        */
/* WORD = typedef unsigned short                                        */
/* BYTE = typedef unsigned char                                         */
/*                                                                      */
/************************************************************************/ 

// Set MAXSECTORS according to the device in question
// e.g. 35 for an Am29LV160B, but only boot sectors listed for clarity

#define 	MAXSECTORS	4

DWORD sectorinfo[MAXSECTORS] = { 0x0000, 0x4000, 0x6000, 0x8000 };

long ProtectSector(long sector)
{
	// This algorithm does NOT work with x8 (byte mode) devices
 	// at this time
	
	unsigned int plscnt = 1;		// Start PLSCNT at 1
	WORD verify;				// Read-back data

	// Always do the safe reset
	fp[0x1234] = 0xF0;
	
	// First, put the reset pin to VID (put +9V onto the pin)
	ToggleVID();

	// Delay for a while (1 us minimum)
	DelayMilli(10);

	// First write cycle contains 0x60h
	fp[0x5555] = 0x60;

	// As per in-system sector protect algorithm, only do this
        // loop a maximum of 25 times

	while( plscnt < 26 ) {
	
		// Write 0x60h to sector address, with A1=1 (or add 2)
		// Remember to convert the base address of the sector
                // Into a word address

		fp[((sectorinfo[sector]/2)+2)] = 0x60;

		//Delay for 150us, but we can delay longer
		DelayMilli(10);

		// Now send the verify pulse, with A6=0, A1=1, A0 =0
                // (or add two)
		fp[((sectorinfo[sector]/2)+2)] = 0x40;

		// Now read from sector offset, with A6=0, A1=1, A0 =0
                // (or add two)	
		verify = *(fp + (sectorinfo[sector]/2) + 2);

		// Is the data back 0x01h?  If not, give it another 
                // write pulse
		if ( (verify & 0x01) != 0x01)
			plscnt++;
		else
			break;
	}

	// If we hit plscnt equals 26, then we gave 25 pulses and it
        // didn't protect...flag error contition
	if (plscnt == 26) {
		ToggleVID();		// Remove high voltage 
		DelayMilli(10);		// Delay for 10 ms for settling
		return( (DWORD) 0 );  	// Failed to protect!
	}
	else {
		ToggleVID();		// Remove high voltage 
		fp[0x1234] = 0xF0;	// Do a safe reset
		DelayMilli(10);		// Delay for 10 ms for settling
		return( (DWORD) 1 );	// Return success
	}
}

long UnprotectSectors(void)
{
	// This algorithm does NOT work with x8 (byte mode) devices
 	// at this time

	unsigned int plscnt = 1, i=0, atleastone=0;
	WORD verify;

	// Always do the safe reset
	fp[0x1234] = 0xF0;

	// Second, put the reset pin to VID
	ToggleVID();

	// Delay for a while
	DelayMilli(10);

	// NOTE: ALL SECTORS MUST BE PROTECTED BEFORE THIS FUNCTION
	// IS CALLED!!  It might be a good idea to add a loop to
        // perform autoselect calls to make sure all sectors read
        // protected, and make calls to ProtectSector on the ones
        // which aren't protected

	// First write cycle contains 0x60h
	fp[0x5555] = 0x60;

	// As per in-system sector protect algorithm, only do this
        // loop a maximum of 1000 times

	// This algorithm differs from the protect algorithm in that
   	// sectors are protected one by one, but unprotected in parallel

	while( plscnt < 1001 ) {
	
		// Write 0x60h to sector address, with A6=1,A1=1 (or add 66)
		// The actual sector doesn't matter, so use sector 1
		fp[((sectorinfo[1]/2)+66)] = 0x60;

		//Delay for 15ms, but we can delay longer
		DelayMilli(20);

		// Now, we need to read offset 66 from all sectors
		// for zero data (A6=1, A1=1, A0=0 = 0x66h)

		for(i=0; i < MAXSECTORS ; i++) {
		
			// Send a verify pulse
			fp[((sectorinfo[i]/2)+66)] = 0x40;

			verify = *(fp + (sectorinfo[i]/2) + 66);

			// If the data isn't zero, then 'at least one'
			// sector didn't verify...hence give another pulse
			if (verify != 0x00) {
				atleastone = 1;
				break;
			}
		}

		// If one didn't verify, reset flag and inc plscnt
		if( atleastone == 1 ) {
			atleastone = 0;
			plscnt++;
		}
		else
			break;
	}

	if (plscnt == 1001) {
		ToggleVID();		// Remove high voltage 
		DelayMilli(10);		// Delay for 10 ms for settling
		return( (DWORD) 0 );  	// Failed to unprotect!
	}
	else {
		ToggleVID();		// Remove high voltage 
		DelayMilli(10);		// Delay for 10 ms for settling
		fp[0x1234] = 0xF0;	// Do a safe reset
		return( (DWORD) 1 );	// Return success
	}
}