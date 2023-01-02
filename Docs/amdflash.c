/*******************************************************************/
/*                                                                 */
/*  AMD Embedded Flash Development Kit (AMD FDK)                   */
/*  File name: AMDFLASH.C                                          */
/*  Revision:  1.12 10/17/95                                       */
/*  Copyright (c) 1994, 1995 ADVANCED MICRO DEVICES, INC. All      */
/*  rights reserved.                                               */
/*  This software is owned by Advanced Micro Devices, Inc.,        */
/*  1 AMD Place, Sunnyvale, CA 94088.                              */
/*                                                                 */
/*  This software is freely given "AS IS" to be used and           */
/*  replicated in part or whole by the user, unless otherwise      */
/*  provided in the Software Agreement associated herewith.        */
/*  The restriction is that this copyright notice must be included */
/*  with this software, whether used in part or whole, at all      */
/*  times. Not acknowledging this copyright notice will be a       */
/*  violation of Federal statutes of the United States of          */
/*  America (specifically, those governing restrictions in         */
/*  paragraphs of the Rights in Technical Data and Computer        */
/*  Software Clauses) and can be punishable by fines.              */
/*                                                                 */
/*  This software constitutes a basic shell of source code for     */
/*  programming all AMD Flash components. AMD                      */
/*  will not be responsible for misuse or illegal use of this      */
/*  software for devices not supported herein. AMD is providing    */
/*  this source code "AS IS" and will not be responsible for       */
/*  issues arising from incorrect user implementation of the       */
/*  source code herein. It is the user's responsibility to         */
/*  properly design-in this source code.                           */
/*                                                                 */ 
/*******************************************************************/


/*******************************************************************/
/*                                                                 */
/*  User notes:                                                    */
/*                                                                 */
/*  1. Please refer to the appropriate documentation for this      */
/*     source code for an explanation of the routines. The         */
/*     documentation is available electronically or with the full  */
/*     AMD FDK, available from distributors and AMD.               */
/*                                                                 */
/*  2. Please read the comments wherever applicable. Due to the    */
/*     hardware semi-independence of this source code, there are   */
/*     comments alluding to the necessity of voltage control       */
/*     codes in some routines.                                     */
/*     Additionally, this code was derived from the working demo   */
/*     programs in the AMD FDK, which is PC AT I/O based.          */
/*     Statements such as:                                         */
/*                  _inp, _inpw, _outp, _outpw                     */
/*     are strictly related to the PC AT I/O architecture, which   */
/*     facilitates input and output to that bus structure. These   */
/*     commands are also specific to Microsoft Visual C++ 1.0 (TM) */
/*     and are only included here for legibility.                  */
/*                                                                 */
/*  3. This software constitutes a basic shell of source code for  */
/*     programming all AMD Flash components.                       */
/*     AMD will not be responsible for misuse or illegal use of    */
/*     this software for devices not supported herein. AMD is      */
/*     providing this source code "AS IS" and will not be respon-  */
/*     sible for issues arising from incorrect user implementation */
/*     of the source code herein. It is the user's responsibility  */
/*     to properly design-in this source code.                     */
/*                                                                 */
/*******************************************************************/

/*******************************************************************/
/*                                                                 */
/*  Errata:                                                        */
/*  6/28/94: First Revision                                        */
/*  9/14/94: Revision 1.1. Adds enhancements, and fixes.           */
/*           Initialization area: removed Macros. Not used         */
/*           Write_bin(): adds logic for word (x16) device support */
/*           Emb_erase(): adds support for 29F016                  */
/*                        fixes bug for not detecting 28FxxxA      */
/*                          devices when checking for sectors      */
/*           Std_erase(): adds a minor printf clarification on     */
/*                          erase_verify                           */
/*           Std_program(): removed check of byte to program. It   */
/*                            already done in Program().           */
/*           Std_parallel_prog(): removed check as in std_program()*/
/*           Program(): adds support for word (x16) devices        */
/*                      adds more 29F devices checking when        */
/*                        checking for protected sectors           */
/*                      fixes bug in highest sector number checked */
/*                        when checking for protected sectors. The */
/*                        correct number is determined from this_  */
/*                        dev_GSP->num_sectors                     */
/*                      adds case structure to see if the next byte*/
/*                        to program is the same as what's already */
/*                        there                                    */
/*           Sector_erase(): Fixed bug for missing flag that was   */
/*                             reset at the right time (bDone)     */
/*                           Erase_suspend() routine was invoked at*/
/*                             wrong place. Removed routines from  */
/*                             middle of unlock cycles and placed  */
/*                             the routine after all 6 unlock      */
/*                             cycles were issued                  */
/*           Erase_suspend(): Removed references to 29F010 because */
/*                              29F010 does not support E/S        */
/*                            Fixed erroneous address limit checks */
/*                              for the read option                */
/*                            Added some printf logic so as not to */
/*                              print nonprintable characters      */
/*                              (such as Backspace,etc)            */
/*           Sector_prot(): This function was removed as           */
/*                            it is requested that a proper prog-  */
/*                            rammer from a qualified 3rd party    */
/*                            vendor (such as Data I/O (TM)) be    */
/*                            used to perform this function, to    */
/*                            improve the robustness of and not    */
/*                            damage the device and hence, the     */
/*                            customer's design, due to the high   */
/*                            voltages that need to be present on  */
/*                            various pins of the device           */
/*           Sector_unprot(): See Sector_prot() explanation above. */
/*           Dq6_poll(): Function was upgraded to show clearly the */
/*                         function flow, as well as provide the   */
/*                         support structure for the devices       */
/*           Dq7_poll(): See Dq6_poll() explanation above.         */
/*           Split_bytes(): New function that splits a word into 2 */
/*                            bytes                                */
/*           Main(): Added correct limit when initializing erase   */
/*                     and protect status arrays. This should be   */
/*                     32.                                         */
/*                   Added support for Word (x16) devices: word    */
/*                     program                                     */
/*                   Changed sector menu to invoke sector erase    */
/*                     (with the option to suspend) only; removed  */
/*                     options to protect and unprotect the device */
/*  2/08/95: Revision 1.11                                         */
/*           Added support for 29F080                              */
/*           Removed extra error messages in Error_Handler(), as   */
/*           well as extra #define statements                      */ 
/*           29F200B byte errata (ID change to 57h) see bulletin   */
/*           PID # 19906A                                          */
/*                                                                 */
/* 10/18/95: Revision 1.12                                         */
/*           Added nomenclature, minor text changes for            */
/*           29F400A T/B support (any 29F400A command will act as  */
/*           a 29F400 - please consult AMD sales offices for       */
/*           specific questions/differences)                       */
/*           Sector_erase: Added important verbage to mention that */
/*                           that while in timeout mode, any com-  */
/*                           mand other than sector erase or erase */
/*                           suspend resume will return the device */
/*                           back to read mode                     */
/*           Dq7_poll(): Errata - When checking the DQ7 bit for    */
/*                       Data Poll, bytewise operator was used     */
/*                       instead of bitwise (3 places corrected)   */
/*******************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <conio.h>
#include <ctype.h>
#include <time.h>

/*******************************************************************/
/*	Defined variables for emb_erase, emb_prog()                    */
/*                                                                 */
/*******************************************************************/
#define FL_28F 		1		/* Select 28F devices */
#define FL_29F 		2		/* Select 29F devices */

#define FL_PROG 	0		/* Select Programming op. */
#define FL_CH_ERASE 1		/* Select Chip Erase op. */
#define FL_SC_ERASE 2		/* Select Sector Erase op. */

#define TRUE		1
#define FALSE		0

#define BIN 		1
#define HEX 		2
#define SINGLE 		3		//single byte program  
#define WORD 		4		/* word program          */


/* ****************************************************************** *
 * 							STRUCTURES                                *
 * ****************************************************************** */
/******  xxxx_GS = variable is a or part of a global structure *****/
/******  xxxx_GA = variable is a global array                  *****/
typedef struct dev {
	int device_no_GS;
	char *devname_GS;
	int num_sectors;
	unsigned long kb_size_GS;
	unsigned char device_id_GS;
	unsigned char manuf_id_GS;
} devices_GS;

#define INVALID    0
#define Am28F010A  1
#define Am28F020A  2
#define Am29F010   3
#define Am29F040   4
#define Am28F256A  5
#define Am28F512A  6
#define Am28F010   7
#define Am28F020   8
#define Am29F100T  9
#define Am29F100B 10
#define Am29F200T 11
#define Am29F200B 12
#define Am29F400T 13	/* or 29F400AT */
#define Am29F400B 14	/* or 29F400AB */
#define Am29F016  15
#define Am28F256  16
#define Am28F512  17
#define Am29F080  18

#define MAX_NUM_SECTORS 11

devices_GS amd_devices_GA[] =
{
	INVALID,   "INVALID",  0,  000000L, 0x00, 0x00,
	Am28F010A, "28F010A",  0,  131072L, 0xA2, 0x01,
	Am28F020A, "28F020A",  0,  262144L, 0x29, 0x01,
	Am29F010,  "29F010",   8,  131072L, 0x20, 0x01,
	Am29F040,  "29F040",   8,  524288L, 0xA4, 0x01,
	Am28F256A, "28F256A",  0,   32768L, 0x2F, 0x01,
	Am28F512A, "28F512A",  0,   65536L, 0xAE, 0x01,
	Am28F010,  "28F010",   0,  131072L, 0xA7, 0x01,
	Am28F020,  "28F020",   0,  262144L, 0x2A, 0x01,
	Am29F100T, "29F100T",  5,   65536L, 0x00, 0x01,
    Am29F100B, "29F100B",  5,   65536L, 0x00, 0x01,
    Am29F200T, "29F200T",  7,  131072L, 0x51, 0x01,
    Am29F200B, "29F200B",  7,  131072L, 0x57, 0x01,
    Am29F400T, "29F400T", 11,  262144L, 0x23, 0x01,
    Am29F400B, "29F400B", 11,  262144L, 0xAB, 0x01,
    Am29F016,  "29F016",  32, 2097152L, 0xAD, 0x01,
	Am28F256,  "28F256",   0,   32768L, 0xA1, 0x01,
	Am28F512,  "28F512",   0,   65536L, 0x25, 0x01,
    Am29F080,  "29F080",  16, 1048576L, 0xD5, 0x01	
};
/* Note: 29F400A is treated as 29F400 for all intents and purposes    */

/* ****************************************************************** *
 * 							FORWARD DECLARATIONS                      *
 * ****************************************************************** */
int  sector_verify(void);
int  sector_erase(int);
void split_addr(long);
unsigned int concat_bytes(int,int);
void split_bytes( int, char *, char *);
void autoselect_seq(int);
void reset_fl(int);
void manuf_code(int);
void device_code(int);
int  dq7_poll(int);
int  dq6_poll(int);
int  sel_device(void);
int  emb_erase(void);
int  emb_program(unsigned long,unsigned int);
int  std_program(unsigned long,unsigned int);
int  std_parallel_prog(unsigned long,unsigned int);
int  std_erase(void);
int  std_parallel_erase( void );
int  program(int);
int  erase_suspend(int);
void error_handler(int);
int  write_bin(void);
int  no_dq_poll(int);

/* ****************************************************************** *
 * 							GLOBALS                                   *
 * ****************************************************************** */
/*****  xxxxx_G   = global variable *****/
/*****  xxxxx_GP  = global pointer  *****/
/*****  xxxxx_L   = local  variable *****/
/*****  xxxxx_LP  = local  pointer  *****/
/*****  xxxxx_GSP = Global Structure pointer *****/
unsigned long address_G      = 0L;		/* address is larger than 16 bits	*/
devices_GS   *this_dev_GSP   = NULL;	/* pointer to the current device 	*/
int 		  curr_control_G = 0;       /* current control code byte 		*/
unsigned int  curr_data_G    = 0;		/* current data to be programmed 	*/
int 		  curr_socket_G  = 0;		/* socket currently selected: 0=none
										   1=s1
										   2=s2 							*/
int 		  m_code_G       = 0;       /* temp storage for manuf code 		*/
int           d_code_G       = 0;		/* temp storage for device code 	*/
int           prot_sects_GA[33];		/* array to store protected sector 
							    		   numbers, starting at 0; values: 
							    		   0 = not protected
										   1 = protected
										   2 = initialized 					*/
int 		  erase_sects_GA[33];		/* array to store erased sector 
										   numbers, starting at 0; values:
										   0 = erased
										   1 = not erased
										   2 = initialized				*/								
unsigned int  base_decode_G  = 0;	/* base decode address for board 	*/
unsigned long poll_addr_G    = 0;	/* address to start polling at (for 29F
									   devices with protected sectors)  */
int 		  errdata_G      = 0;   /* on error, the current byte data is placed here */
int           use_dq7_G      = 1;   /* Toggles use of DQ7/DQ5 or DQ6/DQ5 (use_dq7_G=0) */
int           no_poll_G      = 0;	/* Toggles between use of DQ7/DQ6(no_poll_G = 0) or read byte */
int 		  flerror_code_G = 0;	/* Error Code from subroutines		*/
int 		  x_sector_flag_G= 0;	/* Set if the Sector Unprotect routine called it */
int 		  x_sector_G     = 0;	/* contains the sector number to protect if Sector Unprotect() called it */
char string_GA[32], string2_GA[4];  /* Strings used for int to ASCII conversion	*/

unsigned long last_file_addr_G = 0L; /* the last address of the file opened for programming */
unsigned long this_address_G = 0L;   /* on error, the current address is placed here */
unsigned int  num_bytes_G    = 0;	 /* In pre-processing the input file, the number of bytes
										per line is placed here 		*/
int 		  no_device_G    = 1;	 /* If no device was found, no_device_G=1 */
int 		  filter_read_G  = 0;	 /* filter upper 128 characters when Reading (off = 0)*/
int           status;

/* Hardwired sector addresses for 29F010, 29F040 devices		*/
unsigned long int addr_29F010_GA[] =
	{						/* A16 A15 A14   Addr Range 		*/
	0x00000000L,			/*  0	0	0	00000-03FFFh		*/
	0x00004000L,			/*	0	0	1	04000-07FFFh		*/
	0x00008000L,			/*  0	1	0	08000-0BFFFh		*/
	0x0000C000L,			/*	0	1	1	0C000-0FFFFh		*/
	0x00010000L,			/*	1	0	0	10000-13FFFh		*/
	0x00014000L,			/*	1	0	1	14000-17FFFh		*/
	0x00018000L,			/*	1	1	0	18000-1BFFFh		*/
	0x0001C000L,			/*	1	1	1	1C000-1FFFFh		*/
	0x00020000L				/*	Final address					*/
	};

unsigned long int addr_29F040_GA[] =
	{						/* A18 A17 A16   Addr Range 		*/
	0x00000000L,			/*  0	0	0	00000-0FFFFh		*/
	0x00010000L,			/*	0	0	1	10000-1FFFFh		*/
	0x00020000L,			/*  0	1	0	20000-2FFFFh		*/
	0x00030000L,			/*	0	1	1	30000-3FFFFh		*/
	0x00040000L,			/*	1	0	0	40000-4FFFFh		*/
	0x00050000L,			/*	1	0	1	50000-5FFFFh		*/
	0x00060000L,			/*	1	1	0	60000-6FFFFh		*/
	0x00070000L,			/*	1	1	1	70000-7FFFFh		*/
	0x00080000L				/*	Final address					*/
	};

unsigned long int addr_29F100T_GA[] =
	{						/*   Addr Range 	*/
	0x00000000L,			/* 00000-0FFFFh		*/
	0x00010000L,			/* 10000-17FFFh		*/
	0x00018000L,			/* 18000-19FFFh		*/
	0x0001A000L,			/* 1A000-1BFFFh		*/
	0x0001C000L,			/* 1C000-1FFFFh		*/
	0x00020000L				/*	Final address	*/
	};

unsigned long int addr_29F100B_GA[] =
	{						/*   Addr Range 	*/
	0x00000000L,			/* 00000-03FFFh		*/
	0x00004000L,			/* 04000-05FFFh		*/
	0x00006000L,			/* 06000-07FFFh		*/
	0x00008000L,			/* 08000-0FFFFh		*/
	0x00010000L,			/* 10000-1FFFFh		*/
	0x00020000L				/*	Final address	*/
	};

unsigned long int addr_29F200T_GA[] =
	{						/*   Addr Range 	*/
	0x00000000L,			/* 00000-0FFFFh		*/
	0x00010000L,			/* 10000-1FFFFh		*/
	0x00020000L,			/* 20000-2FFFFh		*/
	0x00030000L,			/* 30000-37FFFh		*/
	0x00038000L,			/* 38000-39FFFh		*/
	0x0003A000L,			/* 3A000-3BFFFh		*/
	0x0003C000L,			/* 3C000-3FFFFh		*/
	0x00040000L				/*	Final address	*/
	};

unsigned long int addr_29F200B_GA[] =
	{						/*   Addr Range 	*/
	0x00000000L,			/* 00000-03FFFh		*/
	0x00004000L,			/* 04000-05FFFh		*/
	0x00006000L,			/* 06000-07FFFh		*/
	0x00008000L,			/* 08000-0FFFFh		*/
	0x00010000L,			/* 10000-1FFFFh		*/
	0x00020000L,			/* 20000-2FFFFh		*/
	0x00030000L,			/* 30000-3FFFFh		*/
	0x00040000L				/*	Final address	*/
	};

unsigned long int addr_29F400T_GA[] =
	{						/*   Addr Range 	*/
	0x00000000L,			/* 00000-0FFFFh		*/
	0x00010000L,			/* 10000-1FFFFh		*/
	0x00020000L,			/* 20000-2FFFFh		*/
	0x00030000L,			/* 30000-3FFFFh		*/
	0x00040000L,			/* 40000-4FFFFh		*/
	0x00050000L,			/* 50000-5FFFFh		*/
	0x00060000L,			/* 60000-6FFFFh		*/
	0x00070000L,			/* 70000-77FFFh		*/
	0x00078000L,			/* 78000-79FFFh		*/
	0x0007A000L,			/* 7A000-7BFFFh		*/
	0x0007C000L,			/* 7C000-7FFFFh		*/
	0x00080000L				/*	Final address	*/
	};

unsigned long int addr_29F400B_GA[] =
	{						/*   Addr Range 	*/
	0x00000000L,			/* 00000-03FFFh		*/
	0x00004000L,			/* 04000-05FFFh		*/
	0x00006000L,			/* 06000-07FFFh		*/
	0x00008000L,			/* 08000-0FFFFh		*/
	0x00010000L,			/* 10000-1FFFFh		*/
	0x00020000L,			/* 20000-2FFFFh		*/
	0x00030000L,			/* 30000-3FFFFh		*/
	0x00040000L,			/* 40000-4FFFFh		*/
	0x00050000L,			/* 50000-5FFFFh		*/
	0x00060000L,			/* 60000-6FFFFh		*/
	0x00070000L,			/* 70000-7FFFFh		*/
	0x00080000L				/*	Final address	*/
	};

