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

// This is a private logging API for the 7D2 Mark II. Since the camera is quite different from the other models I needed
// to log engio calls without using the DryOsDebugMessage as this one would simply crash the camera if I needed to log 
// all engio calls. The known_regs was taking from a older repository and the register map might not be correct!, the 7D2
// uses 0xD00xxxxxxx instead of 0xC0Fxxxxx. The iodata struct is coded to approx 10000 lines.

#ifdef LOGGING
#define MAX_LINES   10000

// Purpose of this file is to create a private logging api, that only logs specific stuff rather than
// using Canon DryOsDebugMessage

typedef struct
{
    uint32_t reg;     // address/register/line
    uint32_t value;   // value/time
    uint32_t lr;      // caller id/lr
    uint32_t bits;
} iodata;

uint32_t io_pos = 0;
uint32_t io_overlap = 0;
bool io_lock = false;
static char msg[2048];
static iodata io[MAX_LINES];

// from old adtg gui module
struct known_reg
{
    uint16_t dst;
    uint16_t reg;
    uint16_t is_nrzi;   
    char* description;
};

const struct known_reg known_regs[] = {

    {0xD000,   0x819c, 0, "Saturate Offset (photo mode) (HIV_POST_SETUP)"},
    {0xD001,   0x2054, 0, "White level?"},

    {0xD000,   0x6000, 0, "FPS register for confirming changes"},
    {0xD000,   0x6004, 0, "FPS related, SetHeadForReadout"},
    {0xD000,   0x6008, 0, "FPS register A"},
    {0xD000,   0x600C, 0, "FPS related"},
    {0xD000,   0x6010, 0, "FPS related"},
    {0xD000,   0x6014, 0, "FPS register B"},
    {0xD000,   0x6018, 0, "FPS related"},
    {0xD000,   0x601C, 0, "FPS related"},
    {0xD000,   0x6020, 0, "FPS related"},

    {0xD000,   0x6084, 0, "RAW first line|column."},
    {0xD000,   0x6088, 0, "RAW last line|column."},

    {0xD000,   0x6800, 0, "RAW first line|column."},
    {0xD000,   0x6804, 0, "RAW last line|column."},

    {0xD000,   0x7000, 0, "HEAD timers (SSG counter, 0x01 to restart)"},
    {0xD000,   0x7004, 0, "HEAD timers"},
    {0xD000,   0x700C, 0, "HEAD timers, 0x01 to stop/standby"},
    {0xD000,   0x7010, 0, "HEAD timers"},
    {0xD000,   0x7014, 0, "HEAD timers"},
    {0xD000,   0x7018, 0, "HEAD timers"},
    {0xD000,   0x701C, 0, "HEAD timers"},
    {0xD000,   0x7038, 0, "HEAD timers, 0x01 <- stops processing?"},
    {0xD000,   0x707C, 0, "HEAD timers"},
    {0xD000,   0x71AC, 0, "HEAD timers"},
    {0xD000,   0x70C8, 0, "HEAD timers, State 2 Register / VCount?"},

    {0xD000,   0x7048, 0, "HEAD1 timer (start?)"},
    {0xD000,   0x704C, 0, "HEAD1 timer"},
    {0xD000,   0x7050, 0, "HEAD1 timer (ticks?)"},

    {0xD000,   0x705C, 0, "HEAD2 timer (start?)"},
    {0xD000,   0x7060, 0, "HEAD2 timer"},
    {0xD000,   0x7064, 0, "HEAD2 timer (ticks?)"},

    {0xD000,   0x7134, 0, "HEAD3 timer (start?)"},
    {0xD000,   0x7138, 0, "HEAD3 timer"},
    {0xD000,   0x713C, 0, "HEAD3 timer (ticks?)"},

    {0xD000,   0x7148, 0, "HEAD4 timer (start?)"},
    {0xD000,   0x714c, 0, "HEAD4 timer"},
    {0xD000,   0x7150, 0, "HEAD4 timer (ticks?)"},

    {0xD000,   0x8D1C, 0, "Vignetting correction data (DIGIC V)"},
    {0xD000,   0x8D24, 0, "Vignetting correction data (DIGIC V)"},
    {0xD000,   0x8578, 0, "Vignetting correction data (DIGIC IV)"},
    {0xD000,   0x857C, 0, "Vignetting correction data (DIGIC IV)"},
    
    {0xD001,   0x40c4, 0, "Display saturation"},
    {0xD001,   0x41B8, 0, "Display brightness and contrast"},
    {0xD001,   0x4140, 0, "Display filter (EnableFilter, DIGIC peaking)"},
    {0xD001,   0x4164, 0, "Display position (vertical shift)"},
    {0xD001,   0x40cc, 0, "Display zebras (used for fast zebras in ML)"},
    
    {0xD003,   0x7ae4, 0, "ISO digital gain (5D3 photo mode)"},
    {0xD003,   0x7af0, 0, "ISO digital gain (5D3 photo mode)"},
    {0xD003,   0x7afc, 0, "ISO digital gain (5D3 photo mode)"},
    {0xD003,   0x7b08, 0, "ISO digital gain (5D3 photo mode)"},

    {0xD003,   0x7ae0, 0, "ISO black/white offset (5D3 photo mode)"},
    {0xD003,   0x7aec, 0, "ISO black/white offset (5D3 photo mode)"},
    {0xD003,   0x7af8, 0, "ISO black/white offset (5D3 photo mode)"},
    {0xD003,   0x7b04, 0, "ISO black/white offset (5D3 photo mode)"},
    
    /* from http://magiclantern.wikia.com/wiki/Register_Map#Misc_Registers */
    {0xD000,   0x8004, 0, "DARK_MODE (bitmask of bits 0x113117F)"},
    {0xD000,   0x8008, 0, "DARK_SETUP (mask 0x7FF signed) (brightns/darkens frame)"},
    {0xD000,   0x800C, 0, "DARK_LIMIT (mask 0x3FFF) (no noticeable change)"},
    {0xD000,   0x8010, 0, "DARK_SETUP_14_12 (mask 0x07FF) (brighten, overwrites DARK_SETUP)"},
    {0xD000,   0x8014, 0, "DARK_LIMIT_14_12 (0x0000 - 0x0FFF) (no noticeable change)"},
    {0xD000,   0x8018, 0, "DARK_SAT_LIMIT (0x0000 - 0x3FFF) (no noticeable change)"},
    {0xD000,   0x82A0, 0, "DARK_KZMK_SAV_A (0/1) (causes white or black screen)"},
    {0xD000,   0x82A4, 0, "DARK_KZMK_SAV_B (0/1) (no noticeable change)"},

    {0xD000,   0x8100, 0, "CCDSEL (0-1)"},
    {0xD000,   0x8104, 0, "DS_SEL (0-1)"},
    {0xD000,   0x8108, 0, "OBWB_ISEL (0-7)"},
    {0xD000,   0x810C, 0, "PROC24_ISEL (0-7)"},
    {0xD000,   0x8110, 0, "DPCME_ISEL (0-15)"},
    //{0xD000,   0x8114, 0, "PACK32_ISEL (0-15)"},
    {0xD000,   0x82D0, 0, "PACK16_ISEL (0-15)"},
    {0xD000,   0x82D4, 0, "WDMAC32_ISEL (0-7)"},
    {0xD000,   0x82D8, 0, "WDMAC16_ISEL (0-1)"},
    {0xD000,   0x82DC, 0, "OBINTG_ISEL (0-15)"},
    {0xD000,   0x82E0, 0, "AFFINE_ISEL (0-15)"},
    {0xD000,   0x8390, 0, "OBWB_ISEL2 (0-1)"},
    {0xD000,   0x8394, 0, "PROC24_ISEL2 (0-1)"},
    {0xD000,   0x8398, 0, "PACK32_ISEL2 (0-3)"},
    {0xD000,   0x839C, 0, "PACK16_ISEL2 (0-3)"},
    {0xD000,   0x83A0, 0, "TAIWAN_ISEL (0-3)"},

    {0xD000,   0x8220, 0, "ADKIZ_ENABLE?"},
    {0xD000,   0x8224, 0, "ADKIZ_THRESHOLD"},
    {0xD000,   0x8238, 0, "ADKIZ_INTR_CLR"},
    {0xD000,   0x825C, 0, "ADKIZ_THRESHOLD_14_12"},
    {0xD000,   0x8234, 0, "ADKIZ_TOTAL_SIZE"},
    {0xD000,   0x823C, 0, "ADKIZ_INTR_EN"},

    {0xD000,   0x8060, 0, "DSUNPACK_ENB?"},
    {0xD000,   0x8064, 0, "DSUNPACK_MODE"},
    {0xD000,   0x8274, 0, "DSUNPACK_DM_EN"},

    {0xD000,   0x8130, 0, "DEFM_ENABLE?"},
    {0xD000,   0x8138, 0, "DEFM_MODE"},
    {0xD000,   0x8140, 0, "DEFM_INTR_NUM"},
    {0xD000,   0x814C, 0, "DEFM_GRADE"},        // RealtimeDefectsGrade
    {0xD000,   0x8150, 0, "DEFM_DAT_TH"},
    {0xD000,   0x8154, 0, "DEFM_INTR_CLR"},
    {0xD000,   0x8158, 0, "DEFM_INTR_EN"},
    {0xD000,   0x815C, 0, "DEFM_14_12_SEL"},
    {0xD000,   0x8160, 0, "DEFM_DAT_TH_14_12"},
    {0xD000,   0x816C, 0, "DEFM_X2MODE"},

    {0xD000,   0x8180, 0, "HIV_ENB"},
    //~ {0xD000,   0x8184, 0, "HIV_V_SIZE"},
    //~ {0xD000,   0x8188, 0, "HIV_H_SIZE"},
    {0xD000,   0x818C, 0, "HIV_POS_V_OFST"},
    {0xD000,   0x8190, 0, "HIV_POS_H_OFST"},
    //{0xD000,   0x819C, 0, "HIV_POST_SETUP"},
    {0xD000,   0x8420, 0, "HIV_BASE_OFST"},
    {0xD000,   0x8428, 0, "HIV_GAIN_DIV"},
    {0xD000,   0x842C, 0, "HIV_PATH"},
    {0xD000,   0x8218, 0, "HIV_IN_SEL"},
    {0xD000,   0x8214, 0, "HIV_PPR_EZ"},
    {0xD000,   0x82C4, 0, "HIV_DEFMARK_CANCEL"},

    {0xD000,   0x8240, 0, "ADMERG_INTR_EN"},
    {0xD000,   0x8244, 0, "ADMERG_TOTAL_SIZE"},
    {0xD000,   0x8250, 0, "ADMERG_2_IN_SE"},

    {0xD000,   0x8020, 0, "SHAD_ENABLE?"},
    //{0xD000,   0x8024, 0, "SHAD_MODE"},
    {0xD000,   0x8028, 0, "SHADE_PRESETUP"},
    {0xD000,   0x802C, 0, "SHAD_POSTSETUP"},
    //{0xD000,   0x8030, 0, "SHAD_GAIN"},
    //{0xD000,   0x8034, 0, "SHAD_PRESETUP_14_12"},
    {0xD000,   0x8038, 0, "SHAD_POSTSETUP_14_12"},
    {0xD000,   0x8280, 0, "SHAD_CBIT"},
    {0xD000,   0x8284, 0, "SHAD_C8MODE"},
    {0xD000,   0x8288, 0, "SHAD_C12MODE"},
    {0xD000,   0x8290, 0, "SHAD_COF_SEL"},
    {0xD000,   0x828C, 0, "SHAD_RMODE"},
    {0xD000,   0x82A8, 0, "SHAD_KZMK_SAV"},

    {0xD000,   0x8040, 0, "TWOADD_ENABLE"},
    {0xD000,   0x8044, 0, "TWOADD_MODE"},
    {0xD000,   0x8050, 0, "TWOADD_SETUP_14_12"},
    {0xD000,   0x8054, 0, "TWOADD_LIMIT_14_12"},
    {0xD000,   0x8048, 0, "TWOADD_SETUP"},
    {0xD000,   0x804C, 0, "TWOADD_LIMIT"},
    {0xD000,   0x8058, 0, "TWOADD_SAT_LIMIT"},
    {0xD000,   0x82AC, 0, "TWOA_KZMK_SAV_A"},
    {0xD000,   0x82B0, 0, "TWOA_KZMK_SAV_B"},
    
    {0xD000,   0x8114, 0, "LV raw type (see lv_af_raw, lv_set_raw) - DIGIC IV (PACK32_ISEL)"},
    {0xD003,   0x7014, 0, "LV raw type (see lv_af_raw, lv_set_raw) - DIGIC V"},

    {0xD001,   0x0064, 0, "LV resolution (RAW.height | RAW.width)"},     // OK, full raw buffer including optial black
    {0xD000,   0x80B0, 0, "LV resolution (RAW.height | RAW.width)"},     // values matches EDMAC size
    {0xD000,   0x8184, 0, "LV resolution (RAW.height) aka HIV_V_SIZE "}, // however, changing them has either no effect or camera locks up :(
    {0xD000,   0x8188, 0, "LV resolution (RAW.width) aka HIV_H_SIZE "},
    {0xD000,   0x8194, 0, "LV resolution (RAW.width)"},
    {0xD000,   0x8198, 0, "LV resolution (RAW.width)"},
    
    {0xD001,   0xD014, 0, "LV resolution (raw.j.height? | raw.j.width?)"},  // values a little larger than active area (?)
    {0xD001,   0x1394, 0, "LV resolution (raw.j.height | raw.j.width)"},
    {0xD001,   0x151c, 0, "LV resolution (raw.j.height | raw.j.width)"},
    {0xD001,   0xa00c, 0, "LV resolution (raw.j.width | raw.j.height)"},
    {0xD002,   0x5054, 0, "LV resolution (raw.j.width)"},
    {0xD002,   0x500c, 0, "LV resolution (raw.j.width)"},
    {0xD001,   0x155c, 0, "LV resolution (raw.j.height | hd.width)"},
    {0xD001,   0x12d4, 0, "LV resolution (raw.j.height | ?) before upsampling?"},           // these two also change at 5x->10x zoom
    {0xD001,   0x1314, 0, "LV resolution (raw.j.height | lv.width) before upsampling?"},    // ratio is around 1.4, so maybe some upsampling is applied afterwards

    {0xD000,   0x8548, 0, "LV resolution * downsize factor? (RAW.height * D | RAW.width * D)"},

    {0xD000,   0x9050, 0, "Aewb metering area (y1|x1)"},
    {0xD000,   0x9054, 0, "Aewb metering area (y2|x2)"},

    {0xD003,   0x83d4, 0, "Preview area (y1 | x1/4)"},  /* similar to raw_info.active_area */
    {0xD003,   0x83dc, 0, "Preview area (y2 | x2/4)"},
    {0xFFFF,   0xFFFF, 0, ""},
};

