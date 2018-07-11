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
#include "SI446x.h"
#include "config.h"
#include "core.h"
#include "rf_cmd.h"
#include "rf_cmd_exec.h"

#if MOVE_VECTOR_TABLE == 1
volatile uint32_t *VectorTable = (volatile uint32_t *)MAIN_PROGRAM_RAM_ADDRESS;
#endif

uint8_t Data[18];

typedef struct {
    uint8_t Seconds : 4;
    uint8_t TenSeconds : 4;

    uint8_t Minutes : 4;
    uint8_t TenMinutes : 4;

    uint8_t Hour : 4;
    uint8_t TenHour : 2;
    uint8_t TimeFormat : 2;

    uint8_t Day : 8;

    uint8_t Date : 4;
    uint8_t TenDate : 4;

    uint8_t Month : 4;
    uint8_t TenMonth : 3;
    uint8_t Century : 1;

    uint8_t Year : 4;
    uint8_t TenYear : 4;
} RTCDateTime;

typedef struct {
    uint8_t Seconds : 4;
    uint8_t TenSeconds : 3;
    uint8_t A1M1 : 1;

    uint8_t Minutes : 4;
    uint8_t TenMinutes : 3;
    uint8_t A1M2 : 1;

    uint8_t Hour : 4;
    uint8_t TenHour : 2;
    uint8_t TimeFormat : 1;
    uint8_t A1M3 : 1;

    uint8_t Date : 4;
    uint8_t TenDate : 2;
    uint8_t DayDate : 1;
    uint8_t A1M4 : 1;
} RTCAlarm1;

typedef struct {
    uint8_t Minutes : 4;
    uint8_t TenMinutes : 3;
    uint8_t A2M2 : 1;

    uint8_t Hour : 4;
    uint8_t TenHour : 2;
    uint8_t TimeFormat : 1;
    uint8_t A2M3 : 1;

    uint8_t Date : 4;
    uint8_t TenDate : 2;
    uint8_t DayDate : 1;
    uint8_t A2M4 : 1;
} RTCAlarm2;

RTCDateTime DateTime;
RTCAlarm1   Alarm1;
RTCAlarm2   Alarm2;
uint8_t     RegControl = 0x1D;
uint8_t     RegStatus  = 0x88;
uint8_t     RegCS[2];
uint8_t     tStatus[8]  = {0};
uint8_t     tBuf[10]    = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
uint8_t     tParams[10] = {0};

int      HandleStatus = 1;
uint32_t Counter      = 0;
uint32_t Blink        = GPIO_BSRR_BR_10;

void InitPeriph(void)
{
    CRC_Configure();
    TIM_Configure();
    GPIO_Configure();
    USART_Configure();

    ADC_Configure();
    RTC_Configure();
    RTC_Time_Configure(0, 0, 0);
    RTC_Alarm_Configure(0xFF, 20, 0); //Alarm each 20 min (Check humidity)

    dxdev_out(USART_SendChar);
    SPI_Configure();
}

int main(void)
{
#if MOVE_VECTOR_TABLE == 1
    for (uint32_t i = 0; i < 48; i++) {
        VectorTable[i] = *(__IO uint32_t *)(MAIN_PROGRAM_START_ADDRESS + (i << 2));
    }
    SYSCFG->CFGR1 |= SYSCFG_CFGR1_MEM_MODE;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;
#endif
    __enable_irq();

    InitPeriph();

    if (SAE_READ_ADDR_ON_START) {
        readConfig();
    }

    Transceiver_Configure();

    dxputs("InitAll Done!\n\n");

#if SAE_ALL_TIME_RX_MODE == 1
    Transceiver_ClearFifo(Transceiver_CLEAR_TX_FIFO | Transceiver_CLEAR_RX_FIFO);
    Transceiver_RxMode();
    EXTI_Configure();
#endif

    rfStartup();

    TIM14->CR1 |= TIM_CR1_CEN;
//TIM16->CR1 |= TIM_CR1_CEN;
#if USE_STOP_MODE == 1
    GOTO_Stop();
    //We should never been here!
    dxputs("What I'm doing here?\n");
#endif
    while (1) {
#if USE_STOP_MODE == 0
        if (HandleStatus) {
            HandleStatus = 0x00;
            Transceiver_HandleStatus();
        }
#endif
    }
}

void uEXTI_IRQHandler(uint32_t Pin)
{
#if USE_STOP_MODE == 1
    Transceiver_HandleStatus();
#else
    HandleStatus = 1;
#endif
}
