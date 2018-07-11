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
 
#include "F030C8_Peripheral.h"
#include "xdebug.h"
#include "stdio.h"

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
    QueueResponse(Buf, OUSART2);
}
#endif
