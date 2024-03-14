/** \file
 * Function overrides needed for 7D2 1.1.2
 */
/*
 * Copyright (C) 2021 Magic Lantern Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include <dryos.h>
#include <property.h>
#include <bmp.h>
#include <config.h>
#include <consts.h>
#include <lens.h>
#include <edmac.h>
#include <patch.h>
#include <arm-mcr.h>
#include "private.h"

/**************************************************************************************************************************/
/* module_exec_cbr(CBR_VSYNC) (vsync_func) syncronization. The state_object is present in the camerra but I've failed     */
/* to find the state_objects needed, perhaps they're inside TCM on the slave, or inside master or slave Omar, thus       */  
/* use another trigger (LVEVF interrupt)                                                                                  */
/**************************************************************************************************************************/
void master_vsync_func(void)
{
  // add to log
  addlog(0,get_ms_clock(),CPU_0|MARIUS|ML|FRAMESYNC_MASK,0xEEEEEEEE);

  vsync_func(); // calls the vsync in stateobjects.c
} 

/**************************************************************************************************************************/
/** Task to handle various stuff                                                                                         **/
/**************************************************************************************************************************/

// hijacks various functions 
void function_overwrite_init(void)
{
   // insert vsync after FW loading 
   msleep(1000);
   startlog();
   uint32_t old = cli();
   MEM((0x000094d6)) = BL_INST_T2(0x000094d6,&master_vsync_func); // vsync
   MEM((0x00013ea0)) = B_INST_T2(0x00013ea0,&ds_engio_write); // ds_engio
   call("lv_save_raw",1);
   sei(old);  
}

INIT_FUNC(__FILE__, function_overwrite_init);