unsigned long int addr_29F016_GA[] =
	{						/*    Addr Range 		*/
	0x00000000L,			/*  000000-00FFFFh		*/
	0x00010000L,			/*	010000-01FFFFh		*/
	0x00020000L,			/*  020000-02FFFFh		*/
	0x00030000L,			/*	030000-03FFFFh		*/
	0x00040000L,			/*	040000-04FFFFh		*/
	0x00050000L,			/*	050000-05FFFFh		*/
	0x00060000L,			/*	060000-06FFFFh		*/
	0x00070000L,			/*	070000-07FFFFh		*/
	0x00080000L,			/*  080000-08FFFFh		*/
	0x00090000L,			/*	090000-09FFFFh		*/
	0x000A0000L,			/*  0A0000-0AFFFFh		*/
	0x000B0000L,			/*	0B0000-0BFFFFh		*/
	0x000C0000L,			/*	0C0000-0CFFFFh		*/
	0x000D0000L,			/*	0D0000-0DFFFFh		*/
	0x000E0000L,			/*	0E0000-0EFFFFh		*/
	0x000F0000L,			/*	0F0000-0FFFFFh		*/
	0x00100000L,			/*  100000-10FFFFh		*/
	0x00110000L,			/*	110000-11FFFFh		*/
	0x00120000L,			/*  120000-12FFFFh		*/
	0x00130000L,			/*	130000-13FFFFh		*/
	0x00140000L,			/*	140000-14FFFFh		*/
	0x00150000L,			/*	150000-15FFFFh		*/
	0x00160000L,			/*	160000-16FFFFh		*/
	0x00170000L,			/*	170000-17FFFFh		*/
	0x00180000L,			/*  180000-18FFFFh		*/
	0x00190000L,			/*	190000-19FFFFh		*/
	0x001A0000L,			/*  1A0000-1AFFFFh		*/
	0x001B0000L,			/*	1B0000-1BFFFFh		*/
	0x001C0000L,			/*	1C0000-1CFFFFh		*/
	0x001D0000L,			/*	1D0000-1DFFFFh		*/
	0x001E0000L,			/*	1E0000-1EFFFFh		*/
	0x001F0000L,			/*	1F0000-1FFFFFh		*/
	0x00200000L				/*	Final address		*/
	};

unsigned long int addr_29F080_GA[] =
	{						/*    Addr Range 		*/
	0x00000000L,			/*  000000-00FFFFh		*/
	0x00010000L,			/*	010000-01FFFFh		*/
	0x00020000L,			/*  020000-02FFFFh		*/
	0x00030000L,			/*	030000-03FFFFh		*/
	0x00040000L,			/*	040000-04FFFFh		*/
	0x00050000L,			/*	050000-05FFFFh		*/
	0x00060000L,			/*	060000-06FFFFh		*/
	0x00070000L,			/*	070000-07FFFFh		*/
	0x00080000L,			/*  080000-08FFFFh		*/
	0x00090000L,			/*	090000-09FFFFh		*/
	0x000A0000L,			/*  0A0000-0AFFFFh		*/
	0x000B0000L,			/*	0B0000-0BFFFFh		*/
	0x000C0000L,			/*	0C0000-0CFFFFh		*/
	0x000D0000L,			/*	0D0000-0DFFFFh		*/
	0x000E0000L,			/*	0E0000-0EFFFFh		*/
	0x000F0000L,			/*	0F0000-0FFFFFh		*/
	0x00100000L,            /*  Final address       */
	};


/********************************************************************/
/* Defines for Error_handler()                                      */
/********************************************************************/
#define NO_FILE_NAME_ENTERED	10
#define INPUT_FILENAME_MISSING	11
#define FILENAME_PROBLEM		12
#define FSEEK_FAILED			13
#define INTEL_FILE_PROBLEM		14
#define EOF_REACHED_EARLY		15
#define NOT_INTEL_HEX			16
#define BYTE_COUNT_ERROR_1		17
#define CHECKSUM_ERROR_1		18
#define BAD_RECORD_CODE_1		19
#define DATACOUNT_ERROR_1		20
#define UNEXPECTED_CHAR_FOUND	21
#define ERROR_RECORD_FIELD		22
#define BAD_SEQUENCE_1			23

#define BYTE_COUNT_ERROR_3		26
#define ADDRESS_ERROR_1			27
#define BAD_SEQUENCE_2			28

#define ALL_SECTORS_PROTECTED	30
#define SOME_SECTORS_PROTECTED	31
#define NOT_FOR_28F_DEVICES		32
#define THIS_SECTOR_PROTECTED	33
#define CANNOT_RE_ERASE_SECTOR	34
#define PLSCNT_GT_25			35
#define CANNOT_REPROTECT_SECTOR	36
#define CANT_REUNPROTECT_SECTOR	37
#define PLSCNT_GT_1000			38
#define PROGRAMMING_FAILURE		39
#define DQ6_PROGRAM_FAILURE		40
#define DQ6_KEEPS_TOGGLING		41
#define DQ6_ERASE_FAILURE_DATA	42
#define DQ6_ERASE_FAILURE_DQ5	43
#define DQ6_ERASE_ON_28F		44
#define DQ6_ERASE_ON_29F		45
#define DQ7_PROGRAM_FAILURE		46
#define DQ7_FAIL_AFTER_DQ5_1	47
#define DQ7_FAIL_AFTER_DQ5_2	48
#define DQ7_ERASE_FAILURE		49
#define DQ7_ERASE_FAILURE_DQ5	50
#define DQ7_ERASE_ON_28F		51
#define DQ7_ERASE_ON_29F		52
#define ADDRESS_EXCEEDS_LIMITS	53
#define UNKNOWN_BREAK_READHEX	54
/* removed: #define DEVICE_IS_A_28F256		55*/
/* removed: #define DEVICE_IS_A_28F512		56*/
/* removed: #define DEVICE_IS_A_28F010		57*/
/* removed: #define DEVICE_IS_A_28F020		58*/
#define DEVICE_IS_NOT_AMD_1		59
#define DEVICE_IS_NOT_AMD_2		60
#define SELECT_DEVICE_FIRST		61
#define DEVICE_SIZE_EXCEEDED	62
#define DQ5_ON_NO_POLL			63
#define CANT_USE_STD_PROG_ALG	64
#define STD_PROG_ALG_FAIL		65
#define CANT_USE_STD_ERASE_ALG	66
#define STD_ERASE_ALG_FAIL		67
#define STD_LL_PROG_FAIL		68
#define STD_LL_ERASE_FAIL		69
#define NOT_FOR_THIS_DEVICE 	70
/* removed:#define DEVICE_IS_A_28F256A		71*/
/* removed:#define DEVICE_IS_A_28F512A		72*/
/* removed:#define DEVICE_IS_A_29F100T		73*/
/* removed:#define DEVICE_IS_A_29F100B		74*/
/* removed:#define DEVICE_IS_A_29F200T		75*/
/* removed:#define DEVICE_IS_A_29F200B		76*/
/* removed:#define DEVICE_IS_A_29F400T		77*/
/* removed:#define DEVICE_IS_A_29F400B		78*/
#define DEVICE_IS_A_29F016		79
#define ERASE_SUSPEND_NOT_SUPP	80
#define DEVICE_IS_A_29F080      81

/*******************************************************************/
/*	error_handler()                                                */
/*                                                                 */
/*	purpose:	Processes all error codes by printing out messages */
/*              to the user.                                       */
/*                                                                 */
/*******************************************************************/
/*	parameters:	int err_val = error code number (0-9 not used)     */
/*******************************************************************/
/*  return value:   none                                           */
/*******************************************************************/
void error_handler( int err_val_L )
{
	switch(err_val_L) {
	
		case NO_FILE_NAME_ENTERED:
			printf("\n(%d)No file name entered!\n", err_val_L);
			printf("Usage: ihex <file, without extension>\n");
			break;
		case INPUT_FILENAME_MISSING :
			printf("\n(%d)Input filename missing!", err_val_L);
			break;
		case FILENAME_PROBLEM :
			printf("\n(%d)File name problem: invalid filename, missing file", err_val_L);
			break;
		case FSEEK_FAILED :
			printf("\n(%d)Fseek failed", err_val_L);
			break;
		case INTEL_FILE_PROBLEM :
			printf("\n(%d)File name problem: invalid filename, missing file", err_val_L);
			break;
		case EOF_REACHED_EARLY:
			printf("\n(%d)Error: EOF reached before checking file!", err_val_L);
			break;
		case NOT_INTEL_HEX :
//			printf("\n(%d)Error: %s is not an Ihex formatted file!", err_val_L, FileName_G);
			break;
		case BYTE_COUNT_ERROR_1 :
			printf("\n(%d)Error: byte count not 0 on 01 record", err_val_L);
			break;
		case CHECKSUM_ERROR_1 :
			printf("\n(%d)Error: checksum not FF on 01 record", err_val_L);
			break;
		case BAD_RECORD_CODE_1 :
			printf("\n(%d)Not recognized record code", err_val_L);
			break;
		case DATACOUNT_ERROR_1 :
			printf("\n(%d)Error: data count exceeded 512 characters", err_val_L);
			break;
		case UNEXPECTED_CHAR_FOUND :
			printf("\n(%d)Found ':'! at position 0", err_val_L);
			break;
		case ERROR_RECORD_FIELD :
			printf("\n(%d)Error in input record field", err_val_L);
			break;
		case BAD_SEQUENCE_1 :
			printf("\n(%d)Error: 01 encountered but not finished", err_val_L);
			break;
		case BYTE_COUNT_ERROR_3 :
			printf("\n(%d)Error: byte count not 2 on 02 record", err_val_L);
			break;
		case ADDRESS_ERROR_1 :
			printf("\n(%d)Error: addr not 0000 on 02 record", err_val_L);
			break;
		case BAD_SEQUENCE_2 :
			printf("\n(%d)Error: 03 encountered again in header", err_val_L);
			break;						
		case ALL_SECTORS_PROTECTED :
			printf("\n(%d)Warning: all sectors protected. Cannot erase", err_val_L);
			break;
		case SOME_SECTORS_PROTECTED :
			printf("\n(%d)Some sectors are protected. Please unprotect first", err_val_L);
			break;						
		case NOT_FOR_28F_DEVICES :
			printf("\n(%d)This function is not applicable to 28F series devices", err_val_L);
			break;
		case THIS_SECTOR_PROTECTED :
			printf("\n(%d)This sector is protected. Cannot perform the erase w/o\n", err_val_L);
			printf("unprotecting first\n");
			break;
		case CANNOT_RE_ERASE_SECTOR :
			printf("\n(%d)Cannot re-erase sector.", err_val_L);
            break;
		case PLSCNT_GT_25 :
			printf("\n(%d)PLSCNT exceeds 25. Aborting...", err_val_L);
			break;
		case CANNOT_REPROTECT_SECTOR :
			printf("\n(%d)Cannot re-protect this sector.", err_val_L);
			break;						
		case CANT_REUNPROTECT_SECTOR :
			printf("\n(%d)Cannot re-unprotect this sector.", err_val_L);
            break;
		case PLSCNT_GT_1000 :
			printf("\n(%d)PLSCNT exceeds 1000. Aborting...", err_val_L);
			break;                        
		case PROGRAMMING_FAILURE :
			_ultoa(this_address_G, string_GA, 16);
			printf("\n(%d)DQ6-Programming failure. Address = %s", err_val_L, string_GA);
			printf("\n curr_data = %x, in_data = %x", curr_data_G, errdata_G);
			break;
		case DQ6_PROGRAM_FAILURE :
			_ultoa(this_address_G, string_GA, 16);
			printf("\n(%d)DQ6-Programming failure after DQ5=1. Address = %s", err_val_L, string_GA);
			printf("\n curr_data = %x, in_data = %x", curr_data_G, errdata_G);
			break;
		case DQ6_KEEPS_TOGGLING :
			printf("\n(%d)DQ6-Programming failure after DQ5=1. DQ6 never stops toggling.", err_val_L);
			_ultoa(this_address_G, string_GA, 16);
			printf("\nAddress = %s", string_GA);
			break;
		case DQ6_ERASE_FAILURE_DATA :
			printf("\n(%d)DQ6-Erase failure. in_data = %x", err_val_L, errdata_G);
			break;
		case DQ6_ERASE_FAILURE_DQ5 :
			printf("\n(%d)DQ6-Erase failure after DQ5=1. in_data = %x", err_val_L, errdata_G);
			break;
		case DQ6_ERASE_ON_28F :
			printf("\n(%d)DQ6-Erase failure on 28F device. Chip is bad.", err_val_L);
			break;
		case DQ6_ERASE_ON_29F :
			printf("\n(%d)DQ6-Erase failure on 29F device. Sector is bad.", err_val_L);
			break;
		case DQ7_PROGRAM_FAILURE :
			_ultoa(this_address_G, string_GA, 16);
			printf("\n(%d)DQ7-Programming failure. Address = %s", err_val_L, string_GA);
			printf("\n curr_data = %x, in_data = %x", curr_data_G, errdata_G);
			break;
		case DQ7_FAIL_AFTER_DQ5_1 :
			_ultoa(this_address_G, string_GA, 16);
			printf("\n(%d)DQ7-Programming failure after DQ5=1. Address = %s", err_val_L, string_GA);
			printf("\n curr_data = %x, in_data = %x", curr_data_G, errdata_G);
			break;
		case DQ7_FAIL_AFTER_DQ5_2 :
			 printf("\n(%d)DQ7-Programming failure after DQ5=1.", err_val_L);
			_ultoa(this_address_G, string_GA, 16);
			printf("\nAddress = %s", string_GA);		
			break;
		case DQ7_ERASE_FAILURE :
			printf("\n(%d)DQ7-Erase failure. in_data = %x", err_val_L, errdata_G);
			break;
		case DQ7_ERASE_FAILURE_DQ5 :
			printf("\n(%d)DQ7-Erase failure after DQ5=1. in_data = %x", err_val_L, errdata_G);
			break;
		case DQ7_ERASE_ON_28F :
			printf("\n(%d)DQ7-Erase failure on 28F device. Chip is bad.", err_val_L);
			break;
		case DQ7_ERASE_ON_29F :
			printf("\n(%d)DQ7-Erase failure on 29F device. Sector is bad.", err_val_L);
			break;
		case ADDRESS_EXCEEDS_LIMITS :
			printf("\n(%d)Error: address exceeds device limits.", err_val_L);
			break;
		case UNKNOWN_BREAK_READHEX :
			printf("\n(%d)Error: undetermined break in read_hex().", err_val_L);
			break;
		case DEVICE_IS_NOT_AMD_1 :
			printf("\n(%d)Error! The device in this socket is either not an AMD Device or there is", err_val_L);
			printf("\nno device in the socket, not a %s. Please power down and remove the device", this_dev_GSP->devname_GS);
            break;
		case DEVICE_IS_NOT_AMD_2 :
			printf("\n(%d)*ERROR!* The device in this socket is either not an AMD Device or there is", err_val_L);
			printf("\nno device in the socket, not a %s. Please power down and remove the device", this_dev_GSP->devname_GS);
            break;
        case SELECT_DEVICE_FIRST :
        	printf("\n(%d)Error! You must select a device first in order to use this function.", err_val_L);
       		break;
       	case DEVICE_SIZE_EXCEEDED :
       		printf("\n(%d)Aborting programming...The last address of file to program exceeds this ", err_val_L);
       		printf("\ndevice's maximum limit. Please select a larger device or a smaller file to program.");
       		break;
       	case DQ5_ON_NO_POLL :
       		_ultoa(this_address_G, string_GA, 16);		
       		printf("\n(%d)Error! Operation failed: DQ5=1 in no_dq_poll().Address = %s\n", err_val_L, string_GA);
       		break;
		case CANT_USE_STD_PROG_ALG:
			printf("\n(%d)Error! The device in this socket is not a 28F010 or a 28F020.", err_val_L);
			printf("\nCannot use Standard Programming algorithm here.");
			break;
		case STD_PROG_ALG_FAIL:
			printf("\n(%d)Error! The device failed the Standard Programming Algorithm.", err_val_L);
			break;
		case CANT_USE_STD_ERASE_ALG:
			printf("\n(%d)Error! The device in this socket is not a 28F010 or a 28F020.", err_val_L);
			printf("\nCannot use Standard Erase algorithm here.");
			break;
		case STD_ERASE_ALG_FAIL:
			printf("\n(%d)Error! The device failed the Standard Erase Algorithm.", err_val_L);
			break;
		case STD_LL_PROG_FAIL:
			printf("\n(%d)Error! The devices failed the Parallel Programming Algorithm.", err_val_L);
			break;
		case STD_LL_ERASE_FAIL:
			printf("\n(%d)Error! The devices failed the Parallel Erase Algorithm.", err_val_L);
			break;
		case NOT_FOR_THIS_DEVICE :
			printf("\n(%d)This function is not applicable to %s devices", err_val_L, this_dev_GSP->devname_GS);
			break;
		case ERASE_SUSPEND_NOT_SUPP:
			printf("\n(%d)Error! Erase suspend not available for %s.", err_val_L, this_dev_GSP->devname_GS);
			break;
		case DEVICE_IS_A_29F016:
			printf("\n(%d)Error! The device in this socket is a 29F016, not a %s.", err_val_L, this_dev_GSP->devname_GS);
			break;
		case DEVICE_IS_A_29F080:
			printf("\n(%d)Error! The device in this socket is a 29F080, not a %s.", err_val_L, this_dev_GSP->devname_GS);
			break;
	}
	flerror_code_G = 0;	//re-initialize it

} /* error_handler */

    
/*******************************************************************/
/*	write_bin()                                                    */
/*                                                                 */
/*	purpose:	This routine reads the device and outputs to the   */
/*              screen or to a file (in binary mode).              */
/*                                                                 */
/*******************************************************************/
/*	parameters:	none                                               */
/*******************************************************************/
/*  return value:   flerror_code = 0 : successful                  */
/*                               = anything : unsuccessful         */
/*******************************************************************/    
int write_bin( void )
{

	char selection_L;
	int in_data_L;
	int i_L;
	int j_L;
	int wordMode_L;
	int inWord_L;
	int deviceType = this_dev_GSP->device_no_GS;
	unsigned long selection2_L;
	unsigned long max_addr_L;
	unsigned char temp[16];
	
	max_addr_L     = this_dev_GSP->kb_size_GS;
	flerror_code_G = 0;
	
	/* Is this a word operation? */
	switch ( deviceType ) {
		case Am29F100T:
		case Am29F100B:
		case Am29F200T:
		case Am29F200B:
		case Am29F400T:
		case Am29F400B:
			wordMode_L = 1;
			break;
		default:
			wordMode_L = 0;
			break;
	}

	printf("\n\t\t[1] Read device and output to screen");
	printf("\n\t\t[2] Return to main menu");
	printf("\nPlease enter number of needed selection: ");
	selection_L = getche();
	printf("\n");

	if (selection_L == '1') {
		printf("\nReading %s", this_dev_GSP->devname_GS);
		printf("\nEnter hexadecimal starting address: ");
		scanf("%Lx", &selection2_L);
		printf("\n");	
		if ( (selection2_L != 0) && (selection2_L <= max_addr_L) ) {
			address_G = selection2_L;
		}
		else if (selection2_L > max_addr_L) {
				return (ADDRESS_EXCEEDS_LIMITS);
		}
		else {
			address_G = 0;
		}
	}
	else {
		return( flerror_code_G );	
	}

	/* send data to user screen, 256 bytes at a time */
	do {

		for (j_L=0; j_L<16; j_L++) {

			/* For each of the 16 rows */
			if (address_G >= max_addr_L) {
				address_G = 0;
			}
			_ultoa(address_G, string_GA, 16);
			printf("\nA: %s  D: ", string_GA);

			/* Get the 16 bytes of the row */
			for ( i_L=0; i_L<16; i_L++, address_G++ ) {

				if (address_G >= max_addr_L) {
					address_G = 0;
				}

				split_addr(address_G);

				if ( wordMode_L ) {

					inWord_L = _inpw(base_decode_G+(curr_socket_G+1));
					printf("%2.2x ", inWord_L);
					split_bytes( inWord_L, &temp[ i_L+1 ], &temp[ i_L ] );
					if (temp[i_L] >= 0x07 && temp[i_L] <= 0x0d) {
						temp[i_L] = 0x2e;
					}
					if (temp[i_L+1] >= 0x07 && temp[i_L+1] <= 0x0d) {
						temp[i_L+1] = 0x2e;
					}
					
					/* Since this is a word, increment the byte count */
					i_L++;
					address_G++;
					
				}
				else {
					in_data_L = _inp(base_decode_G+(curr_socket_G+1));
					printf("%2.2x ", in_data_L);					
					temp[i_L] = (char) in_data_L;
					if (temp[i_L] >= 0x07 && temp[i_L] <= 0x0d) {
						temp[i_L] = 0x2e;
					}
				}

			} /* for each of the 16 bytes in a row */
                
            /* Filter the output */
			for ( i_L = 0; i_L < 16; i_L++ ) {
				if ( filter_read_G && (temp[i_L] > 127) )
					/* filter system is on & is unprintable */
					printf(".");
				else {
					printf("%c", temp[i_L]);
				}
        	}

		} /* For each of the 16 rows */
			
		printf("\n\t\t[0] to continue, any key to abort: ");
		if ( filter_read_G)
			printf("<Filtered> ");
		else
			printf("<Not Filtered> ");
		selection_L = getche();
		printf("\n");			

	} while (selection_L == '0'); 

	return( flerror_code_G );

} /* write_bin */


