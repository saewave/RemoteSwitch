/* 
 * This file is part of the RemoteSwitch distribution 
 * (https://github.com/saewave/RemoteSwitch).
 * Copyright (c) 2018 Samoilov Alexey.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
#include "F103RE_Peripheral.h"
#include "xdebug.h"
#include "stdio.h"
#include "core.h"

#ifdef SAE_ENABLE_DEBUG
void xprintf(                 /* Put a formatted string to the default device */
             char *fmt, /* Pointer to the format string */
             ...              /* Optional arguments */
             )
{
    va_list arp;
    char Buf[SAE_OUTPUT_DATA_LENGTH];
    va_start(arp, fmt);
    vsnprintf(Buf, SAE_OUTPUT_DATA_LENGTH, fmt, arp);
    va_end(arp);
    uint8_t i;
    for (i = 0; i < SAE_OUTPUT_DATA_LENGTH; i++) {
        if (Buf[i] == 0) {
            break;
        }
    }
    if (i == SAE_OUTPUT_DATA_LENGTH)
        Buf[SAE_OUTPUT_DATA_LENGTH-1] = 0;
    QueueResponse(Buf, OUSART1);
}
#endif
