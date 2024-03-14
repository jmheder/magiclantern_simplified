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

void SetEDmac(unsigned int channel, void *address, struct edmac_info *ptr, int flags)
{
    return;
}

void ConnectWriteEDmac(unsigned int channel, unsigned int where)
{
    return;
}

void ConnectReadEDmac(unsigned int channel, unsigned int where)
{
    return;
}

void StartEDmac(unsigned int channel, int flags)
{
}

void AbortEDmac(unsigned int channel)
{
    return;
}

void RegisterEDmacCompleteCBR(int channel, void (*cbr)(void *), void *cbr_ctx)
{
    return;
}

void UnregisterEDmacCompleteCBR(int channel)
{
    return;
}

void RegisterEDmacAbortCBR(int channel, void (*cbr)(void *), void *cbr_ctx)
{
    return;
}

void UnregisterEDmacAbortCBR(int channel)
{
    return;
}

void RegisterEDmacPopCBR(int channel, void (*cbr)(void *), void *cbr_ctx)
{
    return;
}

void UnregisterEDmacPopCBR(int channel)
{
    return;
}