/*******************************************************************/
/*	sel_device()                                                   */
/*                                                                 */
/*	purpose:	This function is selected when the user has        */
/*				selected the device and type of package to use.    */
/*				The device is automatically verified by performing */
/*				the Auto-select functions, manuf_code, to check    */
/*				the Manufacturer's ID, and device_code, to check   */
/*				for the correct device in the socket. Returns an   */
/*				error message indicating type of device, if        */
/*              possible.                                          */
/*                                                                 */
/*******************************************************************/
/*	parameters:	none                                               */
/*******************************************************************/
/*  return value:   flerror_code = 0 : successful                  */
/*                               = anything : unsuccessful         */
/*******************************************************************/    
int sel_device( void )
{

	char selection_L;
	
	flerror_code_G = 0;

	printf("\n");
	printf("\t\t[1]	28F256A PLCC \t\t[3]	28F512A PLCC\n");
	printf("\t\t[2]	28F256A TSOP \t\t[4]	28F512A TSOP\n");
	printf("\t\t[5]	28F010A PLCC \t\t[7]    28F020A PLCC\n");
	printf("\t\t[6]	28F010A TSOP \t\t[8]    28F020A TSOP\n");
	printf("\t\t[9]	29F010 PLCC \t\t[B]	29F040 PLCC\n");
	printf("\t\t[A]	29F010 TSOP \t\t[C]	29F040 TSOP\n");
	printf("\t\t[D]	29F016\n");
	printf("\n");
	printf("\t\t[E]	28F010 PLCC \t\t[G]	28F020 PLCC\n");
	printf("\t\t[F]	28F010 TSOP \t\t[H]	28F020 TSOP\n");
	printf("\n");
	printf("\t\t[J]	29F100 TOP \t\t[K]	29F100 BOTTOM\n");
	printf("\t\t[L]	29F200 TOP \t\t[M]	29F200 BOTTOM\n");
	printf("\t\t[N]	29F400/A TOP \t\t[P]	29F400/A BOTTOM\n");
	printf("\t\t[S]	28F256 PLCC \t\t[T]	28F512 PLCC\n");
	printf("\t\t[U]	28F256 TSOP \t\t[V]	28F512 TSOP\n");
	printf("\t\t[Y] 29F080\n");
	printf("\n");
	printf("\t\t\t\t[0]	 Abort\n");
	printf("Enter the device to use: ");	
	selection_L = getche();
	printf("\n");
	
	switch ( selection_L ) {

		case '0' :
		return(0);		/* maintain old parameters(if any) */

		case '1' :
		this_dev_GSP = &amd_devices_GA[5];		//28F256A
		break;
		
		case '2' :
		this_dev_GSP = &amd_devices_GA[5];		//28F256A
		break;

		case '3' :
		this_dev_GSP = &amd_devices_GA[6];		//28F512A
		break;

		case '4' :
		this_dev_GSP = &amd_devices_GA[6];		//28F512A
		break;

		case '5' :
		this_dev_GSP = &amd_devices_GA[1];		//28F010A
		break;

		case '6' :
		this_dev_GSP = &amd_devices_GA[1];		//28F010A
		break;

		case '7' :
		this_dev_GSP = &amd_devices_GA[2];		//28F020A
		break;

		case '8' :
		this_dev_GSP = &amd_devices_GA[2];		//28F020A
		break;

   		case '9':
		this_dev_GSP = &amd_devices_GA[3];		//29F010
		break;

		case 'a':
		case 'A':
		this_dev_GSP = &amd_devices_GA[3];		//29F010
		break;
		
		case 'b':
		case 'B':
		this_dev_GSP = &amd_devices_GA[4];		//29F040
		break;

		case 'c':
		case 'C':
		this_dev_GSP = &amd_devices_GA[4];		//29F040
		break;

		case 'd':
		case 'D':
		this_dev_GSP = &amd_devices_GA[15];		//29F016
		break;

		case 'e':
		case 'E':
		this_dev_GSP = &amd_devices_GA[7];		//28F010
		break;

		case 'f':
		case 'F':
		this_dev_GSP = &amd_devices_GA[7];		//28F010
		break;
		
		case 'g':
		case 'G':
		this_dev_GSP = &amd_devices_GA[8];		//28F020
		break;

 		case 'h':
		case 'H':
		this_dev_GSP = &amd_devices_GA[8];		//28F020
		break;

 		case 'j':
		case 'J':
		this_dev_GSP = &amd_devices_GA[9];		//29F100T
		break;

 		case 'k':
		case 'K':
		this_dev_GSP = &amd_devices_GA[10];		//29F100B
		break;
 
  		case 'l':
		case 'L':
		this_dev_GSP = &amd_devices_GA[11];		//29F200T
		break;

  		case 'm':
		case 'M':
		this_dev_GSP = &amd_devices_GA[12];		//29F200B
		break;

  		case 'n':
		case 'N':
		this_dev_GSP = &amd_devices_GA[13];		//29F400T or 29F400AT
		break;

  		case 'p':
		case 'P':
		this_dev_GSP = &amd_devices_GA[14];		//29F400B or 29F400AB
		break;

  		case 's':
		case 'S':
		this_dev_GSP = &amd_devices_GA[16];		//28F256
		break;
		
  		case 'u':
		case 'U':
		this_dev_GSP = &amd_devices_GA[16];		//28F256
		break;

  		case 't':
		case 'T':
		this_dev_GSP = &amd_devices_GA[17];		//28F512
		break;
		
  		case 'v':
		case 'V':
		this_dev_GSP = &amd_devices_GA[17];		//28F512
		break;

  		case 'y':
		case 'Y':
		this_dev_GSP = &amd_devices_GA[18];		//29F080
		break;

		default :
		printf("\nNo device was selected. Returning to menu...");
		return(0);

	} /* switch */
					
	/* now check to see if it is in the socket */

	if (selection_L != '0') {
			
		manuf_code(FL_29F);		// do 29F check first
		if (m_code_G != 0x01) {
			manuf_code(FL_28F);	//last resort
			if (m_code_G == 0x01)	// if it is AMD, get the device ID
				device_code(FL_28F);
		}
		else if (m_code_G == 0x01)	//29F detected
			device_code(FL_29F);	//get its ID
			

		if (m_code_G == 0x01) { // AMD device
			if (d_code_G == this_dev_GSP->device_id_GS) {
				/* print - ok AMD device selected is correct */
				printf("Correct device selected\n");
				no_device_G=0;	/* reset the flag */
				return(0);		/* successful: flerror_code = 0 */
			}
			else {	// maybe the user entered the wrong ID
				switch(d_code_G) {
					case 0xA2:	// print - error: device in socket is 28F010A 
						printf("\nError! The device in this socket is a 28F010A, not a %s.", this_dev_GSP->devname_GS);
						printf("\nAdjusting . . .");
						this_dev_GSP = &amd_devices_GA[1];
						no_device_G = 0;
						break;
					case 0x29:  // print - error: device in socket is 28F020A 
						printf("\nError! The device in this socket is a 28F020A, not a %s.", this_dev_GSP->devname_GS);
						printf("\nAdjusting . . .");
						this_dev_GSP = &amd_devices_GA[2];
						no_device_G = 0;
						break;
					case 0x20:	// print - error: device in socket is 29F010 
						printf("\nError! The device in this socket is a 29F010, not a %s.", this_dev_GSP->devname_GS);
						printf("\nAdjusting . . .");
						this_dev_GSP = &amd_devices_GA[3];
						no_device_G = 0;
						break;
					case 0xA4:	// print - error: device in socket is 29F040 
						printf("\nError! The device in this socket is a 29F040, not a %s.", this_dev_GSP->devname_GS);
						printf("\nAdjusting . . .");
						this_dev_GSP = &amd_devices_GA[4];
						no_device_G = 0;
						break;
					case 0xA1:	// print - error: device in socket is 28F256 
						printf("\nError! The device in this socket is a 28F256, not a %s.", this_dev_GSP->devname_GS);
						printf("\nAdjusting . . .");
						this_dev_GSP = &amd_devices_GA[16];
						no_device_G = 0;
						break;
					case 0x2F:	// print - error: device in socket is 28F256A 
						printf("\nError! The device in this socket is a 28F256A, not a %s.", this_dev_GSP->devname_GS);
						printf("\nAdjusting . . .");
						this_dev_GSP = &amd_devices_GA[5];
						no_device_G = 0;
						break;
					case 0x25:	// print - error: device in socket is 28F512 
						printf("\nError! The device in this socket is a 28F512, not a %s.", this_dev_GSP->devname_GS);
						printf("\nAdjusting . . .");
						this_dev_GSP = &amd_devices_GA[17];
						no_device_G = 0;
						break;
					case 0xAE:	// print - error: device in socket is 28F512A 
						printf("\nError! The device in this socket is a 28F512A, not a %s.", this_dev_GSP->devname_GS);
						printf("\nAdjusting . . .");
						this_dev_GSP = &amd_devices_GA[17];
						no_device_G = 0;
						break;
					case 0xA7:	// print - error: device in socket is 28F010 
						printf("\nError! The device in this socket is a 28F010, not a %s.", this_dev_GSP->devname_GS);
						printf("\nAdjusting . . .");
						this_dev_GSP = &amd_devices_GA[7];
						no_device_G = 0;
						break;
					case 0x2A:	// print - error: device in socket is 28F020 
						printf("\nError! The device in this socket is a 28F020, not a %s.", this_dev_GSP->devname_GS);
						printf("\nAdjusting . . .");
						this_dev_GSP = &amd_devices_GA[8];
						no_device_G = 0;
						break;
					case 0x51:	// print - error: device in socket is 29F200T 
						printf("\nError! The device in this socket is a 29F200T, not a %s.", this_dev_GSP->devname_GS);
						printf("\nAdjusting . . .");
						this_dev_GSP = &amd_devices_GA[11];
						no_device_G = 0;
						break;
					case 0x52:	// print - error: device in socket is 29F200B 
						printf("\nError! The device in this socket is a 29F200B, not a %s.", this_dev_GSP->devname_GS);
						printf("\nAdjusting . . .");
						this_dev_GSP = &amd_devices_GA[12];
						no_device_G = 0;
						break;
					case 0x23:	// print - error: device in socket is 29F400T or 29F400AT 
						printf("\nError! The device in this socket is a 29F400T, not a %s.", this_dev_GSP->devname_GS);
						printf("\nAdjusting . . .");
						this_dev_GSP = &amd_devices_GA[13];
						no_device_G = 0;
						break;
					case 0xAB:	// print - error: device in socket is 29F400B or 29F400AB 
						printf("\nError! The device in this socket is a 29F400B, not a %s.", this_dev_GSP->devname_GS);
						printf("\nAdjusting . . .");
						this_dev_GSP = &amd_devices_GA[14];
						no_device_G = 0;
						break;
					case 0xAD:	// print - error: device in socket is 29F016 
						flerror_code_G = DEVICE_IS_A_29F016;
						no_device_G = 1;
						return(flerror_code_G);
						break;
					case 0xD5:	// print - error: device in socket is 29F080 
						flerror_code_G = DEVICE_IS_A_29F080;
						no_device_G = 1;
						return(flerror_code_G);
						break;
					default:	// print - error: unknown AMD device code 
						flerror_code_G = DEVICE_IS_NOT_AMD_1;
						no_device_G = 1;
						return(flerror_code_G);
						break;
								
				} /* switch */
			}
		}
		else if (m_code_G != 0x01) { // Not AMD 
			// print - uh oh not an AMD Device 
			// power down, remove and insert only AMD devices 
			flerror_code_G = DEVICE_IS_NOT_AMD_2;
			no_device_G    = 1;
			return(flerror_code_G);
		}
	}

	return( flerror_code_G );
	
} /* sel_device */


/*******************************************************************/
/*	emb_erase()                                                    */
/*                                                                 */
/*	purpose:	Performs the AMD Embedded Erase algorithm. If the  */
/*              device is a 28F 12v device, the 12v select line is */
/*				pulsed, a don't care address (0h) is issued, and   */
/*              the setup and erase commands are sent to the       */
/*              correct socket. If the device is a 29F 5v device,  */
/*              it is checked for protected sectors, and aborts if */
/*				so, else a polling address is setup, the 6-cycle   */
/*              unlock sequence is initiated, and either 5v or 12v */
/*              devices are polled using dq7_poll(). Upon comple-  */
/*              tion of the erase operation, the 12v devices are   */
/*              powered down from 12v and reset. The 5v devices    */
/*              are reset twice on termination of the erase op.    */
/*                                                                 */
/*******************************************************************/
/*	parameters:	none                                               */
/*******************************************************************/
/*  return value:   flerror_code = 0 : successful                  */
/*                               = anything : unsuccessful         */
/*******************************************************************/    
int emb_erase( void )
{

	int i_L;
	int temp_L;
	int deviceType = this_dev_GSP->device_no_GS;
	
	address_G      = 0;
	flerror_code_G = 0;

    /* the following section does:
		1. scan for any protected sectors on 29F devices
		2. if all sectors are protected on the current device, the erase
			operation cannot continue
		3. if some sectors are protected, a loop is used to find the first
			unprotected sector so that its address can be used for polling,
			because polling at a protected sector address may generate an
			error as well as being undefined. */

	switch ( deviceType ) {

		case Am29F010:
		case Am29F040:
		case Am29F100T:
		case Am29F100B:
		case Am29F200T:
		case Am29F200B:
		case Am29F400T:
		case Am29F400B:
		case Am29F080:
		case Am29F016:

			printf("\nVerifying sectors...");		
			flerror_code_G = sector_verify();
			if ( flerror_code_G ) {
				return ( flerror_code_G );
			}

			for ( i_L = 0; i_L < this_dev_GSP->num_sectors; i_L++ ) {
				if (prot_sects_GA[i_L] == 1) {
					printf("\nSome sectors are protected. Chip Erase will not erase them");
					temp_L=0;
					while ((prot_sects_GA[temp_L++] == 1) && (temp_L < this_dev_GSP->num_sectors));
					if (temp_L < this_dev_GSP->num_sectors) {
						/* found an unprotected sector (prot_sects_GA[temp_L] = 0) */
						switch ( deviceType ) {
							case Am29F010:
								poll_addr_G = addr_29F010_GA[temp_L];
								break;
							case Am29F040:
								poll_addr_G = addr_29F040_GA[temp_L];
								break;
							case Am29F100T:
								poll_addr_G = addr_29F100T_GA[temp_L];
								break;
							case Am29F100B:
								poll_addr_G = addr_29F100B_GA[temp_L];
								break;
							case Am29F200T:
								poll_addr_G = addr_29F200T_GA[temp_L];
								break;
							case Am29F200B:
								poll_addr_G = addr_29F200B_GA[temp_L];
								break;
							case Am29F400T:
								poll_addr_G = addr_29F400T_GA[temp_L];
								break;
							case Am29F400B:
								poll_addr_G = addr_29F400B_GA[temp_L];
								break;
							case Am29F080:
								poll_addr_G = addr_29F080_GA[temp_L];
								break;
							case Am29F016:
								poll_addr_G = addr_29F016_GA[temp_L];
								break;
							default :	
								/* device is not supported */
								flerror_code_G = NOT_FOR_THIS_DEVICE;
								return( flerror_code_G );
						}
						i_L = this_dev_GSP->num_sectors; /* force a break out of the for-loop */
					}
					else {	/* temp_L >= this_dev_GSP->num_sectors */
						/* all sectors are protected, cannot continue */
						flerror_code_G = ALL_SECTORS_PROTECTED;
						return(flerror_code_G);
					}
				}
				else {
					poll_addr_G = i_L;
				}
	 		} /* for */

	 	/* the following statement is needed only for a 29F sectored device, because
	 		a 28F device has no sectors, therefore the poll_address = X */
		address_G = poll_addr_G;	/* needed because address is used in DQx rtns */

		break;

		/* The following devices are supported, but do not need sector checking */
		case Am28F010A:
		case Am28F020A:
		case Am28F512A:
		case Am28F256A:
			break;

		default :	
			/* device is not supported */
			flerror_code_G = NOT_FOR_THIS_DEVICE;
			return( flerror_code_G );

	 } /* switch */

	printf("\nErasing chip : %s\n", this_dev_GSP->devname_GS);

	/* setup for 28F devices */
	switch ( deviceType ) {

		case Am28F010A: 
		case Am28F020A:
		case Am28F256A: 
		case Am28F512A:
			/* Setup 12V here */
			split_addr(0x00000000);
			/* Send Erase setup command */
			/*_outp(base_decode_G+(curr_socket_G+1), 0x30);*/	/* setup */
			/* Send Erase command */
			/*_outp(base_decode_G+(curr_socket_G+1), 0x30);*/	/* erase */
			break;

		case Am29F010:
		case Am29F040:
		case Am29F080:
		case Am29F016:
			split_addr(0x00005555);
			/* Send data */
			/* _outp(base_decode_G+(curr_socket_G+1), 0xAA);*/
			split_addr(0x00002AAA);
			/* Send data */
			/* _outp(base_decode_G+(curr_socket_G+1), 0x55);*/
			split_addr(0x00005555);
			/* Send data */
			/* _outp(base_decode_G+(curr_socket_G+1), 0x80);*/
			split_addr(0x00005555);
			/* Send data */
			/* _outp(base_decode_G+(curr_socket_G+1), 0xAA);*/
			split_addr(0x00002AAA);
			/* Send data */
			/* _outp(base_decode_G+(curr_socket_G+1), 0x55);*/
			split_addr(0x00005555);
			/* Send data */
			/* _outp(base_decode_G+(curr_socket_G+1), 0x10);*/
			split_addr(poll_addr_G);			/* use the poll_addr_G */
			break;

		default:  /* device is Am29FX00 */
			split_addr(0x00005555);
			/* Send 16 bit data */
			/* _outpw(base_decode_G+(curr_socket_G+1), 0x00AA);*/
			split_addr(0x00002AAA);
			/* Send 16 bit data */
			/* _outpw(base_decode_G+(curr_socket_G+1), 0x0055);*/
			split_addr(0x00005555);
			/* Send 16 bit data */
			/* _outpw(base_decode_G+(curr_socket_G+1), 0x0080);*/
			split_addr(0x00005555);
			/* Send 16 bit data */
			/* _outpw(base_decode_G+(curr_socket_G+1), 0x00AA);*/
			split_addr(0x00002AAA);
			/* Send 16 bit data */
			/* _outpw(base_decode_G+(curr_socket_G+1), 0x0055);*/
			split_addr(0x00005555);
			/* Send 16 bit data */
			/* _outpw(base_decode_G+(curr_socket_G+1), 0x0010);*/
			split_addr(poll_addr_G);			/* use the poll_addr_G */
			break;
			
	}

	if (!no_poll_G) {
		printf("\nUsing DQx polling");
		if (use_dq7_G)
			flerror_code_G = dq7_poll(FL_CH_ERASE);	/* dq7 poll for erase op */
		else
			flerror_code_G = dq6_poll(FL_CH_ERASE);
	}
	else {
		printf("\nUsing no_poll");
		flerror_code_G = no_dq_poll(FL_CH_ERASE);
	}

	if ( flerror_code_G ) {
		switch ( deviceType ) {
			case Am28F010A:
			case Am28F020A:
			case Am28F256A:
			case Am28F512A:
				/* switch off the 12V supply here */
				reset_fl(FL_28F);
				break;
			default:
				reset_fl(FL_29F);
		}
		return(flerror_code_G);
	}		

	switch ( deviceType ) {
		case Am28F010A:
		case Am28F020A:
		case Am28F256A:
		case Am28F512A:
			/* switch off the 12V supply here */
			break;
		default:
			break;
	}

	printf("\nSuccessfully erased %s \n", this_dev_GSP->devname_GS);
	return( flerror_code_G );		

} /* emb_erase */


