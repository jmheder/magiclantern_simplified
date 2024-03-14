/** \file
 * Entry points into the firmware image.
 *
 * These are the functions that we can call from our tasks
 * in the Canon 1.1.2 firmware for the 7D Mark II.
 *
 */
/*
 * Copyright (C) 2023 Magic Lantern Team
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

#include <stub.h>

THUMB_FN(0x8000626a EngDrvOut);
THUMB_FN(0x800062a4 EngDrvBits);
THUMB_FN(0x800062ba EngDrvOuts);
THUMB_FN(0x80006298 shamem_read);
THUMB_FN(0x800063c2 StartEDmac);
THUMB_FN(0x80006498 edmac_set_addr);
THUMB_FN(0x800064c2 edmac_set_size);
THUMB_FN(0x80006bfe ConnectWriteEDmac);
THUMB_FN(0x80006c10 ConnectReadEDmac);
THUMB_FN(0x80006c42 RegisterEDmacCompleteCBR);
