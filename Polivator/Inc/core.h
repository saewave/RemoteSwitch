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

#include "stdint.h"
#include "stm32f0xx.h"

#define STATE_CHECK_HUMIDITY_ADC_WAIT_GND_TIMEOUT 1
#define STATE_CHECK_HUMIDITY_ADC_CHECK_GND_TIMEOUT 1
#define STATE_CHECK_HUMIDITY_ADC_ON_TIMEOUT 3

#define CHANNELS_CONFIG_FLASH_ADDRESS 0x08007C00 + 32

#define ADC_CH_LEN 3
#define ADC_CH_CNT 10

enum mState { 
    STATE_IDLE           = 0, 
    STATE_GND            = STATE_IDLE    + 1,
    STATE_GND_ADC        = STATE_GND     + 1,
    STATE_ON             = STATE_GND_ADC + 1,
    STATE_ON_ADC         = STATE_ON      + 1,
    STATE_CALC           = STATE_ON_ADC  + 1,
    STATE_WATERING       = STATE_CALC    + 1,
    STATE_RESET          = STATE_WATERING+ 20
};

enum chStatus { 
    CH_STATUS_NORMAL     = 0,
    CH_STATUS_WATERING   = 1,
    CH_STATUS_ECOVERFLOW = 2,
    CH_STATUS_ENOWATER   = 3,
    CH_STATUS_DISABLED   = 0xff
};

typedef struct {
    uint16_t      MinDryness;       //Min Dryness. value by ADC is 0 - very Wet! *** Stop watering ***
    uint16_t      MaxDryness;       //Max Dryness. value by ADC is 4096 - very Dry!  *** Need watering ***
    uint8_t       Cycles;           //Current watering cycles
    uint8_t       MaxCycles;        //Max watering cycles
    uint8_t       CycleTime;        //Time of each watering cycle
    uint8_t       CycleTimeCounter; //Counter
    enum chStatus Status;           //0 - Normal, 1 - watering, 2 - Error: Cycles overflow, 3 - Error: No water of pump is broken, 0xFF - disabled
    GPIO_TypeDef  *GPIO_Port;
    uint32_t      GPIO_Pin;
} tChannelSettings;

typedef struct
{
    uint8_t H : 8;
    uint8_t L : 8;
} WByte;

typedef struct {
    uint8_t       Channel;
    WByte         MinDryness;       //Min Dryness. value by ADC is 0 - very Wet! *** Stop watering ***
    WByte         MaxDryness;       //Max Dryness. value by ADC is 4096 - very Dry!  *** Need watering ***
    uint8_t       MaxCycles;        //Max watering cycles
    uint8_t       CycleTime;        //Time of each watering cycle
//    uint8_t       CycleTimeCounter; //Counter
    enum chStatus Status;           //0 - Normal, 1 - watering, 2 - Error: Cycles overflow, 3 - Error: No water of pump is broken, 0xFF - disabled
} tUpdateChannelSettings;

void Process(void);
void SwitchToState(enum mState newState);
uint8_t SetParamsToChannel(tUpdateChannelSettings * Data);