/*******************************************************************/
/*	std_erase()                                                    */
/*                                                                 */
/*	purpose:	Performs the AMD Standard Erase algorithm. If the  */
/*              device is a 28F 12v device, the 12v select line is */
/*				pulsed, a don't care address (0h) is issued, and   */
/*              the setup and erase commands are sent to the       */
/*              correct socket. Upon completion of the erase 	   */
/*				operation, the 12v devices are powered down from   */
/*				12v and reset. 								       */
/*                                                                 */
/*******************************************************************/
/*	parameters:	none                                               */
/*******************************************************************/
/*  return value:   flerror_code = 0 : successful                  */
/*                               = anything : unsuccessful         */
/*******************************************************************/    
int std_erase( void )
{

	int iPulseCnt  = 0;
	int deviceType = this_dev_GSP->device_no_GS;
	unsigned long max_addr_L;
	register unsigned long curr_addr_L;
	
	flerror_code_G = 0;

	switch ( deviceType ) {
	
		case Am28F010:
		case Am28F020:
		case Am28F256:
		case Am28F512:

			printf("\nErasing chip : %s\n", this_dev_GSP->devname_GS);

			/* get address or start at sector */
			max_addr_L = this_dev_GSP->kb_size_GS;

			/* We need to first program ALL bytes in the device to 00H */

			/* switch on the 12V supply here */

			for ( curr_addr_L = 0; curr_addr_L < max_addr_L; curr_addr_L++ ) {
				if (curr_addr_L % 0x100 == 0 || max_addr_L - curr_addr_L < 0x100) {
					_ultoa(curr_addr_L, string_GA, 16);
					printf("Pre-programming Address (x100h): %s\r", string_GA);
				}
				flerror_code_G = std_program( curr_addr_L, 0x00 );
				/* Make sure there was no problem in programming */
				if ( flerror_code_G ) {
					printf("\nProblem in pre-programming device to all 00H.");
					return( flerror_code_G );
				}
			}

			/* switch off the 12V supply here */
			reset_fl(FL_28F);

			/* Apply Vpph here*/

			/* Write Erase Set-up Command, address is XXh */
			/* _outp(base_decode_G+(curr_socket_G+1), 0x20);*/
			/* Write Erase Command, address is XXh */
			/* _outp(base_decode_G+(curr_socket_G+1), 0x20);*/

			/* Time out for 10 ms here */

			/* Check all the bytes on the chip, one at a time... */
			for ( curr_addr_L = 0; (curr_addr_L < max_addr_L) && (iPulseCnt < 1000); ) {

				if (curr_addr_L % 0x100 == 0 || max_addr_L - curr_addr_L < 0x100) {
					_ultoa(curr_addr_L, string_GA, 16);
					printf("Erase_verify Addr (x00h): %s plscnt: %d\r", string_GA, iPulseCnt);
				}

				/* Write Erase Verify Command */
				split_addr( curr_addr_L );
				/* _outp(base_decode_G+(curr_socket_G+1), 0xA0);*/
				
				/* Time out for 6 us here */

				/* Read Data from device and Verify Byte */
				if ( _inp(base_decode_G+(curr_socket_G+1)) != 0xFF) {
					iPulseCnt++;
					/* Write Erase Set-up Command */
					/* _outp(base_decode_G+(curr_socket_G+1), 0x20);*/
					
					/* Write Erase Command */
					/* _outp(base_decode_G+(curr_socket_G+1), 0x20);*/

					/* Time out for 10 ms here */
				}
				else {
					curr_addr_L++;
				}

           	} /* for */

			if ( iPulseCnt >= 1000 ) {
				/* PLSCNT > 1000 => the device did not erase properly */
				flerror_code_G = STD_ERASE_ALG_FAIL;
			}

			split_addr(0x00000000);
			/* Write Reset Command, reset the register for read operations */
			/* _outp(base_decode_G+(curr_socket_G+1), 0xFF);*/
			
			/* Apply Vppl here */

			if ( iPulseCnt < 1000 ) {
				printf("\nSuccessfully erased %s \n", this_dev_GSP->devname_GS);
			}
			break;
			
		default :	
			/* device is not 28F010, 28F020, 28F256 or 28F512 */
			flerror_code_G = CANT_USE_STD_ERASE_ALG;
			break;

	} /* switch */		

	return(flerror_code_G);

} /* std_erase */


/*******************************************************************/
/*	std_parallel_erase()                                           */
/*                                                                 */
/*	purpose:	Performs the AMD Standard Erase algorithm in 	   */
/*				parallel.	 								       */
/*                                                                 */
/*******************************************************************/
/*	parameters:	none                                               */
/*******************************************************************/
/*  return value:   flerror_code = 0            : successful       */
/*                               = anything else: unsuccessful     */
/*******************************************************************/    
int std_parallel_erase( void )
{

	int iPulseCnt1 = 0;
	int iPulseCnt2 = 0;
	int iReadWord  = 0;
	int deviceType = this_dev_GSP->device_no_GS;
	unsigned long max_addr_L;
	unsigned long curr_addr_L;
	
	flerror_code_G = 0;

	switch ( deviceType ) {
	
		case Am28F010:
		case Am28F020:
		case Am28F256:
		case Am28F512:

			printf("\nErasing chip : %s\n", this_dev_GSP->devname_GS);

			/* get address or start at sector */
			max_addr_L = this_dev_GSP->kb_size_GS;

			/* We need to first program ALL bytes in the devices to 00H */
			/* Apply Vpph here*/

			for ( curr_addr_L = 0; curr_addr_L < max_addr_L; curr_addr_L++ ) {
				flerror_code_G = std_parallel_prog( curr_addr_L, 0x0000 );
				/* Make sure there was no problem in programming */
				if ( flerror_code_G ) {
					printf("\nProblem in pre-programming devices to all 00H.");
					return( flerror_code_G );
				}
			}

			/* Apply Vppl here */
			reset_fl(FL_28F);

			/* Apply Vpph */

			/* Write Erase Set-up Command, address is XXh */
			/* _outpw(base_decode_G+(curr_socket_G+1), 0x2020);*/
			
			/* Write Erase Command, address is XXh */
			/* _outpw(base_decode_G+(curr_socket_G+1), 0x2020);*/

			/* Time out for 10 ms here */

			/* Check all the bytes on the chip, one at a time... */
			for ( curr_addr_L = 0; 
				  (curr_addr_L < max_addr_L) && (iPulseCnt1 < 1000) && (iPulseCnt2 < 1000); 
				  curr_addr_L++ ) 
			{

				/* Write Erase Verify Command */
				split_addr( curr_addr_L );
				/* _outpw(base_decode_G+(curr_socket_G+1), 0xA0A0);*/
				
				/* Time out for 6 us here */

				/* Read Data from device */
				/* iReadWord = _inpw(base_decode_G+(curr_socket_G+1));*/
				
				/* Verify Word */
				if ( iReadWord != 0xFFFF ) {
					if ( (iReadWord && 0x00FF) == 0x00FF ) {
						/* Device 1 passed */
						iPulseCnt2++;
						/* Write Erase Set-up Command */
						/* _outpw(base_decode_G+(curr_socket_G+1), 0x20FF);*/
						
						/* Write Erase Command */
						/* _outpw(base_decode_G+(curr_socket_G+1), 0x20FF);*/

						/* Time out for 10 ms here */
					}
					else if ( (iReadWord && 0xFF00) == 0xFF00 ) {
						/* Device 2 passed */
						iPulseCnt1++;
						/* Write Erase Set-up Command */
						/* _outpw(base_decode_G+(curr_socket_G+1), 0xFF20);*/
						
						/* Write Erase Command */
						/* _outpw(base_decode_G+(curr_socket_G+1), 0xFF20);*/

						/* Time out for 10 ms here */
					}
					else {
						/* Both failed */
						iPulseCnt1++;
						iPulseCnt2++;
						/* Write Erase Set-up Command */
						/* _outpw(base_decode_G+(curr_socket_G+1), 0x2020);*/
						
						/* Write Erase Command */
						/*_outpw(base_decode_G+(curr_socket_G+1), 0x2020);*/

						/* Time out for 10 ms here */
					}
				}

           	} /* for */

			if ( iPulseCnt1 >= 1000 ) {
				/* PLSCNT > 1000 => the device did not erase properly */
				flerror_code_G = STD_LL_ERASE_FAIL;
			}
			if ( iPulseCnt2 >= 1000 ) {
				/* PLSCNT > 1000 => the device did not erase properly */
				flerror_code_G = STD_LL_ERASE_FAIL;
			}

			split_addr(0x00000000);
			/* Write Reset Command, reset the register for read operations */
			/* _outpw(base_decode_G+(curr_socket_G+1), 0xFFFF);*/
			
			/* Apply Vppl here */

    		if ( ( iPulseCnt1 < 1000 ) && ( iPulseCnt2 < 1000 ) ) {
				printf("\nSuccessfully erased devices in parallel\n");
			}
			break;
			
		default :	
			/* device is not 28F010, 28F020, 28F256 or 28F512 */
			flerror_code_G = CANT_USE_STD_ERASE_ALG;
			break;

	} /* switch */		

	return(flerror_code_G);

} /* std_parallel_erase */


/*******************************************************************/
/*	emb_program()                                                  */
/*                                                                 */
/*	purpose:	Performs the AMD Embedded Programming algorithm.   */
/*              If the  device is a 28F 12v device, the setup and  */
/*              erase commands are sent.If the device is a 29F 5v  */
/*              device, it is checked for protected sectors, and   */
/*              aborts if so, else a polling address is setup, the */
/*              address is started, a 4-cycle unlock sequence is   */
/*              initiated either 5v or 12v devices are polled using*/
/*              dq6_poll() or dq7_poll() for programming. Upon     */
/*              completion of the immediate byte program, execution*/
/*              returns to program(). Note that in order to return */
/*              to program(), dq6_poll() or dq7_poll() must have   */
/*              returned successfully.                             */
/*                                                                 */
/*******************************************************************/
/*	parameters:	unsigned long address = address to program         */
/*              unsigned int byte  = byte to program               */
/*******************************************************************/
/*  return value:   flerror_code = 0 : successful                  */
/*                               = anything : unsuccessful         */
/*******************************************************************/    
int emb_program(unsigned long address_L, unsigned int byte_L)
{

	int deviceType = this_dev_GSP->device_no_GS;

	flerror_code_G = 0;
	
	switch ( deviceType ) {
	
		case Am28F010A:
		case Am28F020A:
		case Am28F256A:
		case Am28F512A:
			split_addr(0x00000000);
			/* setup command */
			/* Send data */
			/* _outp(base_decode_G+(curr_socket_G+1), 0x50);*/
			
			/* Set the address to program at */
			split_addr(address_G);
			/* output the byte to the device */
			/* Send data */
			/* _outp(base_decode_G+(curr_socket_G+1), byte_L);*/
			break;
			
		case Am29F010:
		case Am29F040:
		case Am29F080:
		case Am29F016:
			split_addr(0x00005555);
			/* Send data */
			/* _outp(base_decode_G+(curr_socket_G+1), 0xAA);*/
			split_addr(0x00002AAA);
			/* Send data */
			/* _outp(base_decode_G+(curr_socket_G+1), 0x55);*/
			split_addr(0x00005555);
			/* Send data */
			/* _outp(base_decode_G+(curr_socket_G+1), 0xA0);*/
			
			/* Set the address to program at */
			split_addr(address_G);
			/* output the byte to the device */
			/* Send data */
			/* _outp(base_decode_G+(curr_socket_G+1), byte_L);*/
			break;

		default :	/* device is 5v 29FX00 */
			split_addr(0x00005555);
			/* Send 16 bit data */
			/* _outpw(base_decode_G+(curr_socket_G+1), 0x00AA);*/
			split_addr(0x00002AAA);
			/* Send 16 bit data */
			/* _outpw(base_decode_G+(curr_socket_G+1), 0x0055);*/
			split_addr(0x00005555);
			/* Send 16 bit data */
			/* _outpw(base_decode_G+(curr_socket_G+1), 0x00A0);*/
			/* Set the address to program at */
			split_addr(address_G);
			/* output the byte to the device */
			/* Send data */
			/* _outpw(base_decode_G+(curr_socket_G+1), byte_L);*/
			break;

	}		

	curr_data_G = byte_L;

	if ( ! no_poll_G ) {
		if ( use_dq7_G ) {
			flerror_code_G = dq7_poll(FL_PROG);	// dq7 poll for program op 
		}
		else {
			flerror_code_G = dq6_poll(FL_PROG);
		}
	}
	else {
		flerror_code_G = no_dq_poll(FL_PROG);
	}

	if ( flerror_code_G != 0 ) {
		return( flerror_code_G );
	}
	
	return( flerror_code_G );

} /* emb_program */


/*******************************************************************/
/*	std_program()                                                  */
/*                                                                 */
/*	purpose:	Performs the AMD Standard Programming algorithm.   */
/*              For 28F devices, a don't care address (0h) is      */
/*              issued, and the setup and erase commands are sent  */
/*              to the correct socket. Upon completion of the      */
/*              immediate byte program, the routine returns to 	   */
/*              program().                                         */
/*******************************************************************/
/*	parameters:	unsigned long address = address to program         */
/*              unsigned int byte  = byte to program               */
/*******************************************************************/
/*  return value:   flerror_code = 0 : successful                  */
/*                               = anything : unsuccessful         */
/*******************************************************************/    
int std_program(unsigned long address_L, unsigned int byte_L)
{

	int iPulseCnt = 0;
	int bSuccess  = FALSE;
	
	flerror_code_G = 0;

	switch ( this_dev_GSP->device_no_GS ) {
	
		case Am28F010:
		case Am28F020:
		case Am28F256:
		case Am28F512:

			split_addr(address_L);
			for ( iPulseCnt = 0, status = 0; iPulseCnt < 25; iPulseCnt++, status = iPulseCnt ) {
				/* Write Program Set-up Command */
				/* _outp(base_decode_G+(curr_socket_G+1), 0x40);*/
				
				/* Write Program Command (A/D) */
				/* _outp(base_decode_G+(curr_socket_G+1), byte_L);*/
				
				/* Time out for 10 us */

				/* Write Program Verify Command */
				/* _outp(base_decode_G+(curr_socket_G+1), 0xC0);*/
				
				/* Time out for 6 us */

				/* check to see if the byte we're programming is the same as the one 
			   	   on the device already, i.e. Read Data from device and verify byte */
				if ((unsigned)_inp(base_decode_G+(curr_socket_G+1)) == byte_L) {
					bSuccess  = TRUE;
					flerror_code_G = 0;		// no error, set to 0 so as not to send garbage
					break;
				}
			} /* for */

			if ( ! bSuccess ) {
				/* PLSCNT > 25 and the device did not program */
				flerror_code_G = STD_PROG_ALG_FAIL;
			}
			break;
			
		default :	
			/* device is not 28F010, 28F020, 28F256 or 28F512 */
			flerror_code_G = CANT_USE_STD_PROG_ALG;
			break;

	}		

	return(flerror_code_G);

} /* std_program */


/*******************************************************************/
/*	std_parallel_prog()                                            */
/*                                                                 */
/*	purpose:	Performs the AMD Standard Programming algorithm    */
/*              for 28Fxxx devices in parallel. This program takes */
/*              in a word, and splits it into 2 bytes, each byte   */
/*              going to a 28Fxxx device. In this algorithm, the   */
/*              device that finishes early, waits for the other    */
/*              device to complete before exiting the routine.     */
/*				Note : device 1 refers to the lower byte, and	   */
/*					   device 2 refers to the upper byte.		   */
/*******************************************************************/
/*	parameters:	unsigned long address = address to program         */
/*              unsigned int byte  = byte to program               */
/*******************************************************************/
/*  return value:   flerror_code = 0             : successful      */
/*                               = anything else : unsuccessful    */
/*******************************************************************/    
int std_parallel_prog(unsigned long address_L, unsigned int word_L)
{

	int iPulseCnt1;
	int iPulseCnt2;
	int bCompleted1 = FALSE;
	int bCompleted2 = FALSE;
	int iCurrWord   = 0;
	int iReadWord   = 0;
	
	flerror_code_G = 0;

	for ( iPulseCnt1 = 0, iPulseCnt2 = 0; (iPulseCnt1 < 25) && (iPulseCnt2 < 25); ) {

		if ( ( ! bCompleted1 ) && ( ! bCompleted2 ) ) {

			/* If both the device are waiting to be programmed */

			/* Store word to be programmed for comparison later on */
			iCurrWord = word_L;
			/* Write Program Set-up Command */
			split_addr(address_L);
			/* _outpw( base_decode_G+(curr_socket_G+1), 0x4040 );*/
			
			/* Write Program Command (A/D) */
			/* _outpw(base_decode_G+(curr_socket_G+1), word_L);*/
			
			/* Time out for 10 us here */

			/* Write Program Verify Command */
			/*	_outpw(base_decode_G+(curr_socket_G+1), 0xC0C0);*/
			
			/* Time out for 6 us here */

			/* check to see if the word we're programming is the same as the one 
		   	   on the device already, i.e. Read Data from device */
			/* iReadWord = _inpw(base_decode_G+(curr_socket_G+1));*/
			
			/* Verify Word */
			if ( (unsigned)iReadWord == word_L ) {
				/* We're done ... */
				break;
			}
			else if ( (iReadWord && 0x00FF) == (word_L && 0x00FF) ) {
				/* Device 1 passed, device 2 still not done... */
				bCompleted1 = TRUE;
				iPulseCnt2++;
			}
			else if ( (iReadWord && 0xFF00) == (word_L && 0xFF00) ) {
				/* Device 2 passed, device 1 still not done... */
				bCompleted2 = TRUE;
				iPulseCnt1++;
			}
			else {
				/* Both devices did not program, nothing to do, keep looping... */
				iPulseCnt1++;
				iPulseCnt2++;
			}
			
		} /* If both devices are waiting to be programmed */
		else if ( ! bCompleted1 ) {

			/* Device 1 still waiting to be programmed */

			/* Store word to be programmed for comparison later on */
			iCurrWord = (int)concat_bytes( 0x00, (0x00FF && word_L) );
			/* Write Program Set-up Command */
			
			split_addr(address_L);
			/* Send mask */
			/* _outpw( base_decode_G+(curr_socket_G+1), 0xFF40 );*/
			
			/* Write Program Command (A/D) */
			/* _outpw(base_decode_G+(curr_socket_G+1), iCurrWord );*/

			/* Time out for 10 us here */

			/* Write Program Verify Command */
			/* _outpw(base_decode_G+(curr_socket_G+1), 0xFFC0);*/
			
			/* Time out for 6 us here */

			/* check to see if the word we're programming is the same as the one 
		   	   on the device already, i.e. Read Data from device */
			/* iReadWord = _inpw(base_decode_G+(curr_socket_G+1));*/
			
			/* Verify lower byte */
			if ( (iReadWord && 0x00FF) == (iCurrWord && 0x00FF) ) {
				/* Device 1 passed, we're done... */
				break;
			}
			else {
				iPulseCnt1++;
			}
		} /* ! bCompleted1 */
		else if ( ! bCompleted2 ) {
			/* If device 2 still waiting to be programmed */

			/* Store word to be programmed for comparison later on */
			iCurrWord = (int)concat_bytes( (0xFF00 && word_L), 0x00 );
			/* Write Program Set-up Command */
			
			split_addr(address_L);
			/* Send mask */
			/* _outpw( base_decode_G+(curr_socket_G+1), 0x40FF );*/
			
			/* Write Program Command (A/D) */
			/* _outpw(base_decode_G+(curr_socket_G+1), iCurrWord );*/
			
			/* Time out for 10 us here */

			/* Write Program Verify Command */
			/* _outpw(base_decode_G+(curr_socket_G+1), 0xC0FF);*/
			
			/* Time out for 6 us here */

			/* check to see if the word we're programming is the same as the one 
		   	   on the device already, i.e. Read Data from device */
			/* iReadWord = _inpw(base_decode_G+(curr_socket_G+1));*/
			
			/* Verify upper byte */
			if ( (iReadWord && 0xFF00) == (iCurrWord && 0xFF00) ) {
				/* Device 2 passed, we're done... */
				break;
			}
			else {
				iPulseCnt2++;
			}
		} /* ! bCompleted2 */
		else {
			/* Should never get here... */
			printf( "\nError: Unknown state in parallel programming." );
			flerror_code_G = STD_LL_PROG_FAIL;
		}
				
	} /* for */

	if ( iPulseCnt1 >= 25 ) {
		printf("\nDevice 1 failed in parallel programming.");
		flerror_code_G = STD_LL_PROG_FAIL;
	}
	if ( iPulseCnt2 >= 25 ) {
		printf("\nDevice 2 failed in parallel programming.");
		flerror_code_G = STD_LL_PROG_FAIL;
	}

	return(flerror_code_G);

} /* std_parallel_prog */


