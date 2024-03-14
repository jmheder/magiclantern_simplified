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

extern int uart_printf(const char *fmt, ...);

void LoadCalendarFromRTC(struct tm *tm)
{
    // differs from D78, one arg is missing
    _LoadCalendarFromRTC(tm, 0, 16);
}

/*
 * Partition tables stuff. Copied from R as I was too lazy to search for stub.
 * And clean implementation in ML code is worth it anyway I guess.
 */

struct chs_entry
{
    uint8_t head;
    uint8_t sector; // sector + cyl_msb
    uint8_t cyl_lsb;
} __attribute__((packed));

struct partition
{
    uint8_t state;
    struct chs_entry start;
    uint8_t type;
    struct chs_entry end;
    uint32_t start_sector;
    uint32_t size;
} __attribute__((aligned, packed));

struct partition_table
{
    uint8_t state; // 0x80 = bootable
    uint8_t start_head;
    uint16_t start_cylinder_sector;
    uint8_t type;
    uint8_t end_head;
    uint16_t end_cylinder_sector;
    uint32_t sectors_before_partition;
    uint32_t sectors_in_partition;
} __attribute__((packed));

void fsuDecodePartitionTable(void *partIn, struct partition_table *pTable)
{
    struct partition *part = (struct partition *)partIn;
    pTable->state = part->state;
    pTable->type = part->type;
    pTable->start_head = part->start.head;
    pTable->end_head = part->end.head;
    pTable->sectors_before_partition = part->start_sector;
    pTable->sectors_in_partition = part->size;

    // tricky bits - TBD
    pTable->start_cylinder_sector = 0;
    pTable->end_cylinder_sector = 0;

    uart_printf("Bootflag: %02x\n", pTable->state);
    uart_printf("Type: %02x\n", pTable->type);
    uart_printf("Head start: %02x end %02x\n", pTable->start_head, pTable->end_head);
    uart_printf("Sector start: %08x size %08x\n", pTable->sectors_before_partition, pTable->sectors_in_partition);
    uart_printf("CS: Start %04x End %04x\n", pTable->start_cylinder_sector, pTable->end_cylinder_sector);
}

// DryOs DM levels 
void dm_set_store_level( uint32_t class, uint32_t level )
{
    call("dmstore",class,level);
}

void dm_set_print_level( uint32_t class, uint32_t level )
{
    call("dmprint",class,level);
}

int get_task_info_by_id(int unknown_flag, int task_id, void *task_attr)
{
    // task_id is something like two u16s concatenated.  The flag argument,
    // present on D45 but not on D678 allows controlling if the task info request
    // uses the whole thing, or only the low half.
    //
    // ML calls with this set to 1, meaning task_id is used as is,
    // if 0, the high half is masked out first.
    //
    // D678 doesn't have the 1 option, we use the low half as index
    // to find the full value.
    struct task *task = first_task + (task_id & 0xffff);
    return _get_task_info_by_id(task->taskId, task_attr);
}

// We can't patch instructions located (only in RAM ... )
int patch_instruction(uintptr_t addr,
                      uint32_t old_value,
                      uint32_t new_value,
                      const char *description)
{
}

int unpatch_memory(uintptr_t addr)
{
    return 0; 
}

const char pic_msg[] = "Unknown"; 
const char* get_picstyle_name(int raw_picstyle)
{
  return pic_msg;
}

struct LockEntry *CreateResLockEntry(uint32_t *resIds, uint32_t resIdCount)
{
    static uint32_t dummy;
    return (struct LockEntry *)&dummy;
}

unsigned int LockEngineResources(struct LockEntry *lockEntry)
{
  return 0; 
}

unsigned int UnLockEngineResources(struct LockEntry *lockEntry)
{
  return 0; 
}

void dcache_lock() {}
void icache_lock() {}
void dcache_unlock() {}
void icache_unlock() {}
uint32_t cache_locked() {return 1;}
void cache_lock(){}
uint32_t cache_fake(uint32_t address, uint32_t data, uint32_t type){ return address;}




