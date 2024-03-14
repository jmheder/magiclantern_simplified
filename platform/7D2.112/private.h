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

/////////////////////////////////////////////////// 
// Code to stay  
///////////////////////////////////////////////////

// prototypes 
void FAST vsync_func();
void ds_engio_write(uint32_t *reg_list);

// private logging   
void savelog();
void startlog();
void addlog(uint32_t reg, uint32_t value, uint32_t bits, uint32_t lr);

#define LOGGING     1

// bit 0 
#define BIT0        0
#define CANON       0x0
#define ML          0x1 << BIT0
#define SW_MASK     0x1 << BIT0

// bit 1
#define BIT1        1
#define CPU_0       0x0
#define CPU_1       0x1 << BIT1
#define CPU_MASK    0x1 << BIT1  

// bit 2
#define BIT2        2
#define MARIUS      0x0
#define OMAR        0x1 << BIT2
#define CORE_MASK   0x1 << BIT2

// bit 3
#define BIT3        3
#define WRITE       0x0
#define READ        0x1 << BIT3
#define WRITE_MASK  0x1 << BIT3

// bit 4
#define BIT4        4
#define FRAMESYNC_MASK  0x1 << BIT4

// bit 5
#define BIT5        5
#define SIMULATION_MASK  0x1 << BIT5

// bit 5
#define BIT6        6
#define FPS_MASK  0x1 << BIT6

// bit 6
#define BIT7        7
#define VALUE_MASK  0x1 << BIT7