/*******************************************************************/    
/* program()                                                       */
/* purpose: This routine obtains all the data needed from 		   */
/*            the files to program, and then invokes the           */
/*            routine appropriate to the device. That is,          */
/*            emb_program() for Embedded Alg. devices, or  		   */
/*            std_program() for the standard algorithm devices     */
/*******************************************************************/
/*	parameters:	int which = BIN or HEX, indicates what format to   */
/*                                      use                        */
/*                        = SINGLE, indicates a single byte prog.  */
/*******************************************************************/
/*  return value:   flerror_code = 0 : successful                  */
/*                               = anything : unsuccessful         */
/*******************************************************************/    
int program(int which_L)
{

	unsigned int out_data_L;
	unsigned long max_addr_L, selection_L;
	unsigned long i_L;
	int k_L;
	unsigned int in_data_L;
    int deviceType = this_dev_GSP->device_no_GS;
    
	flerror_code_G = 0;

	/* get address or start at sector */
	address_G  = 0; 
	max_addr_L = this_dev_GSP->kb_size_GS;

	if (last_file_addr_G > max_addr_L) {
		printf("\nWarning: Program addressing exceeds device limits.");
		printf("\nProgramming will abort after device limits are reached.");
	}
		
	/* if sectors are protected, send warning to user must unprotect
		first... */
	switch ( deviceType ) {

		case Am29F010:
		case Am29F040:
		case Am29F100T:
		case Am29F100B:
		case Am29F200T:
		case Am29F200B:
		case Am29F400T:
		case Am29F400B:
		case Am29F080:
		case Am29F016:
			printf("\nVerifying sectors...");
			flerror_code_G = sector_verify();
			if ( flerror_code_G ) {
				return ( flerror_code_G );
			}
			for ( k_L=0; k_L < this_dev_GSP->num_sectors; k_L++ ) {
				if (prot_sects_GA[k_L] == 1) {
					printf("\nEmb Prog: Sector %d is protected", k_L);
					flerror_code_G = SOME_SECTORS_PROTECTED;
					return(flerror_code_G);
				}     
			}
			break;

		default:
			;
	}

	/* the following avoids having to toggle Vpp for every byte, thereby
		only having to set Vpp once. Must Sleep to allow MosFets to 
		switch. This only affects 28F devices. */
	switch ( deviceType ) {
		case Am28F010A:
		case Am28F020A:
		case Am28F010 :
		case Am28F020 :
		case Am28F256A:
		case Am28F512A:
		case Am28F256 :
		case Am28F512 :
				/* Set Vpph here */
		default :
			break;
	}

	_ultoa(max_addr_L, string_GA, 16);
	printf("\nCurrent device: %s, max_addr_L = %s\n", this_dev_GSP->devname_GS, string_GA);

	/* parse the argument */
	if (which_L == BIN || which_L == SINGLE || which_L == WORD) {

		/* a Binary program only requires an address to start. If the file to program
		is larger than the last address on the Flash, programming continues until
		the Flash is fully programmed. If not, the Flash remains partially 
		programmed. */
		printf("\nNote: if starting address is not 0, this routine will continue until the\n");
		printf("last address of the device or up to the current file length.\n");
		printf("\nEnter hexadecimal starting address: ");
		scanf("%Lx", &selection_L);
		printf("\n");	

		if (selection_L != 0) {
			address_G=selection_L;
		}
		else if (selection_L >= max_addr_L) {
			flerror_code_G = ADDRESS_EXCEEDS_LIMITS;
			return(flerror_code_G);
		}
		else {
			address_G = 0;
		}
    }

	if (deviceType != Am28F010 && deviceType != Am28F020 && deviceType != Am28F512 && deviceType != Am28F256)
		{
		if (!no_poll_G)
			printf("\nUsing DQx polling\n");
		else
			printf("\nUsing no_poll\n");
		}
    	
	/* get address and data info */
	do {
		/* read the Binary program data into the array */
		if (which_L == BIN) {
			/* Get Binary data */
		}
		else if (which_L == HEX) {
			/* read the IHex program data into the array */
			/* Get the Intel Hex format data */
//			}
			address_G = this_address_G;
			if (address_G > max_addr_L) {
				/* if program address exceeds Flash max. address, then stop */
				printf("\nWarning: This program address exceeds device limits.");
				_ultoa(address_G, string_GA, 16);
				printf("\nAddress is %s. Aborting...", string_GA);
			}
		}
		else if (which_L == SINGLE) {
			/* this is a routine for a single byte program */
			i_L = 0;
			do {
				/* input the byte to program */
				printf("\nEnter decimal byte to program: ");
			    scanf("%d", &selection_L);
			    if (selection_L >= 0 && selection_L <= 0xFF)
			    	i_L=1;
			} while (!i_L);
			printf("\n");
			out_data_L = (int)selection_L;
			num_bytes_G=1;	/* only 1 byte to program */
		}
		else {
			//which_L = WORD
			/* this is a routine for a single word program */
			i_L = 0;
			do {
				/* input the word to program */
				printf("\nEnter decimal word to program: ");
			    scanf("%d", &selection_L);
			    if (selection_L >= 0 && selection_L <= 0xFFFF)
			    	i_L=1;
			} while (!i_L);
			printf("\n");
			out_data_L  = (int)selection_L;
			num_bytes_G = 1;	/* only 1 data to program */
		}
				
		for ( i_L=0; i_L < num_bytes_G; i_L++ ) {

			split_addr(address_G);

			/* check to see if the byte we're programming is the same as the one on the 
			  device already */
			switch ( deviceType ) {
				case Am29F100T:
				case Am29F100B:
				case Am29F200T:
				case Am29F200B:
				case Am29F400T:
				case Am29F400B:
					in_data_L = _inpw(base_decode_G+(curr_socket_G+1));
					break;
				case Am28F010:
				case Am28F020:
				case Am28F256:
				case Am28F512:
					/* issue reset commands so that we can read properly */
					/* _outp(base_decode_G+(curr_socket_G+1), 0xFF);*/
					/* _outp(base_decode_G+(curr_socket_G+1), 0xFF);*/
					/* in_data_L = _inp(base_decode_G+(curr_socket_G+1));*/
					break;
				default :
					/*in_data_L = _inp(base_decode_G+(curr_socket_G+1));*/
					break;
			}

			if ( (which_L != SINGLE) && (which_L != WORD) )  {
				/* if programming binary or hex, get the byte here */
				;
			}

			if ( out_data_L != in_data_L ) { //are they the same? No, then program it
				switch ( deviceType ) {
					case Am28F010:
					case Am28F020:
					case Am28F256:
					case Am28F512:
						flerror_code_G = std_program(address_G, out_data_L);
						break;
					default:
						flerror_code_G = emb_program(address_G, out_data_L);
				}
			}

			/* On Error, do the following: */
			if ( flerror_code_G != 0 ) {
				switch ( deviceType ) {
					case Am28F010A:
					case Am28F020A:
					case Am28F010 :
					case Am28F020 :
					case Am28F256A:
					case Am28F512A:
					case Am28F256 :
					case Am28F512 :
						/* Set Vppl here */
						reset_fl(FL_28F);
						break;
					default :
						reset_fl(FL_29F);
						break;
					}
					return (flerror_code_G);
				}

			/* See the status */
			if (address_G % 0x100 == 0 || (max_addr_L - address_G <= 0x100)) {
				_ultoa(address_G, string_GA, 16);
				printf("Address: %s\r", string_GA);
			}
			address_G++;
			if (address_G >= max_addr_L) {
				num_bytes_G = 0;
				/* print programming operation is done */
				printf("\nSuccessfully programmed %s \n", this_dev_GSP->devname_GS);
			}

		} /* for */

	} while (address_G <= (max_addr_L - 1));

	/* Reset the device */
	switch ( deviceType ) {
		case Am28F010A:
		case Am28F020A:
		case Am28F010 :
		case Am28F020 :
		case Am28F256A:
		case Am28F512A:
		case Am28F256 :
		case Am28F512 :
				/* Set Vppl here */
		default :
			break;
	}

	return( flerror_code_G );

} /* program */


/*******************************************************************/
/*	sector_erase()                                                 */
/*                                                                 */
/*	purpose:	Performs the AMD sector Erase algorithm. This will */
/*              only work with 5v 29F devices. The selected        */
/*              sectors are verified to obtain the protection      */
/*              status. If the sector(s) is protected, it aborts.  */
/*				Otherwise, the unlock cycle is issued. Erase       */
/*				continues by polling for erase operations. Upon    */
/*				completion, the device is reset.                   */
/*                                                                 */
/*******************************************************************/
/*	parameters:	int suspend = FALSE or TRUE, indicates whether to  */
/*                            use Erase Suspend or not             */
/*******************************************************************/
/*  return value:   flerror_code = 0 : successful                  */
/*                               = anything : unsuccessful         */
/*******************************************************************/    
int sector_erase( int suspend )
{	

	int j_L;
	int this_sect_L;
	int in_data_L  = 0x0;
	int bDone      = FALSE;
	int deviceType = this_dev_GSP->device_no_GS;
	unsigned long max_L;
	
	flerror_code_G = 0;

	switch ( deviceType ) {
		case Am28F010A:
		case Am28F020A:
		case Am28F010 :
		case Am28F020 :
		case Am28F256A:
		case Am28F512A:
		case Am28F256 :
		case Am28F512 :
			/* print error - not supported for the above devices */
			flerror_code_G = NOT_FOR_THIS_DEVICE;
			return(flerror_code_G);
		default:
			break;
	}

	if ( suspend ) {
		switch ( deviceType ) {
			case Am29F010 :
			case Am29F040 :
			case Am29F100T:
			case Am29F100B:
			case Am29F200T:
			case Am29F200B:
			case Am29F400T:
			case Am29F400B:
			case Am29F080 :
			case Am29F016 :
				break;
			default:
				/* print error - not supported for the remaining devices */
				flerror_code_G = ERASE_SUSPEND_NOT_SUPP;
				return(flerror_code_G);
		}
	}
	
	do {		

		printf("\nChecking sectors\n");
		flerror_code_G = sector_verify();
		if ( flerror_code_G ) {
			return ( flerror_code_G );
		}

		for ( j_L = 0; j_L < this_dev_GSP->num_sectors; j_L++ ) {

			/* for each sector, get the beginning and ending address */
			switch ( deviceType ) {

				case Am29F010:
					address_G = addr_29F010_GA[j_L];
					max_L 	  = addr_29F010_GA[j_L + 1] - 1;	/* end address */
					break;
				case Am29F040:
					address_G = addr_29F040_GA[j_L];
					max_L 	  = addr_29F040_GA[j_L + 1] - 1;	/* end address */
					break;
				case Am29F100T:
					address_G = addr_29F100T_GA[j_L];
					max_L 	  = addr_29F100T_GA[j_L + 1] - 1;	/* end address */
					break;
				case Am29F100B:
					address_G = addr_29F100B_GA[j_L];
					max_L 	  = addr_29F100B_GA[j_L + 1] - 1;	/* end address */
					break;
				case Am29F200T:
					address_G = addr_29F200T_GA[j_L];
					max_L 	  = addr_29F200T_GA[j_L + 1] - 1;	/* end address */
					break;
				case Am29F200B:
					address_G = addr_29F200B_GA[j_L];
					max_L 	  = addr_29F200B_GA[j_L + 1] - 1;	/* end address */
					break;
				case Am29F400T:
					address_G = addr_29F400T_GA[j_L];
					max_L 	  = addr_29F400T_GA[j_L + 1] - 1;	/* end address */
					break;
				case Am29F400B:
					address_G = addr_29F400B_GA[j_L];
					max_L 	  = addr_29F400B_GA[j_L + 1] - 1;	/* end address */
					break;
				case Am29F080:
					address_G = addr_29F080_GA[j_L];
					max_L 	  = addr_29F080_GA[j_L + 1] - 1;	/* end address */
					break;
				case Am29F016:
					address_G = addr_29F016_GA[j_L];
					max_L 	  = addr_29F016_GA[j_L + 1] - 1;	/* end address */
					break;
				default:
					flerror_code_G = NOT_FOR_THIS_DEVICE;
					return(flerror_code_G);
			}

			bDone = FALSE;	/* Reset the flag */
			
			for ( ; address_G <= max_L; address_G++ ) {

				split_addr(address_G);			/* send the address */
				switch ( deviceType ) {

					case Am29F010:
					case Am29F040:
					case Am29F080:
					case Am29F016:
						/* get data in x8 mode */
						/* in_data_L = _inp(base_decode_G+(curr_socket_G+1));*/
						if ( in_data_L != 0xFF ) {
							bDone = TRUE;
						}
						break;

					default: 
						/* get data in x16 mode */
						/* in_data_L = _inpw(base_decode_G+(curr_socket_G+1));*/
						if ( in_data_L != 0xFFFF ) {
							bDone = TRUE;
						}
						break;
				}
				if ( bDone ) {
					break;
				}
				
			} /* for */

			if ( bDone ) { /* failed: therefore, not erased */
				if (prot_sects_GA[j_L] != 0)
		    		printf("Sector [%d] : Unerased, Protected\n", j_L );
		    	else
		    		printf("Sector [%d] : Unerased\n", j_L );
		    	erase_sects_GA[j_L] = 1;	/* mark as erasable */
		    }
			else {					/* else already erased */
				if (prot_sects_GA[j_L] != 0) {
					printf("Sector [%d] : Erased, Protected\n", j_L);
				}
				else {
					printf("Sector [%d] : Erased\n", j_L);
				}
				erase_sects_GA[j_L] = 0;	/* mark as not re-erasable */
			}
		
		} /* for */

		printf("Return to Main menu [33]\n");
		printf("Please select the appropriate sector number to erase: ");
		scanf("%d", &this_sect_L);
		printf("\n");

		if ( (this_sect_L >= this_dev_GSP->num_sectors) || (this_sect_L < 0) ) {
			return ( flerror_code_G );
		}

		if (prot_sects_GA[this_sect_L] != 0) {
			/* print warning - sector is protected, cannot erase w/out
			unprotecting first - exit here; user must do the above first */
			flerror_code_G = THIS_SECTOR_PROTECTED;
			return(flerror_code_G);
		}

		if (erase_sects_GA[this_sect_L] == 0) {
			/* if the sector is already erased, we shouldn't re-erase it */
			flerror_code_G = CANNOT_RE_ERASE_SECTOR;
			return(flerror_code_G);
		}
		
		switch ( deviceType ) {

			case Am29F010:
			case Am29F040:
			case Am29F080:
			case Am29F016:
				/* x8 mode */
				/* send Unlock sequences */
				split_addr(0x00005555);
				/* Send data */
				/* _outp(base_decode_G+(curr_socket_G+1), 0xAA);*/
				split_addr(0x00002AAA);
				/* Send data */
				/* _outp(base_decode_G+(curr_socket_G+1), 0x55);*/
				split_addr(0x00005555);
				/* Send data */
				/* _outp(base_decode_G+(curr_socket_G+1), 0x80);*/
				split_addr(0x00005555);
				/* Send data */
				/* _outp(base_decode_G+(curr_socket_G+1), 0xAA);*/
				split_addr(0x00002AAA);
				/* Send data */
				/* _outp(base_decode_G+(curr_socket_G+1), 0x55);*/
				break;

			default: /* x16 mode */
				/* send Unlock sequences */
				split_addr(0x00005555);
				/* Send 16 bit data */
				/* _outpw(base_decode_G+(curr_socket_G+1), 0x00AA);*/
				split_addr(0x00002AAA);
				/* Send 16 bit data */
				/* _outpw(base_decode_G+(curr_socket_G+1), 0x0055);*/
				split_addr(0x00005555);
				/* Send 16 bit data */
				/* _outpw(base_decode_G+(curr_socket_G+1), 0x0080);*/
				split_addr(0x00005555);
				/* Send 16 bit data */
				/* _outpw(base_decode_G+(curr_socket_G+1), 0x00AA);*/
				split_addr(0x00002AAA);
				/* Send 16 bit data */
				/* _outpw(base_decode_G+(curr_socket_G+1), 0x0055);*/
				break;
		}

		if ( (this_sect_L >= 0) && (this_sect_L < this_dev_GSP->num_sectors) ) {

			printf("\nErasing sector %d on %s\n", this_sect_L, this_dev_GSP->devname_GS);

			switch ( deviceType ) {

				case Am29F010:
					address_G = addr_29F010_GA[this_sect_L];
					break;
				case Am29F040:
					address_G = addr_29F040_GA[this_sect_L];
					break;
				case Am29F100T:
					address_G = addr_29F100T_GA[this_sect_L];
					break;
				case Am29F100B:
					address_G = addr_29F100B_GA[this_sect_L];
					break;
				case Am29F200T:
					address_G = addr_29F200T_GA[this_sect_L];
					break;
				case Am29F200B:
					address_G = addr_29F200B_GA[this_sect_L];
					break;
				case Am29F400T:
					address_G = addr_29F400T_GA[this_sect_L];
					break;
				case Am29F400B:
					address_G = addr_29F400B_GA[this_sect_L];
					break;
				case Am29F080:
					address_G = addr_29F080_GA[this_sect_L];
					break;
				case Am29F016:
					address_G = addr_29F016_GA[this_sect_L];
					break;
				default:
					flerror_code_G = NOT_FOR_THIS_DEVICE;
					return(flerror_code_G);
			}

			poll_addr_G = address_G;

			/* send the address */
			split_addr(address_G);

			/* send the 6th Unlock cycle data */
			switch ( deviceType ) {

				case Am29F010:
				case Am29F040:
				case Am29F080:
				case Am29F016: 
					/* Send data in x8 mode */
					/* _outp(base_decode_G+(curr_socket_G+1), 0x30);*/
					break;
				default: 
					/* Send data in x16 mode */
					/* _outpw(base_decode_G+(curr_socket_G+1), 0x0030);*/
					break;
			}

            /**************************************************/
			/* Important: timeout to account for 100us window */
			/* ANY COMMAND OTHER THAN SECTOR ERASE OR ERASE   */
			/* SUSPEND DURING THIS PERIOD WILL RESET THE      */
			/* DEVICE TO THE READ MODE.                       */
			/* Please use 100us timeout for all devices.      */
			/**************************************************/

			/* Can optionally timeout for a few more milliseconds to get the
				erase underway before starting erase-suspend */
            
			if ( suspend && (deviceType != Am29F010) ) {
					/* Do erase suspend */
					flerror_code_G = erase_suspend( this_sect_L );
					if ( flerror_code_G ) {
						return( flerror_code_G );
				}
			}
            

			if (!no_poll_G) {
				if (use_dq7_G) {
					flerror_code_G = dq7_poll(FL_SC_ERASE); /* poll by DQ7 */
				}
				else {
					flerror_code_G = dq6_poll(FL_SC_ERASE);
				}
			}
			else {
				flerror_code_G = no_dq_poll(FL_SC_ERASE);
			}
			if (flerror_code_G != 0) {
				reset_fl(FL_29F);
				return(flerror_code_G);
			}
			/* print done */
			printf("\nSuccessfully erased sector %d\n", this_sect_L);
			/* avoid re-erasing the sector that was just erased */
			erase_sects_GA[this_sect_L] = 0;	

		} /* if valid sector */

	} while ( (this_sect_L < this_dev_GSP->num_sectors) && ( ! flerror_code_G) );

	return( flerror_code_G );		

} /* sector_erase */