char *description(uint32_t address, uint32_t value)
{
    int i=0;
    static char text[10] = "?"; 

    while (known_regs[i].dst != 0xFFFF)
    {
       if (known_regs[i].dst == 0xFFFF)
        break;

       if ((address & 0xFFFF0000)>>16 == known_regs[i].dst && (address & 0xFFFF) == known_regs[i].reg)
         return known_regs[i].description;
       i++;
    }

    return text;
}

/* reset log position */
void startlog(void)
{
  io_pos = 0;
}

/* add a log entry */
void addlog(uint32_t reg, uint32_t value, uint32_t bits, uint32_t lr)
{
   if (io_lock == true)
     return;

    // wrap around is needed, keep the last 
    if (MAX_LINES > 7000 && io_pos == MAX_LINES)
    {
        io_pos = 7000;
        io_overlap = 1;
    }

    if (io_pos < MAX_LINES)
    {
      io[io_pos].reg = reg; 
      io[io_pos].value = value; 
      io[io_pos].lr = lr;
      io[io_pos].bits = bits; 
      io_pos++;
    }
}

/* save log to disk */
void savelog(void)
{
    FILE *pFile;
    int j = 0;    
    io_lock = true;  

    pFile = FIO_CreateFile("debuglog.txt");
    
    if (io_overlap)
      io_pos = 10000;

    for (int i=0;i<io_pos;i++)
    {
        uint32_t ML_OR_CN = io[i].bits & SW_MASK;
        uint32_t M_OR_O = io[i].bits & CORE_MASK;
        uint32_t CPU = io[i].bits & CPU_MASK;
        uint32_t FrameSync = io[i].bits & FRAMESYNC_MASK;
        uint32_t ValueLog = io[i].bits & VALUE_MASK;
        uint32_t Simulation = io[i].bits & SIMULATION_MASK;
        uint32_t Write = io[i].bits & WRITE_MASK;
        uint32_t fps = io[i].bits & FPS_MASK;

        // keep the last 3000 lines running .. 
        if (io_overlap && i == 7000)
        {
          j+= snprintf(msg + j,4096,"-------------------- OVERLAP -------------------- \n");      
        }        

        // frame sync with time 
        if (FrameSync == FRAMESYNC_MASK)
        {  
          j+= snprintf(msg + j,4096,"%6d CPU(%d,%s)(%s) Frame Sync\n",
            io[i].value,
            CPU==0?0:1,
            M_OR_O==0?"MARIUS":"OMAR  ",
            ML_OR_CN==0?"CN":"ML");
        }  
        // generic value logging, id is typical line within a file 
        else if (ValueLog == VALUE_MASK)
        {
          j+= snprintf(msg + j,4096,"id = %d, value = %#8lx [%5d %5d]\n",io[i].reg,io[i].value,io[i].value>>16,io[i].value&0xFFFF);
        }
        // EngDrv, engio_write logging
        else
        {  
          char *txt = description(io[i].reg,io[i].value);
          j+= snprintf(msg + j,4096,"------ CPU(%d,%s)(%s) - IO:%s - LR : %#8lx, ADR : %#8lx, Dat : %#8lx [%5d %5d] - %s%s\n",
            CPU==0?0:1,
            M_OR_O==0?"MARIUS":"OMAR  ",
            ML_OR_CN==0?"CN":"ML",
            Write ? "RD":"WR",
            io[i].lr,
            io[i].reg,
            io[i].value,
            io[i].value>>16,
            io[i].value&0xFFFF,
            Simulation ? "Simulation - ":"", 
            txt);
        }

        if (j>1800)
        {
          FIO_WriteFile(pFile,msg,strlen(msg));
          strcpy(msg,"");
          j=0;
        }
    } 

    j+= snprintf(msg + j,4096,"MEMORY MAP 0xD0006000+ : %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx\n",
      shamem_read(0xd0006000),
      shamem_read(0xd0006004),
      shamem_read(0xd0006008),
      shamem_read(0xd000600c),
      shamem_read(0xd0006010),
      shamem_read(0xd0006014),
      shamem_read(0xd0006018),
      shamem_read(0xd000601c),
      shamem_read(0xd0006020),
      shamem_read(0xd0006024));
    FIO_WriteFile(pFile,msg,strlen(msg));
    FIO_CloseFile(pFile);
    io_lock = false;
    io_pos = 0;
}
#else

void savelog(){};
void startlog(){};
void addlog(uint32_t reg, uint32_t value, uint32_t bits, uint32_t iter, uint32_t lr){};

#endif

