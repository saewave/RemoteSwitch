/* 
 * This file is part of the SaeWave RemoteSwitch (USB-CDC-CMSIS) 
 * distribution (https://github.com/saewave/STM32F103-USB-CDC-CMSIS).
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

#ifndef __CONFIG_H
#define __CONFIG_H

#define _DEBUG

#define SAE_FLASH_CFG_ADDR 0x08007C00
#define SAE_FW_SET_MEMORY_ADDRESS 0x0800FC00        //FW settings address (Length, CRC32, etc)
#define SAE_FW_MEMORY_ADDRESS 0x08008000
#define SAE_READ_ADDR_ON_START 0x01
#define SAE_SETUP_TIMEOUT 30 * 1000 // Setup timeout in ms (30 sec)
#define SAE_ALL_TIME_RX_MODE 0x01
#define DEVICE_TYPE 0x01

void readConfig(void);

#define MOVE_VECTOR_TABLE 1
#define MAIN_PROGRAM_START_ADDRESS (uint32_t)0x08000400
#define MAIN_PROGRAM_RAM_ADDRESS 0x20000000
#define USE_STOP_MODE 0
#define DEBUG_IN_STOP_MODE 0

#endif /* __CONFIG_H */