/*******************************************************************/
/*	erase_suspend()                                                */
/*                                                                 */
/*	purpose:	Performs the Erase Suspend algorithm. This will    */
/*              only work with 5v 29F devices. The device is first */
/*				made ready for Read or Program (depending on the   */
/*				device). The user is then asked whether he/she     */
/*				wants to Read or Program. The corresponding algo   */
/*				(embedded) is then invoked.						   */
/*                                                                 */
/*******************************************************************/
/*	parameters:	int sect_num_L = refers to which sector was being  */
/*								 erased.	   				       */
/*******************************************************************/
/*  return value:   flerror_code = 0 : successful                  */
/*                               = anything : unsuccessful         */
/*******************************************************************/    
int erase_suspend( int sect_num_L )
{

	int selection_L;
	int i_L;
	int bValidInput            = FALSE;
	int iByteToProgram;
	int deviceType             = this_dev_GSP->device_no_GS;
	unsigned long selection2_L = 0;
	unsigned long monitor_addr_L;
	unsigned int  data1_L;
	unsigned int  data2_L;
	
	flerror_code_G = 0;

	/* First find out which address to use for monitoring the toggle bit.
	   We can only use the address of the sector that is not being erased */
	for ( i_L = 0; i_L < this_dev_GSP->num_sectors; i_L++ ) {

		switch ( deviceType ) {

			case Am29F040:
					monitor_addr_L = addr_29F040_GA[i_L];
					break;
			case Am29F100T:
					monitor_addr_L = addr_29F100T_GA[i_L];
					break;
			case Am29F100B:
					monitor_addr_L = addr_29F100B_GA[i_L];
					break;
			case Am29F200T:
					monitor_addr_L = addr_29F200T_GA[i_L];
					break;
			case Am29F200B:
					monitor_addr_L = addr_29F200B_GA[i_L];
					break;
			case Am29F400T:
					monitor_addr_L = addr_29F400T_GA[i_L];
					break;
			case Am29F400B:
					monitor_addr_L = addr_29F400B_GA[i_L];
					break;
			case Am29F080:
					monitor_addr_L = addr_29F080_GA[i_L];
					break;
			case Am29F016:
					monitor_addr_L = addr_29F016_GA[i_L];
					break;
			default:
					flerror_code_G = NOT_FOR_THIS_DEVICE;
					return(flerror_code_G);
		}
		
		/* If this address does not match the address of the sector being erased, we
		   can use it */
		if ( monitor_addr_L != address_G ) {
			break;
		}

	} /* for */

	/* Suspend the Sector Erase in progress */
	switch ( deviceType ) {

		case Am29F040:
		case Am29F080:
		case Am29F016:
			/* Send data in x8 mode */
			/* _outp(base_decode_G+(curr_socket_G+1), 0xB0);*/
			break;

		default:
			/* Send data in x16 mode */
			/* _outpw(base_decode_G+(curr_socket_G+1), 0x00B0);*/
			break;

	}

	/* Monitor the toggle bit (D6) using address of sector NOT being erased (found above) */
	split_addr(monitor_addr_L);
	
	do {

		switch ( deviceType ) {
    	
			case Am29F040:
			case Am29F080:
			case Am29F016: /* x8 mode */
				/* read 1st data */
				/* data1_L = _inp(base_decode_G+(curr_socket_G+1));*/
				
				/* read 2nd data */
				/* data2_L = _inp(base_decode_G+(curr_socket_G+1));*/
				break;

			default: /* x16 mode */
				/* read 1st data */
				/* data1_L = _inpw(base_decode_G+(curr_socket_G+1));*/
				
				/* read 2nd data */
				/* data2_L = _inpw(base_decode_G+(curr_socket_G+1));*/
				break;

		}

	} while ( (data1_L & 0x40) != (data2_L & 0x40) );

	/* OK, Erase successfully suspended */
	printf("\nSector Erase successfully suspended.\n");
	
	/* Ask user to either Read or Program */	
	do {

		printf("\n\t\t\tErase Suspend Menu\n");
		printf("\t\t\t-----------\n");			
		switch ( deviceType ) {
		
			case Am29F080:
			case Am29F016:
				printf("\t\t[P]rogram to Program byte to a different Sector\n");
			default:  /* Must be 29F080 or 29F016 */
				break;
		}
		printf("\t\t[R]ead to Read from a different Sector\n");
		printf("\t\t[Q]uit to quit Erase suspend and resume Sector Erase\n");		
		printf("Please enter Letter of needed selection: ");
		selection_L = _getche();
		printf("\n");												

		switch(selection_L) {

			/* Read */
			case 'r':
			case 'R':
				printf("\nReading %s", this_dev_GSP->devname_GS);
				printf("\nEnter hexadecimal starting address: ");
				scanf("%Lx", &selection2_L);
				printf("\n");

				switch ( deviceType ) {
		
					case Am29F040:
							/* Make sure address is within reasonable bounds */	
							if ( (selection2_L < 0) || (selection2_L >= addr_29F040_GA[8]) ) {
								printf("\nError. Address specified exceeds device address space.");
								break;
							}
							/* Make sure address does not fall within sector currently being erased */
							if ( (selection2_L >= addr_29F040_GA[sect_num_L]) &&
								 (selection2_L < addr_29F040_GA[sect_num_L + 1]) ) 
							{
								printf("\nError. Address specified falls in the sector currently being erased.");
								break;
							}
							break;
					case Am29F100T:
							/* Make sure address is within reasonable bounds */	
							if ( (selection2_L < 0) || (selection2_L >= addr_29F100T_GA[5]) ) {
								printf("\nError. Address specified exceeds device address space.");
								break;
							}
							/* Make sure address does not fall within sector currently being erased */
							if ( (selection2_L >= addr_29F100T_GA[sect_num_L]) &&
								 (selection2_L < addr_29F100T_GA[sect_num_L + 1]) ) 
							{
								printf("\nError. Address specified falls in the sector currently being erased.");
								break;
							}
							break;
					case Am29F100B:
							/* Make sure address is within reasonable bounds */	
							if ( (selection2_L < 0) || (selection2_L >= addr_29F100B_GA[5]) ) {
								printf("\nError. Address specified exceeds device address space.");
								break;
							}
							/* Make sure address does not fall within sector currently being erased */
							if ( (selection2_L >= addr_29F100B_GA[sect_num_L]) &&
								 (selection2_L < addr_29F100B_GA[sect_num_L + 1]) ) 
							{
								printf("\nError. Address specified falls in the sector currently being erased.");
								break;
							}
							break;
					case Am29F200T:
							/* Make sure address is within reasonable bounds */	
							if ( (selection2_L < 0) || (selection2_L >= addr_29F200T_GA[7]) ) {
								printf("\nError. Address specified exceeds device address space.");
								break;
							}
							/* Make sure address does not fall within sector currently being erased */
							if ( (selection2_L >= addr_29F200T_GA[sect_num_L]) &&
								 (selection2_L < addr_29F200T_GA[sect_num_L + 1]) ) 
							{
								printf("\nError. Address specified falls in the sector currently being erased.");
								break;
							}
							break;
					case Am29F200B:
							/* Make sure address is within reasonable bounds */	
							if ( (selection2_L < 0) || (selection2_L >= addr_29F200B_GA[7]) ) {
								printf("\nError. Address specified exceeds device address space.");
								break;
							}
							/* Make sure address does not fall within sector currently being erased */
							if ( (selection2_L >= addr_29F200B_GA[sect_num_L]) &&
								 (selection2_L < addr_29F200B_GA[sect_num_L + 1]) ) 
							{
								printf("\nError. Address specified falls in the sector currently being erased.");
								break;
							}
							break;
					case Am29F400T:
							/* Make sure address is within reasonable bounds */	
							if ( (selection2_L < 0) || (selection2_L >= addr_29F400T_GA[11]) ) {
								printf("\nError. Address specified exceeds device address space.");
								break;
							}
							/* Make sure address does not fall within sector currently being erased */
							if ( (selection2_L >= addr_29F400T_GA[sect_num_L]) &&
								 (selection2_L < addr_29F400T_GA[sect_num_L + 1]) ) 
							{
								printf("\nError. Address specified falls in the sector currently being erased.");
								break;
							}
							break;
					case Am29F400B:
							/* Make sure address is within reasonable bounds */	
							if ( (selection2_L < 0) || (selection2_L >= addr_29F400B_GA[11]) ) {
								printf("\nError. Address specified exceeds device address space.");
								break;
							}
							/* Make sure address does not fall within sector currently being erased */
							if ( (selection2_L >= addr_29F400B_GA[sect_num_L]) &&
								 (selection2_L < addr_29F400B_GA[sect_num_L + 1]) ) 
							{
								printf("\nError. Address specified falls in the sector currently being erased.");
								break;
							}
							break;
					case Am29F080:
							/* Make sure address is within reasonable bounds */	
							if ( (selection2_L < 0) || (selection2_L >= addr_29F080_GA[16]) ) {
								printf("\nError. Address specified exceeds device address space.");
								break;
							}
							/* Make sure address does not fall within sector currently being erased */
							if ( (selection2_L >= addr_29F080_GA[sect_num_L]) &&
								 (selection2_L < addr_29F080_GA[sect_num_L + 1]) ) 
							{
								printf("\nError. Address specified falls in the sector currently being erased.");
								break;
							}
							break;
					case Am29F016:
							/* Make sure address is within reasonable bounds */	
							if ( (selection2_L < 0) || (selection2_L >= addr_29F016_GA[32]) ) {
								printf("\nError. Address specified exceeds device address space.");
								break;
							}
							/* Make sure address does not fall within sector currently being erased */
							if ( (selection2_L >= addr_29F016_GA[sect_num_L]) &&
								 (selection2_L < addr_29F016_GA[sect_num_L + 1]) ) 
							{
								printf("\nError. Address specified falls in the sector currently being erased.");
								break;
							}
							break;
					default:
							flerror_code_G = NOT_FOR_THIS_DEVICE;
							return(flerror_code_G);
				}

				_ultoa(selection2_L, string_GA, 16);
				printf("\nA: %s  D: ", string_GA);
				split_addr(selection2_L);
				switch ( deviceType ) {
    	
					case Am29F010:
					case Am29F040:
					case Am29F080:
					case Am29F016: /* x8 mode */
						/* read the data */
						/* data1_L = _inp(base_decode_G+(curr_socket_G+1));*/
						break;

					default: /* x16 mode */
						/* read the data */
						/* data1_L = _inpw(base_decode_G+(curr_socket_G+1));*/
						break;

				}
				printf("%2.2x ", data1_L);
				/* Do not print unprintable characters, like Backspace, etc */
				if (data1_L >= 0x07 && data1_L <= 0x0d) 
					data1_L = 0x2e;		//if it's a beep, etc, don't print it
				printf("ASCII:  %c", data1_L);				

				break;

			/* Program */
			case 'p':
			case 'P':
				switch ( deviceType ) {
					case Am29F080:
						printf("\nProgramming %s", this_dev_GSP->devname_GS);
						printf("\nEnter hexadecimal starting address: ");
						scanf("%Lx", &selection2_L);
						printf("\n");
						/* Make sure address is within reasonable bounds */	
						if ( (selection2_L < 0) || (selection2_L >= addr_29F080_GA[16]) ) {
							printf("\nError. Address specified exceeds device address space.");
							break;
						}
						/* Make sure address does not fall within sector currently being erased */
						if ( (selection2_L >= addr_29F080_GA[sect_num_L]) &&
							 (selection2_L < addr_29F080_GA[sect_num_L + 1]) ) 
						{
							printf("\nError. Address specified falls in the sector currently being erased.");
							break;
						}
					case Am29F016:
						printf("\nProgramming %s", this_dev_GSP->devname_GS);
						printf("\nEnter hexadecimal starting address: ");
						scanf("%Lx", &selection2_L);
						printf("\n");
						/* Make sure address is within reasonable bounds */	
						if ( (selection2_L < 0) || (selection2_L >= addr_29F016_GA[32]) ) {
							printf("\nError. Address specified exceeds device address space.");
							break;
						}
						/* Make sure address does not fall within sector currently being erased */
						if ( (selection2_L >= addr_29F016_GA[sect_num_L]) &&
							 (selection2_L < addr_29F016_GA[sect_num_L + 1]) ) 
						{
							printf("\nError. Address specified falls in the sector currently being erased.");
							break;
						}

						/* input the byte to program */
						printf("\nEnter decimal byte to program: ");
			    		scanf("%d", &iByteToProgram);
			    		if ( (iByteToProgram < 0) || (iByteToProgram <= 0xFF) ) {
							printf("\nError. Data specified is incorrect.");
							break;
			    		}
			    		
			    		/* Program the byte */
						flerror_code_G = emb_program(selection2_L, iByteToProgram);
			    		if ( flerror_code_G ) {
							return( flerror_code_G );			    			
			    		}
						break;

					default:  /* Must be 29F040 or 29FX00 */
						printf("\nIncorrect choice, please select again.");
				}
				break;

			/* Return to the previous menu */
			case 'q':
			case 'Q':
				/* Resume the Sector Erase in progress */
				switch ( deviceType ) {

					case Am29F010:
					case Am29F040:
					case Am29F080:
					case Am29F016:
						/* Send data in x8 mode */
						/* _outp(base_decode_G+(curr_socket_G+1), 0x30);*/
						break;

					default:
						/* Send data in x16 mode */
						/* _outpw(base_decode_G+(curr_socket_G+1), 0x0030);*/
						break;

				}
				bValidInput = TRUE;
				break;

			default :
				printf("\nIncorrect choice, please select again.");
				break;

		} /* switch ( selection_L ) */

	} while ( ! bValidInput );

	return( flerror_code_G );
	
} /* erase_suspend */


/*******************************************************************/
/*	sector_verify()                                                */
/*                                                                 */
/*	purpose:	Checks 5v 29F devices for protected sectors, uses  */
/*				12v on A9.                                         */
/*                                                                 */
/*******************************************************************/
/*	parameters:	none                                               */
/*******************************************************************/
/*  return value:   flerror_code = 0 : successful                  */
/*                               = anything : unsuccessful         */
/*******************************************************************/
int sector_verify( void )
{
	int i_L;
	int in_data_L;
	int deviceType = this_dev_GSP->device_no_GS;
	
	flerror_code_G = 0;
	
	/* Apply Vpph to A9 here */

	for ( i_L=0; i_L < this_dev_GSP->num_sectors; i_L++ ) {

		/* get the correct address */
		switch ( deviceType ) {

			case Am29F010:
				address_G = addr_29F010_GA[i_L] | 0x00000002L;
				break;
			case Am29F040:
				address_G = addr_29F040_GA[i_L] | 0x00000002L;
				break;
			case Am29F100T:
				address_G = addr_29F100T_GA[i_L] | 0x00000002L;
				break;
			case Am29F100B:
				address_G = addr_29F100B_GA[i_L] | 0x00000002L;
				break;
			case Am29F200T:
				address_G = addr_29F200T_GA[i_L] | 0x00000002L;
				break;
			case Am29F200B:
				address_G = addr_29F200B_GA[i_L] | 0x00000002L;
				break;
			case Am29F400T:
				address_G = addr_29F400T_GA[i_L] | 0x00000002L;
				break;
			case Am29F400B:
				address_G = addr_29F400B_GA[i_L] | 0x00000002L;
				break;
			default:
				flerror_code_G = NOT_FOR_THIS_DEVICE;
				return ( flerror_code_G );
		}

		/* send the address */
		split_addr(address_G);

		switch ( deviceType ) {        		
			case Am29F010:
			case Am29F040:
				in_data_L = _inp(base_decode_G+(curr_socket_G+1));
				break;
			default:
				in_data_L = _inpw(base_decode_G+(curr_socket_G+1));
				break;
		}			

		if (in_data_L == 0x01) {
			/* sector is protected */
			prot_sects_GA[i_L] = 1;			 
		}
		else {
			/* sector is unprotected */
			prot_sects_GA[i_L] = 0;
		}

	} /* for all sectors */

	/* return A9 to normal TTL levels here */

	/* reset the address */
	address_G = 0;

	return( flerror_code_G );

} /* sector_verify */


/*******************************************************************/
/*	no_dq_poll()                                                   */
/*                                                                 */
/*	purpose:	This is a quick-and-dirty check routine, rather    */
/*              than performing the full DQ6 or DQ7 polling. This  */
/*              routine reads data until it is the same as what was*/
/*              programmed or erased, and also checks for DQ5=1, in*/
/*              case a timeout occurred. It may or may not be      */
/*              faster than dq6_poll() or dq7_poll(). Error        */
/*				recovery is minimal, that is, the user is warned   */
/*				of the error, but the calling function is          */
/*				responsible for exiting gracefully.                */
/*				                                                   */
/*******************************************************************/
/*	parameters:	int op_type = 0, use no_dq_poll() in a program op. */
/*                            (FL_PROG)                            */
/*                          = 1, use no_dq_poll() in a chip erase op.*/
/*                            (FL_CH_ERASE)                        */
/*                          = 2, use no_dq_poll() in a sector erase*/
/*                            (FL_SC_ERASE) operation              */
/*******************************************************************/
/*  return value:   flerror_code = 0 : successful                  */
/*                               = anything : unsuccessful         */
/*******************************************************************/    
int no_dq_poll(int op_type_L)
{
	unsigned int in_data_L;
	int op_done_L  = 0;
	int deviceType = this_dev_GSP->device_no_GS;

	if ((op_type_L == FL_CH_ERASE) || (op_type_L == FL_SC_ERASE)) {
		if ((deviceType == Am29F010) || (deviceType == Am29F040)) {
			/* must use an address that is not protected when doing any polling */
			split_addr(poll_addr_G);	/* address from emb_erase() */ 
			curr_data_G = 0xFF;
		}
	}
		          
	do {

		in_data_L = _inp(base_decode_G+(curr_socket_G+1));
       	if (in_data_L != curr_data_G) {
       		if ((in_data_L & 0x20) == 0x20)	{ //watch for DQ5
				in_data_L = _inp(base_decode_G+(curr_socket_G+1));
	        	if (in_data_L != curr_data_G) { //check again after DQ5 = 1
					if ((deviceType == Am29F010) || (deviceType == Am29F040)) {
						reset_fl(FL_29F);
					}
					else {
						reset_fl(FL_28F);
					}
					flerror_code_G = DQ5_ON_NO_POLL;
					this_address_G = address_G;
					return(flerror_code_G);
				}
				else { //curr_data_G == in_data_L after DQ5 =1
					return(0);
				}
			}
			op_done_L = 0;	//not finished
		}
		else { //curr_data_G == in_data_L
			return(0);
		}

	} while (!op_done_L);

	return(0);				

} /* no_dq_poll */


