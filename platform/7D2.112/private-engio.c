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

extern uint32_t _shamem_read(uint32_t _reg);

// This is a private engio override API for the 7D2 Mark II. The purpose of this api is first and foremost to make the 
// camera work flawlessly with the current APi and to overcome some of the problems that would requires additional code
// inside the src/ directory. The current magic lantern code is pretty hard to read.  
//
// FPS override is currently impossible to complete without some hacking as the FPS register B is constantly written
// to the registers, thus hijacking the DS engio write was nessasary. The code check is ML has enable fps override and
// write the new fps values if needed. FPS override also requires shamem_read. 

uint32_t fps_regs[16] = {0,0,0x3050305,0x3050305,0x305,0x816,0x20,0x0,0x2,0x816,0x0,0x0,0x0,0x0,0x0,0x0,0x0}; // some defaults, we dont want to crash during development.

#define LV_INIT       1            // initialization started
#define LV_INIT_DONE  2            // initialization completed
#define LV_RUNNING    3            // initialization completed and first frame was shown (i.e. all fps registers was written)

int lv_state = false;             // save current state of LV initialization
int lv_fps_ready = false;          // true, when LV is fully initialized AND the first frame was shown

// from fps-engio.c
extern int fps_needs_updating;

/* Private shadow maps for fps */
void shamem_write(uint32_t reg, uint32_t value)
{
  // local shamem for FPS override
  if (reg < 0xD0006040 && reg >= 0xD0006000) // private shamem region
  {
    fps_regs[(reg & 0xFF)>>2] = value;
  }

  // PACK32_MODE is never written .. 

}

/* shamem_read */
uint32_t shamem_read(uint32_t _reg)
{
  uint32_t value;
  uint32_t reg = _reg & 0x7FFFF | 0xD0000000;  // new address space 

  if (reg < 0xD0006040 && reg >= 0xD0006000) // private shamem region
   value = fps_regs[(reg & 0xFF)>>2];
  else 
    value = _shamem_read(reg); 

  //addlog(reg,value,CPU_0|MARIUS|ML|READ,0xAAAAAAAA); 
  return value;
}

/* private implementation which also updates shadow memory */
void _EngDrvOut(uint32_t _reg, uint32_t value)
{

  // Remap address from old io-space to new io space
  uint32_t reg = _reg & 0x7FFFF | 0xD0000000;  // new address space 
  //addlog(reg,value,CPU_0|MARIUS|ML|SIMULATION_MASK|WRITE,0xBBBBBBBB);
  shamem_write(reg,value);

   // raw bits for MLV 
  if (_reg == 0xC0F08094) // PACK32_MODE
  {
    // enum from raw.c
    enum {
        MODE_16BIT = 0x130,
        MODE_14BIT = 0x030,
        MODE_12BIT = 0x010,
        MODE_10BIT = 0x000,
    };

    uint32_t bits = 14;
    if (value == MODE_16BIT)
      bits = 16;
    else if (value == MODE_14BIT)
      bits = 14;
    else if (value == MODE_12BIT)
      bits = 12;
    else if (value == MODE_10BIT)
      bits = 10;

    //call("FA_SetCRawBitNum",bits); // use FA function to set the requested levels .. 
  }

  //MEM(reg)=value;
  return;
}


/* used ? */
void _engio_write(uint32_t *reg_list)
{
    uint32_t address = 0;
    uint32_t value = 0;
    uint32_t index = 0;

    while (reg_list[index] != 0xffffffff)
    {
       address = reg_list[index++];
       value = reg_list[index++];
       //_EngDrvOut(address,value); 
    }

   return;
}

/* The hijacked Marius _engio_write (0x00013ea0), this function activates LiveView (0x00017c24), including FPS registers. */
void ds_engio_write(uint32_t *reg_list)
{
  // record lr
  register uint32_t lr;
  asm volatile ("mov %0,LR\n" : "=r" (lr));

  uint32_t reg = 0;
  uint32_t value = 0;
  uint32_t index = 0;
  
  // Identify the current state
  if (lr == 0xfe199abd)
  {
    lv_fps_ready = 0; // no read nor writting allowed
    lv_state = LV_INIT;
  }
  else if (lr == 0xfe1989b5) // all registers (except fps) are initialized 
  {
    lv_state = LV_INIT_DONE;
  }
  else if (lr == 0x12eeb & lv_state == LV_INIT_DONE) // fps registers was written
  {
    lv_state = LV_RUNNING;
  }
  
  // write all values 
  while (reg_list[index] != 0xffffffff)
  {
    reg = reg_list[index++];
    value = reg_list[index++];

    // only in Canon high fps mode we survive changing the fps without loss of the video stream   
    if (lv_state == LV_RUNNING && reg == 0xD0006008)
    {
      if (value == 0x1df01df) // 60 fps mode (x1 mode)  
        lv_fps_ready = 1;
      else      
        lv_fps_ready = 0; // some other mode .. 
    }

    // fps override enabled will only occur when caller is 0x12eeb. 
    if (lr == 0x12eeb)
    {
        if (reg == 0xD0006008 && fps_needs_updating && lv_state == LV_RUNNING)
        {
          MEM(0xD0006008) = shamem_read(0xD0006008) & 0xFFFF;                   
          //addlog(0xD0006008,shamem_read(0xD0006008),CPU_0|MARIUS|ML|WRITE|FPS_MASK,0xCCCCCCCC);
          return;
        }
        else if (reg == 0xD0006014 && fps_needs_updating && lv_state == LV_RUNNING)
        {
          MEM(0xD0006014) = shamem_read(0xD0006014) & 0xFFFF;
          //addlog(0xD0006014,shamem_read(0xD0006014),CPU_0|MARIUS|ML|WRITE|FPS_MASK,0xCCCCCCCC);
          return;
        }
        else if (reg == 0xD0006024 && fps_needs_updating && lv_state == LV_RUNNING)
        {
          MEM(0xD0006014) = shamem_read(0xD0006014) & 0xFFFF;
          //addlog(0xD0006024,shamem_read(0xD0006014),CPU_0|MARIUS|ML|WRITE|FPS_MASK,0xCCCCCCCC);
          return;
        }
    }

    // only update private shadow map if we not in fps override mode ..
    if (lv_state > LV_INIT && fps_needs_updating == false)
    {
      shamem_write(reg,value);
    }      

    //addlog(reg,value,CPU_0|MARIUS|CANON|WRITE,lr);
    MEM(reg) = value;
  }

  return;
}
