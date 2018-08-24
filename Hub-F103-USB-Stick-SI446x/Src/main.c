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
 
#include "stm32f10x.h"
#include "F103RE_Peripheral.h"
#include "SI446x.h"
//#include "rf_device.h"
#include "xdebug.h"
#include "core.h"
//#include "usblib.h"

#define MOVE_VECTOR_TABLE 1

uint8_t Buf[10] = {1,2,3,4,5,6,7,8,9,0};
volatile uint32_t test1;
volatile uint32_t test2;

int main (void) {
#if MOVE_VECTOR_TABLE==1
    SCB->VTOR = SAE_START_MEMORY_ADDRESS;   //Move vector to new address
    __enable_irq();                         //Enable IRQ
#endif

    RCC_Configure();
    CRC_Configure();
    DWT_Configure();
    GPIO_Configure();

    SPI_Configure();
    USART_Configure();
    
#if SAE_ENABLE_USB == 1
    USBLIB_Init();
    GPIOB->ODR |= GPIO_ODR_ODR13; //USB UP
#endif
    
//    Transceiver_Reset();
    Transceiver_Configure();
    
    TIM_Configure();
    Transceiver_RxMode();
    EXTI_Configure();

    rfLoadDevices();
    rfListDevices();

    QueueResponse("OK, I'm fine!\n", OUSART1);
    uEXTI_IRQHandler((uint32_t) 0x10);
    while(1){
        ProcessDeviceTXQueue();
        ProcessDeviceRXQueue();
        ProcessCMDQueue();
        ProcessResponseQueue();
    };
}

void uTIM_IRQHandler(TIM_TypeDef *Tim)
{
    if (Tim == TIM1) {
        GPIOB->ODR ^= GPIO_ODR_ODR12;
//        Transceiver_WriteTXBuf(Buf, 7);
//        Transceiver_StartTX(7);
    }
    
    if (Tim == TIM2) {
        GPIOB->ODR ^= GPIO_ODR_ODR12;
        AddToDeviceTXQueue(0x1234, Buf, 10);
    }
}