/*******************************************************************/
/*	dq6_poll()                                                     */
/*                                                                 */
/*	purpose:	Performs Data polling using DQ6 Toggle bit and DQ5 */
/*				algorithm for program and erase operations. Error  */
/*				recovery is minimal, that is, the user is warned   */
/*				of the error, but the calling function is          */
/*				responsible for exiting gracefully.                */
/*				                                                   */
/*******************************************************************/
/*	parameters:	int op_type = 0, use dq6_poll() in a program op.   */
/*                            (FL_PROG)                            */
/*                          = 1, use dq6_poll() in a chip erase op.*/
/*                            (FL_CH_ERASE)                        */
/*                          = 2, use dq6_poll() in a sector erase  */
/*                            (FL_SC_ERASE) operation              */
/*******************************************************************/
/*  return value:   flerror_code = 0 : successful                  */
/*                               = anything : unsuccessful         */
/*******************************************************************/    
int dq6_poll(int op_type_L)				
{
	unsigned int in_data_L;
	unsigned int temp_L;
	unsigned int temp2_L;
	int prog_done_L    = 0;
	int erase_done_L   = 0;
	int erase_failed_L = 0;
	int deviceType     = this_dev_GSP->device_no_GS;

	flerror_code_G = 0;

	/* program operation */
	if (op_type_L == FL_PROG) {

		do {

			/* read 1st data */
			switch ( deviceType ) {
				case Am29F100T:
				case Am29F100B:
				case Am29F200T:
				case Am29F200B:
				case Am29F400T:
				case Am29F400B:
					in_data_L = _inpw(base_decode_G+(curr_socket_G+1));
					break;
				case Am29F010:
				case Am29F040:
				case Am29F080:
				case Am29F016:
				default :	
					in_data_L = _inp(base_decode_G+(curr_socket_G+1));
					break;
			}
			temp2_L = in_data_L;

			/* read 2nd data */
			switch ( deviceType ) {
				case Am29F100T:
				case Am29F100B:
				case Am29F200T:
				case Am29F200B:
				case Am29F400T:
				case Am29F400B:
					in_data_L = _inpw(base_decode_G+(curr_socket_G+1));
					break;
				case Am29F010:
				case Am29F040:
				case Am29F080:
				case Am29F016:
				default :	
					in_data_L = _inp(base_decode_G+(curr_socket_G+1));
					break;
			}
			temp_L = in_data_L;

			if ((temp_L & 0x40) == (temp2_L & 0x40)) {

				/* if they are the same, then DQ6 has stopped toggling */
				/* read again for final data */
				switch ( deviceType ) {
					case Am29F100T:
					case Am29F100B:
					case Am29F200T:
					case Am29F200B:
					case Am29F400T:
					case Am29F400B:
						in_data_L = _inpw(base_decode_G+(curr_socket_G+1));
						break;
					case Am29F010:
					case Am29F040:
					case Am29F080:
					case Am29F016:
					default :	
						in_data_L = _inp(base_decode_G+(curr_socket_G+1));
						break;
				}

				if (curr_data_G != in_data_L) {
					/* if original data != in_data_L, error */
					switch ( deviceType ) {
	
						case Am29F010:
						case Am29F040:
						case Am29F100T:
						case Am29F100B:
						case Am29F200T:
						case Am29F200B:
						case Am29F400T:
						case Am29F400B:
						case Am29F080:
						case Am29F016:
							reset_fl(FL_29F);
							break;
						default :	
							reset_fl(FL_28F);
	
					}

					flerror_code_G = PROGRAMMING_FAILURE;						
					errdata_G      = in_data_L;
					this_address_G = address_G;

				}
				return( flerror_code_G );
			}
			else {

				/* if they are not the same, DQ6 is still toggling */
				temp_L = in_data_L;
				if ((temp_L & 0x20) == 0x20) {

					/* DQ5 = 1; read again because maybe DQ6 changed 
					simultaneously with DQ5 */
					switch ( deviceType ) {
						case Am29F100T:
						case Am29F100B:
						case Am29F200T:
						case Am29F200B:
						case Am29F400T:
						case Am29F400B:
							in_data_L = _inpw(base_decode_G+(curr_socket_G+1));
							break;
						case Am29F010:
						case Am29F040:
						case Am29F080:
						case Am29F016:
						default :	
							in_data_L = _inp(base_decode_G+(curr_socket_G+1));
							break;
					}

					if ((in_data_L & 0x40) == (temp_L & 0x40)) {
						/* DQ6 has stopped toggling */

						/* read final data */
						switch ( deviceType ) {
							case Am29F100T:
							case Am29F100B:
							case Am29F200T:
							case Am29F200B:
							case Am29F400T:
							case Am29F400B:
								in_data_L = _inpw(base_decode_G+(curr_socket_G+1));
								break;
							case Am29F010:
							case Am29F040:
							case Am29F080:
							case Am29F016:
							default :	
								in_data_L = _inp(base_decode_G+(curr_socket_G+1));
								break;
						}

						if (in_data_L != curr_data_G) {

							switch ( deviceType ) {
			
								case Am29F010:
								case Am29F040:
								case Am29F100T:
								case Am29F100B:
								case Am29F200T:
								case Am29F200B:
								case Am29F400T:
								case Am29F400B:
								case Am29F080:
								case Am29F016:
									reset_fl(FL_29F);
									break;
								default :	
									reset_fl(FL_28F);
			
							}

							flerror_code_G = DQ6_PROGRAM_FAILURE;
							errdata_G      = in_data_L;
							this_address_G = address_G;

						}

						return( flerror_code_G );

					}
					else {
						/* DQ6 has not stopped toggling, even after DQ5 = 1 */
						switch ( deviceType ) {
			
							case Am29F010:
							case Am29F040:
							case Am29F100T:
							case Am29F100B:
							case Am29F200T:
							case Am29F200B:
							case Am29F400T:
							case Am29F400B:
							case Am29F080:
							case Am29F016:
								reset_fl(FL_29F);
								break;
							default :	
								reset_fl(FL_28F);
			
						}
						flerror_code_G = DQ6_KEEPS_TOGGLING;
						this_address_G = address_G;
						return( flerror_code_G );
					}
				} /* DQ5 = '1' */
			} /* DQ6 still toggling */
		} while (!prog_done_L);

	} /* FL_PROG */
	else if (op_type_L == FL_CH_ERASE || op_type_L == FL_SC_ERASE) {

		/* chip & sector erase operation */
		do {

			switch ( deviceType ) {
				case Am29F010:
				case Am29F040:
				case Am29F100T:
				case Am29F100B:
				case Am29F200T:
				case Am29F200B:
				case Am29F400T:
				case Am29F400B:
				case Am29F080:
				case Am29F016:
					/* must use an address that is not protected when doing any polling */
					split_addr(poll_addr_G);	/* address from emb_erase() */
					break;
				default :
					;	
			}

			/* read 1st data */
			switch ( deviceType ) {
				case Am29F100T:
				case Am29F100B:
				case Am29F200T:
				case Am29F200B:
				case Am29F400T:
				case Am29F400B:
					in_data_L = _inpw(base_decode_G+(curr_socket_G+1));
					break;
				case Am29F010:
				case Am29F040:
				case Am29F080:
				case Am29F016:
				default :	
					in_data_L = _inp(base_decode_G+(curr_socket_G+1));
					break;
			}
			temp2_L = in_data_L;

			/* read 2nd data */
			switch ( deviceType ) {
				case Am29F100T:
				case Am29F100B:
				case Am29F200T:
				case Am29F200B:
				case Am29F400T:
				case Am29F400B:
					in_data_L = _inpw(base_decode_G+(curr_socket_G+1));
					break;
				case Am29F010:
				case Am29F040:
				case Am29F080:
				case Am29F016:
				default :	
					in_data_L = _inp(base_decode_G+(curr_socket_G+1));
					break;
			}
			temp_L = in_data_L;

			if ((temp_L & 0x40) == (temp2_L & 0x40)) {
				/* if they are the same, then DQ6 has stopped toggling */

				/* read again for final data */
				switch ( deviceType ) {
					case Am29F100T:
					case Am29F100B:
					case Am29F200T:
					case Am29F200B:
					case Am29F400T:
					case Am29F400B:
						in_data_L = _inpw(base_decode_G+(curr_socket_G+1));
						if (in_data_L == 0xFFFF) {
							erase_failed_L = 0;
						}
						else {
							erase_failed_L = 1;
						}
						break;
					case Am29F010:
					case Am29F040:
					case Am29F080:
					case Am29F016:
					default :	
						in_data_L = _inp(base_decode_G+(curr_socket_G+1));
						if (in_data_L == 0xFF) {
							erase_failed_L = 0;
						}
						else {
							erase_failed_L = 1;
						}
						break;
				}

				if ( erase_failed_L ) {

					/* erase failed because FFh not found */
					switch ( deviceType ) {
			
						case Am29F010:
						case Am29F040:
						case Am29F100T:
						case Am29F100B:
						case Am29F200T:
						case Am29F200B:
						case Am29F400T:
						case Am29F400B:
						case Am29F080:
						case Am29F016:
							reset_fl(FL_29F);
							break;
						default :	
							reset_fl(FL_28F);
			
					}
					flerror_code_G = DQ6_ERASE_FAILURE_DATA;
					errdata_G      = in_data_L;

				}

				return( flerror_code_G );

			}
			else {

				/* if they are not the same, DQ6 is still toggling */
				temp_L = in_data_L;

				if ((temp_L & 0x20) == 0x20) {

					/* DQ5 = 1; read again because maybe DQ6 changed
					simultaneously with DQ5 */
					switch ( deviceType ) {
						case Am29F100T:
						case Am29F100B:
						case Am29F200T:
						case Am29F200B:
						case Am29F400T:
						case Am29F400B:
							in_data_L = _inpw(base_decode_G+(curr_socket_G+1));
							break;
						case Am29F010:
						case Am29F040:
						case Am29F080:
						case Am29F016:
						default :	
							in_data_L = _inp(base_decode_G+(curr_socket_G+1));
							break;
					}

					if ((in_data_L & 0x40) == (temp_L & 0x40)) {

						/* DQ6 has stopped toggling */
						switch ( deviceType ) {
							case Am29F100T:
							case Am29F100B:
							case Am29F200T:
							case Am29F200B:
							case Am29F400T:
							case Am29F400B:
								in_data_L = _inpw(base_decode_G+(curr_socket_G+1));
								if (in_data_L == 0xFFFF) {
									erase_failed_L = 0;
								}
								else {
									erase_failed_L = 1;
								}
								break;
							case Am29F010:
							case Am29F040:
							case Am29F080:
							case Am29F016:
							default :	
								in_data_L = _inp(base_decode_G+(curr_socket_G+1));
								if (in_data_L == 0xFF) {
									erase_failed_L = 0;
								}
								else {
									erase_failed_L = 1;
								}								break;
						}

						/* check for 0xFF */
						if ( erase_failed_L ) {
							switch ( deviceType ) {
					
								case Am29F010:
								case Am29F040:
								case Am29F100T:
								case Am29F100B:
								case Am29F200T:
								case Am29F200B:
								case Am29F400T:
								case Am29F400B:
								case Am29F080:
								case Am29F016:
									reset_fl(FL_29F);
									break;
								default :	
									reset_fl(FL_28F);
					
							}

							errdata_G=in_data_L;
							flerror_code_G = DQ6_ERASE_FAILURE_DQ5;

						}
						return( flerror_code_G );
					}
					else {
						/* DQ6 has not stopped toggling, even after DQ5 = 1 */

						if (op_type_L == FL_CH_ERASE) {	
							/* chip erase */
							switch ( deviceType ) {
					
								case Am29F010:
								case Am29F040:
								case Am29F100T:
								case Am29F100B:
								case Am29F200T:
								case Am29F200B:
								case Am29F400T:
								case Am29F400B:
								case Am29F080:
								case Am29F016:
									reset_fl(FL_29F);
									flerror_code_G = DQ6_ERASE_ON_29F;								
									break;
								default :	
									reset_fl(FL_28F);
									flerror_code_G = DQ6_ERASE_ON_28F;								
					
							}
							return( flerror_code_G );
						}
						else {
							/* sector erase */
							flerror_code_G = DQ6_ERASE_ON_29F;
							reset_fl( FL_29F );
							return( flerror_code_G );
						}
					} /* DQ6 still toggling */
				} /* DQ5 = '1' */
			} /* DQ6 is still toggling */
		} while (!erase_done_L);
		/* end of DQ6 Polling */
	} /* FL_CH_ERASE || FL_SC_ERASE */

} /* dq6_poll */


/*******************************************************************/
/*	dq7_poll()                                                     */
/*                                                                 */
/*	purpose:	Performs Data polling using DQ7 Toggle bit and DQ5 */
/*				algorithm for program and erase operations. Error  */
/*				recovery is minimal, that is, the user is warned   */
/*				of the error, but the calling function is          */
/*				responsible for exiting gracefully.                */
/*				                                                   */
/*******************************************************************/
/*	parameters:	int op_type = 0, use dq7_poll() in a program op.   */
/*                            (FL_PROG)                            */
/*                          = 1, use dq7_poll() in a chip erase op.*/
/*                            (FL_CH_ERASE)                        */
/*                          = 2, use dq7_poll() in a sector erase  */
/*                            (FL_SC_ERASE) operation              */
/*******************************************************************/
/*  return value:   flerror_code = 0 : successful                  */
/*                               = anything : unsuccessful         */
/*******************************************************************/    
int dq7_poll(int op_type_L)
{

	unsigned int in_data_L    = 0;
	unsigned int temp_L       = 0;
	unsigned int temp2_L      = 0;
	int	erase_failed_L		  = 0;
	int deviceType            = this_dev_GSP->device_no_GS;

	flerror_code_G = 0;

	/* program operation */
	switch ( op_type_L ) {

		case FL_PROG : /* Called from emb_prog */

			for ( ; ; ) {

				/* read data */
				switch ( deviceType ) {
					case Am29F100T:
					case Am29F100B:
					case Am29F200T:
					case Am29F200B:
					case Am29F400T:
					case Am29F400B:
						in_data_L = _inpw(base_decode_G+(curr_socket_G+1));
						break;
					case Am29F010:
					case Am29F040:
					case Am29F080:
					case Am29F016:
					default :	
						in_data_L = _inp(base_decode_G+(curr_socket_G+1));
						break;
				}
				temp2_L = curr_data_G;
				temp_L  = in_data_L;
	
				/* Check the DQ7 bit for DATA POLL */
/* Errata: corrected 10.95 */
				if ((temp2_L & 0x80) == (temp_L & 0x80)) {
	
					/* DQ7 = true data bit (programmed correctly) */
						/* read again for final data */
					switch ( deviceType ) {
						case Am29F100T:
						case Am29F100B:
						case Am29F200T:
						case Am29F200B:
						case Am29F400T:
						case Am29F400B:
							in_data_L = _inpw(base_decode_G+(curr_socket_G+1));
							break;
						case Am29F010:
						case Am29F040:
						case Am29F080:
						case Am29F016:
						default :	
							in_data_L = _inp(base_decode_G+(curr_socket_G+1));
							break;
					}

					if ( curr_data_G != in_data_L ) {
						/* if original data != in_data_L, even after DQ7 = true
							data, error */
						switch ( deviceType ) {
	
							case Am29F010:
							case Am29F040:
							case Am29F100T:
							case Am29F100B:
							case Am29F200T:
							case Am29F200B:
							case Am29F400T:
							case Am29F400B:
							case Am29F080:
							case Am29F016:
								reset_fl(FL_29F);
								break;
							default :	
								reset_fl(FL_28F);
	
						}
	
						errdata_G      = in_data_L;
						this_address_G = address_G;
						flerror_code_G = DQ7_PROGRAM_FAILURE;
						return( flerror_code_G );
					}
					else {
						/* Embedded program worked properly as per DATA POLL */
						return( flerror_code_G );
					}
	
				} /* DQ7 = true data bit (programmed correctly) */
	
				/* DQ7 still toggling */
				else {                   	
	
					/* DQ7 = complement of true data (not finished programming yet) */
					temp_L = in_data_L;
					if ((temp_L & 0x20) == 0x20) {
	
						/* DQ5 = 1; read again because maybe DQ7 changed 
						simultaneously with DQ5 */
						switch ( deviceType ) {
							case Am29F100T:
							case Am29F100B:
							case Am29F200T:
							case Am29F200B:
							case Am29F400T:
							case Am29F400B:
								in_data_L = _inpw(base_decode_G+(curr_socket_G+1));
								break;
							case Am29F010:
							case Am29F040:
							case Am29F080:
							case Am29F016:
							default :	
								in_data_L = _inp(base_decode_G+(curr_socket_G+1));
								break;
						}
	
						/* Compare DQ7 with the original data */
/* Errata: corrected 10.95 */						
						if ( (temp2_L & 0x80) == (in_data_L & 0x80) ) {
							/* We're done, the device passed the DATA POLL algorithm */
							return( flerror_code_G );
						}
						else {
							/* DQ7 is complement of true data, even after DQ5 = 1 */
							switch ( deviceType ) {
	
								case Am29F010:
								case Am29F040:
								case Am29F100T:
								case Am29F100B:
								case Am29F200T:
								case Am29F200B:
								case Am29F400T:
								case Am29F400B:
								case Am29F080:
								case Am29F016:
									reset_fl(FL_29F);
									break;
								default :	
									reset_fl(FL_28F);
	
							}
	
							flerror_code_G = DQ7_FAIL_AFTER_DQ5_2;
							this_address_G = address_G;
							return( flerror_code_G );
	
						} /* DQ7 is complement of true data, even after DQ5 = 1 */
					} /* DQ5 = '1' */
				} /* DQ7 failed, try DQ5 */

			} /* for */
			break;

		case FL_CH_ERASE:
		case FL_SC_ERASE:
		default :

			/* chip & sector erase operation */
			for ( ; ; ) {

				/* must use an address that is not protected when doing any polling */
				switch ( deviceType ) {
					case Am29F010:
					case Am29F040:
					case Am29F100T:
					case Am29F100B:
					case Am29F200T:
					case Am29F200B:
					case Am29F400T:
					case Am29F400B:
					case Am29F080:
					case Am29F016:
						split_addr(poll_addr_G);	/* address from emb_erase() */
						break;
					default :
						;	
				}

				/* read data */
				switch ( deviceType ) {
					case Am29F100T:
					case Am29F100B:
					case Am29F200T:
					case Am29F200B:
					case Am29F400T:
					case Am29F400B:
						in_data_L = _inpw(base_decode_G+(curr_socket_G+1));
						break;
					case Am29F010:
					case Am29F040:
					case Am29F080:
					case Am29F016:
					default :	
						in_data_L = _inp(base_decode_G+(curr_socket_G+1));
						break;
				}
				temp_L = in_data_L;				

				if ((temp_L & 0x80) == 0x80) {	

					/* if they are the same, then DQ7 = true data */
					switch ( deviceType ) {
						case Am29F100T:
						case Am29F100B:
						case Am29F200T:
						case Am29F200B:
						case Am29F400T:
						case Am29F400B:
							in_data_L = _inpw(base_decode_G+(curr_socket_G+1));
							if (in_data_L == 0xFFFF) {
								erase_failed_L = 0;
							}
							else {
								erase_failed_L = 1;
							}
							break;
						case Am29F010:
						case Am29F040:
						case Am29F080:
						case Am29F016:
						default :	
							in_data_L = _inp(base_decode_G+(curr_socket_G+1));
							if (in_data_L == 0xFF) {
								erase_failed_L = 0;
							}
							else {
								erase_failed_L = 1;
							}
							break;
					}

					if ( erase_failed_L ) {
						/* erase failed because	FFh not found */
						switch ( deviceType ) {
	
							case Am29F010:
							case Am29F040:
							case Am29F100T:
							case Am29F100B:
							case Am29F200T:
							case Am29F200B:
							case Am29F400T:
							case Am29F400B:
							case Am29F080:
							case Am29F016:
								reset_fl(FL_29F);
								break;
							default :	
								reset_fl(FL_28F);

						}

						flerror_code_G = DQ7_ERASE_FAILURE;						
						errdata_G      = in_data_L;
						return( flerror_code_G );

					}
					else {
						/* We're done, DATA POLL algorithm returned success */
						return( flerror_code_G );
					}
				} /* DQ7 = '1' */
				else {

					if ((in_data_L & 0x20) == 0x20) {
						/* DQ5 = 1; read again because maybe DQ6 changed
							simultaneously with DQ5 */
						switch ( deviceType ) {
							case Am29F100T:
							case Am29F100B:
							case Am29F200T:
							case Am29F200B:
							case Am29F400T:
							case Am29F400B:
								in_data_L = _inpw(base_decode_G+(curr_socket_G+1));
								break;
							case Am29F010:
							case Am29F040:
							case Am29F080:
							case Am29F016:
							default :	
								in_data_L = _inp(base_decode_G+(curr_socket_G+1));
								break;
						}
						temp_L = in_data_L;
/* Errata: corrected 10.95 */						
						if ((temp_L & 0x80) == 0x80) {
							/* DQ& = '1', the device passed the DATA POLL algorithm */
							return( flerror_code_G );
						}
						else {
							/* DQ7 is still not true data, even after DQ5 = 1 */

							if (op_type_L == FL_CH_ERASE) {	
								/* chip erase */
								switch ( deviceType ) {
			
									case Am29F010:
									case Am29F040:
									case Am29F100T:
									case Am29F100B:
									case Am29F200T:
									case Am29F200B:
									case Am29F400T:
									case Am29F400B:
									case Am29F080:
									case Am29F016:
										reset_fl(FL_29F);
										flerror_code_G = DQ7_ERASE_ON_29F;								
										break;
									default :	
										reset_fl(FL_28F);
										flerror_code_G = DQ7_ERASE_ON_28F;								
		
								}
								return(flerror_code_G);
							}
							else {			
								/* sector erase */
								flerror_code_G = DQ7_ERASE_ON_29F;
								reset_fl(FL_29F);
								return(flerror_code_G);
							}
						} /* DQ7 is still not true data, even after DQ5 = 1 */
					} /* DQ5 = '1' */
				} /* DQ7 failed, try DQ5 */

			} /* for */

	} /* switch */

} /* dq7_poll */


