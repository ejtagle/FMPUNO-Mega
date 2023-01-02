/************************************************************************/
/*                                                                      */
/*  AMD Flash Memory Drivers                                            */
/*  File name: STATUS.C                                                 */
/*  Revision:  1.1  9/17/98                                             */
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
/* Status.c is an updated version of the flash_status() function        */
/* referenced in flash.c .  This updated version of flash_status() can  */
/* be substituted in cases where more demanding status checking is of   */
/* concern to a systems designer.  This algorithm can distinguish       */
/* between DQ6 and DQ2 toggle bits, and can return DQ3 status during    */
/* the Sector Erase timeout window (see datasheet).  For systems which  */
/* only care if the flash is 'busy' or 'not busy', the functionality    */
/* of this newer algorithm is not needed, and only serves to increase   */
/* resultant code size.                                                 */
/************************************************************************/

int flash_status(word far *fp)
{
         /* Remeber that we only need data bits 0 through 7, so if
            we cast to unsigned char we'll get the right ones */
         
         /* We don't normally use the DQ7# bit since it's akward to
            'remember' the data and pass it to this function....we can
            get everything we need through combinational use of the other
            bits */ 

         unsigned char d, t;
         int retry = 1;

again:

         d = *fp;        /* read data */
         t = d ^ *fp;    /* read it again and see what toggled */

         if (t == 0) {           /* no toggles, nothing's happening */
             return STATUS_READY;
         }
         else if (d & 0x20 && (t & 0x40)) {   /* Is there a DQ5 error (along with a DQ6 toggle)? */
             if (d & 0x10) {                  /* Is DQ4 set to 1? Indicates an erase error... */
                 if (retry--) goto again;     /* may have been write completion? */
                 return STATUS_TIMEOUT_AUTOERASE;
             } else {
                 if (retry--) goto again;     /* may have been write completion? */
                 return STATUS_TIMEOUT_PREPROGRAM;
             }
         }
         else if (   t & 0x40 &&              /* Is DQ6 toggling? */
                   !(t & 0x04)    ) {         /* Is DQ2 not toggling? */
             return STATUS_BUSY_PROGRAMMING;
         }
         else if (   t & 0x40 &&              /* Is DQ6 toggling? */
                     t & 0x04 &&              /* Is DQ2 toggling? */
                     d & 0x08     ) {         /* Is DQ3 set? */
             return STATUS_BUSY_ERASING;
         }
         else if (   t & 0x40 &&              /* Is DQ6 toggling? */
                     t & 0x04 &&              /* Is DQ2 toggling? */
                     !(d & 0x08)     ) {      /* Is DQ3 not yet set? */
             return STATUS_BUSY_ERASE_WINDOW_OPEN;
         }
         else if ( !(t & 0x40) &&             /* Is DQ6 not toggling? */
                     t & 0x04  &&             /* Is DQ2 toggling? */
                     d & 0x80     ) {         /* Is DQ7 set to 1? */
 
             if (retry--) goto again;    /* may have been write completion? */
                 return STATUS_ERASE_SUSPENDED;
         }

         /* It always possible that we'll read status right when a write
            completes, causing a wierd toggling combination.  We'll check twice
            just to make sure */

         if (retry--) goto again;    /* may have been write completion */

         return STATUS_ERROR;
}