/*******************************************************************/
/*	split_addr()                                                   */
/*                                                                 */
/*	purpose:	Uses up to a 32 bit address and splits it into a   */
/*				16 bit chunk with a 8 bit followup (since we only  */
/*				require 19 bits). The 16 bit address always goes   */
/*				to the xx0h PC I/O address. This is the only 16    */
/*				bit transfer over the PC's data bus. The remaining */
/*				8 bits are transferred as 1 byte to I/O.           */
/*                                                                 */
/*******************************************************************/
/*	parameters:	long addr = 32 bit address to split into 1 word    */
/*                          and 1 byte.                            */
/*******************************************************************/
/*  return value: none                                             */
/*******************************************************************/
void split_addr(long addr_L)
{
	int	 low_word_L;
	char high_byte_L;

	low_word_L  = (int)(addr_L & 0x0000FFFFL);	/* get lower 16 bits */
	/* shift remaining address lines */
	high_byte_L = (int)((addr_L & 0x000F0000L) >> 16); 
	/* send the lower 16 address lines */
	/* _outpw(base_decode_G, low_word_L);*/
	
	/* send remaining address lines */
	/* _outp(base_decode_G+1, high_byte_L);*/

} /* split_addr */


/*******************************************************************/
/*	concat_bytes()                                                 */
/*                                                                 */
/*	purpose:	Uses 2 bytes, and returns a concatenated 16 bits   */
/*				of data.           								   */
/*                                                                 */
/*******************************************************************/
/*	parameters:	high_byte = 8 bit data to use for MSByte	       */
/*               low_byte = 8 bit data to use for LSByte      	   */
/*******************************************************************/
/*  return value: none                                             */
/*******************************************************************/
unsigned int concat_bytes( int high_byte, int low_byte )
{
	unsigned int uiRetWord = 0;

	/* Get the MSbyte */
	uiRetWord = (unsigned int)( high_byte & 0x00FF );
	/* shift the data */
	uiRetWord = ( uiRetWord << 8 );
	/* Get the LSByte */
	uiRetWord = ( uiRetWord & ( (unsigned int)( low_byte & 0x00FF ) ) );

	return( uiRetWord );
	
} /* concat_bytes */

/*******************************************************************/
/*	split_bytes()                                                  */
/*                                                                 */
/*	purpose:	Takes in an int (word) and returns the high & low  */
/*				bytes of data.           			   			   */
/*                                                                 */
/*******************************************************************/
/*	parameters:	 inWord     = 16 bits of data 		               */
/*				 highByteP = pointer to 8 bit data (MSByte)	       */
/*               lowByteP  = pointer to 8 bit data (LSByte)        */
/*******************************************************************/
/*  return value: none                                             */
/*******************************************************************/
void split_bytes( int inWord, char *highByteP, char *lowByteP )
{

	int iWord = inWord;
	
	/* Get the LSbyte */
	*lowByteP = (char)( iWord && 0x00FF );
	/* shift the data */
	iWord = ( inWord >> 8 );
	/* Get the MSByte */
	*highByteP = (char)( iWord && 0x00FF );
	
} /* split_bytes */


/*******************************************************************/
/*	autoselect_seq()                                               */
/*                                                                 */
/*	purpose:	Performs the Auto select setup, prior to           */
/*				requesting either the manufacturer or device ID.   */
/*				Note that the currently selected socket is used.   */
/*				For 12v 28F devices, a don't care address is       */
/*				issued (by this time the device already has 12v in */
/*				this mode only). For 5v 29F devices, the device    */
/*				receives the 3 cycles of unlock sequence sent      */
/*				prior to obtaining the manuf. or device ID.        */
/*                                                                 */
/*******************************************************************/
/*	parameters:	int devtype = 1 for 12v devices (FL_28F)           */
/*                          = 2 for 5v devices  (FL_29F)           */
/*******************************************************************/
/*  return value: none                                             */
/*******************************************************************/
void autoselect_seq(int devtype_L)		/* 1 = 12v, 2 = 29F */
{

	if (devtype_L == FL_29F) {
		split_addr(0x00005555);
		/* Send data */
		/* _outp(base_decode_G+(curr_socket_G+1), 0xAA);*/

		split_addr(0x00002AAA);
		/* Send data */
		/* _outp(base_decode_G+(curr_socket_G+1), 0x55);*/
		
		split_addr(0x00005555);
		/* Send data */
		/* _outp(base_decode_G+(curr_socket_G+1), 0x90);*/
	}
	else {
		/* devtype_L = 12v 28F flash */
		split_addr(0x00000000);			/* don't care address */
		/* Send data */
		/* _outp(base_decode_G+(curr_socket_G+1), 0x80);*/
	}

} /* autoselect_seq */


/*******************************************************************/
/*	reset_fl()                                                     */
/*                                                                 */
/*	purpose:	Resets the currently selected device. If it is a   */
/*				12v device, a don't care address is sent followed  */
/*				by two FFh. If it is a 5v device, the cycle is     */
/*				sent.                                              */
/*                                                                 */
/*******************************************************************/
/*	parameters:	int devtype = 1 for 12v devices (FL_28F)           */
/*                          = 2 for 5v devices  (FL_29F)           */
/*******************************************************************/
/*  return value: none                                             */
/*******************************************************************/
void reset_fl(int devtype_L)
{
	if (devtype_L == FL_28F) {
		split_addr(0x00000000);
		/* Send data */
		/* _outp(base_decode_G+(curr_socket_G+1), 0xFF);*/
		/* _outp(base_decode_G+(curr_socket_G+1), 0xFF);*/
	}
	else {
		/*_outp(base_decode_G+(curr_socket_G+1), 0xF0);*/
			;
	}

} /* reset_fl */


/*******************************************************************/
/*	device_code()                                                  */
/*                                                                 */
/*	purpose:	Obtains the Device ID, depending on the device     */
/*              requested. If the device is a 12v device, A9 is    */
/*              ramped up to 12v. Then, autoselect_seq() is called */
/*				to send out the appropriate command sequence. This */
/*				is followed by address 00001h. On receipt of this  */
/*				ID, A9 is ramped down to normal levels for 12v     */
/*              devices.                                           */
/*                                                                 */
/*******************************************************************/
/*	parameters:	int devtype = 1 for 12v devices (FL_28F)           */
/*                          = 2 for 5v devices  (FL_29F)           */
/*******************************************************************/
/*  return value: none                                             */
/*******************************************************************/
void device_code(int devtype_L)
{
	if (devtype_L == FL_28F) {
		/* Apply Vpph to A9 here */
			;
	}

	autoselect_seq(devtype_L);
	split_addr(0x00000001);
	/* Obtain the device code */
	d_code_G = _inp(base_decode_G+(curr_socket_G+1));

	if (devtype_L == FL_28F) {
		/* return A9 to TTL levels here */
		reset_fl(FL_28F);	/* must reset, otherwise it stays in auto mode */
	}
	else {
		reset_fl(FL_29F);
	}

} /* device_code */


/*******************************************************************/
/*	manuf_code()                                                   */
/*                                                                 */
/*	purpose:	Obtains the Manufacturers's ID, depending on the   */
/*              device requested. If the device is a 12v device,   */
/*              A9 is ramped up to 12v. Then, autoselect_seq() is  */
/*              called to send out the appropriate command         */
/*              sequence. This is followed by address 00000h. On   */
/*              receipt of this ID, A9 is ramped down to normal    */
/*              levels for 12v devices.                            */
/*                                                                 */
/*******************************************************************/
/*	parameters:	int devtype = 1 for 12v devices (FL_28F)           */
/*                          = 2 for 5v devices  (FL_29F)           */
/*******************************************************************/
/*  return value: none                                             */
/*******************************************************************/
void manuf_code(int devtype_L)			/* devtype_L =1 12v, =2 5v */
{

	if (devtype_L == FL_28F) {
		/* Apply Vpph to A9 here */
			;
	}

	autoselect_seq(devtype_L);
	split_addr(0x00000000);
	/* Obtain the manufacturer code */
	m_code_G = _inp(base_decode_G+(curr_socket_G+1));

	if (devtype_L == FL_28F) {
		/* return A9 to TTL levels here */
		reset_fl(FL_28F);	/* must reset, otherwise is stays in auto mode */
	}
	else {
		reset_fl(FL_29F);
	}

} /* manuf_code */


/*******************************************************************/
/*	main()                                                         */
/*                                                                 */
/*	purpose:	Entry point of Flash evaluation kit program source */
/*              code. This initializes the hardware and program.   */
/*				This would be called by the menu shell.            */
/*                                                                 */
/*******************************************************************/
/*	parameters:	none                                               */
/*******************************************************************/
/*  return value: none                                             */
/*******************************************************************/
void main()
{

	int i_L, end_prog_L=0, err_val_L=0;
	char selection_L, selection2_L;
	
	address_G = 0L;
	split_addr(address_G);				/* set address to known state */

	curr_socket_G=0;
	for (i_L=0; i_L<8; i_L++) {
		erase_sects_GA[i_L] = 2;
		prot_sects_GA[i_L]=2;				/* initialize to 2 */
	}
	use_dq7_G=1;
	no_poll_G = 0;
	
	printf("\n\n\n\t\t\t AMD Flash Evaluation Kit Programmer\n");
	printf("\n\n\t\t\t Copyright AMD 1993\n\n\n");

	if (err_val_L != 0) {
		error_handler(err_val_L);
	}

	/* display the menu */
	do {
		printf("\n\t\t\tMain Menu\n");
		printf("\t\t\t---------\n");
		printf("\t\t[D]evice select\n");
		printf("\t\t[R]ead device (display or write to file)\n");
		printf("\t\t[P]rogram user file\n");
		printf("\t\t[B]yte - program single byte\n");
		printf("\t\t[W]ord - program single word\n");
		printf("\t\t[C]hip erase\n");
		printf("\t\t[S]ector erase\n");
		printf("\t\t[T]oggle DQ6 or DQ7 Polling (default is DQ7 Polling)\n");
		printf("\t\t[F]ilter upper 128 characters for Reads\n");
		printf("\t\t[N]o DQx polling\n");
		printf("\t\t[Q]uit program\n\n");
		printf("Please enter Letter of needed selection: ");
        selection_L = _getche();
		printf("\n");
		switch (selection_L) {

			/* Select the device to use */
			case 'd':
			case 'D':	
				err_val_L = sel_device();
				if (err_val_L != 0) {
					error_handler(err_val_L);
				}
				printf("\n");
				break;

			/* Read the device selected */
			case 'r':
			case 'R':	
				if (no_device_G) {
					err_val_L = SELECT_DEVICE_FIRST;
				}
				else {
					err_val_L=write_bin();
				}
				if (err_val_L != 0) {
					error_handler(err_val_L);
				}
				printf("\n");
				break;

			/* Program a single byte on the device */
			case 'b':
			case 'B':	
				if (no_device_G) {
					err_val_L = SELECT_DEVICE_FIRST;
				}
				else {
					if ((this_dev_GSP->device_no_GS == Am29F100T) || 
						(this_dev_GSP->device_no_GS == Am29F100B) || 
						(this_dev_GSP->device_no_GS == Am29F200B) || 
						(this_dev_GSP->device_no_GS == Am29F200B) || 
						(this_dev_GSP->device_no_GS == Am29F400B) || 
						(this_dev_GSP->device_no_GS == Am29F400B))
						{
						err_val_L = NOT_FOR_THIS_DEVICE;
					}
					else {
						err_val_L = program(SINGLE);
					}
				}
				if (err_val_L != 0) {
					error_handler(err_val_L);
				}
				printf("\n");
				num_bytes_G=0;
				break;

			/* Program a single word on the device */
			case 'w':
			case 'W':	
				if (no_device_G) {
					err_val_L = SELECT_DEVICE_FIRST;
				}
				else {
					if ((this_dev_GSP->device_no_GS == Am29F100T) || 
						(this_dev_GSP->device_no_GS == Am29F100B) || 
						(this_dev_GSP->device_no_GS == Am29F200B) || 
						(this_dev_GSP->device_no_GS == Am29F200B) || 
						(this_dev_GSP->device_no_GS == Am29F400B) || 
						(this_dev_GSP->device_no_GS == Am29F400B))
					{
						err_val_L = program(WORD);
					}
					else {
						err_val_L = NOT_FOR_THIS_DEVICE;
					}
				}
				if (err_val_L != 0) {
					error_handler(err_val_L);
				}
				printf("\n");
				num_bytes_G = 0;
				break;


			/* Program a device */
			case 'p':
			case 'P':	
				if (no_device_G) {
					err_val_L = SELECT_DEVICE_FIRST;
				}
				else { 
					/* must ask user for file name here and
						any parsing needed */
						
					if (flerror_code_G == 0) {			/* fixed 5/28/94 return error or HEX/BIN */
						err_val_L=program(i_L);
					}
					else {
						err_val_L = flerror_code_G;
					}
				}
				if (err_val_L != 0) {
					error_handler(err_val_L);
				}
				printf("\n");
				last_file_addr_G = 0L;
				this_address_G = 0L;
				num_bytes_G = 0;						
				_fcloseall();
				break;

			/* Perform a Chip Erase to the device */
			case 'c':
			case 'C':	
				if (no_device_G) {
					err_val_L = SELECT_DEVICE_FIRST;
				}
				else {
					if ((this_dev_GSP->device_no_GS == Am28F010) || 
						(this_dev_GSP->device_no_GS == Am28F020) || 
						(this_dev_GSP->device_no_GS == Am28F256) || 
						(this_dev_GSP->device_no_GS == Am28F512))
					{
						err_val_L = std_erase();
					}
					else {
						err_val_L=emb_erase();
					}
				}
				if (err_val_L != 0) {
					error_handler(err_val_L);
				}
				printf("\n");
				break;

			/* Toggle between DQ7/DQ5 and DQ6/DQ5 Polling */
			case 't':
			case 'T':
				if (use_dq7_G) {
					use_dq7_G=0;
					printf("\nDQ6 Polling now in effect\n");
				}
				else {
					use_dq7_G=1;
					printf("\nDQ7 Polling now in effect\n");
				}
				break;

			case 'n':
			case 'N':	
				if (no_poll_G) {
					no_poll_G = 0;
					printf("\nDQx polling enabled\n");
				}
				else {
					no_poll_G = 1;
					printf("\nDQx polling disabled; read-until-valid active\n");
				}
				break;

			case 'f':
			case 'F':	
				if (filter_read_G) {
					filter_read_G = 0;
					printf("\nDevice Reads are not filtered\n");
				}
				else {
					filter_read_G = 1;
					printf("\nDevice Reads are filtered\n");
				}
				break;

			/* Exit this program */
			case 'q':
			case 'Q':	
				end_prog_L=1;
				break;

			/* Enter the Sector Operations menu */
			case 's':
			case 'S':   
				{
				if (no_device_G) {
					err_val_L = SELECT_DEVICE_FIRST;
				}
				else {
					do {
						if ((this_dev_GSP->device_no_GS == Am29F100T) || 
							(this_dev_GSP->device_no_GS == Am29F100B) || 
							(this_dev_GSP->device_no_GS == Am29F200B) || 
							(this_dev_GSP->device_no_GS == Am29F200B) || 
							(this_dev_GSP->device_no_GS == Am29F400B) || 
							(this_dev_GSP->device_no_GS == Am29F400B) ||
							(this_dev_GSP->device_no_GS == Am29F040)  ||
							(this_dev_GSP->device_no_GS == Am29F080)  ||
							(this_dev_GSP->device_no_GS == Am29F016)) {

								printf("\n\t\t\tSector Erase Menu\n");
								printf("\t\t\t-----------\n");			
								printf("\t\t[Y]es to Erase Suspend\n");
								printf("\t\t[N]o to Erase Suspend\n");
								printf("\t\t[R]eturn to previous menu\n");		
								printf("Please enter Letter of needed selection: ");
								selection2_L = _getche();
								printf("\n");												

								switch(selection2_L) {

								/* Do Erase Suspend */
									case 'y':
									case 'Y':
										err_val_L = sector_erase( TRUE );
										if (err_val_L != 0) 
											error_handler(err_val_L);
										selection2_L = 'r';	//just so it doesn't give same menu again
															// even on error
              	                        break;
        
        						/* Do not Erase Suspend */
									case 'n':
									case 'N':
										err_val_L = sector_erase( FALSE );
										if (err_val_L != 0) 
											error_handler(err_val_L);
										selection2_L = 'r';	//just so it doesn't give same menu again												break;
															// even on error
											
								/* Return to the previous menu */
									case 'r':
									case 'R':
									default :
										break;

								} /* switch ( selection2_L ) */
                        } //if 
                                        
                        else { //not any of the above: 29F010 only
							err_val_L = sector_erase( FALSE );
							if (err_val_L != 0) 
								error_handler(err_val_L);
							selection2_L = 'r';	// to get out
                        }	//else

					} while (selection2_L != 'r' && selection2_L != 'R');

				} //else

				printf("\n");
				break;

			} /* Sector Operation */
            default:
            	break;

		} /* switch (selection_L ) */

	} while (!end_prog_L);

} /* main */