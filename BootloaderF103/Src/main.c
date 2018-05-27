/* 
 * This file is part of the RemoteSwitch distribution (https://github.com/saewave/RemoteSwitch).
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

#define MAIN_PROGRAM_START_ADDRESS (uint32_t)0x08000400
#define NEW_FW_START_ADDRESS (uint32_t)0x08008000
#define NEW_FW_SETTINGS_ADDRESS (uint32_t)0x0800FC00
#define NEW_FW_OFFSET_ADDRESS (uint32_t)0x00000400
#define FLASH_FKEY1   0x45670123
#define FLASH_FKEY2   0xCDEF89AB

#define NEW_FW_SETTINGS_ADDRESS_VAL *(__IO uint32_t *)NEW_FW_SETTINGS_ADDRESS
#define NEW_FW_SETTINGS_ADDRESS_VAL_4 *(__IO uint32_t *)(NEW_FW_SETTINGS_ADDRESS + 4)
#define NEW_FW_START_OFFSET NEW_FW_START_ADDRESS | NEW_FW_OFFSET_ADDRESS

#define ENABLE_LED 0
#define BLINK_ON_CRCERR 1

void FLASH_Erase(uint32_t EraseAddress, uint8_t Pages)
{
    FLASH->KEYR = FLASH_FKEY1;
    FLASH->KEYR = FLASH_FKEY2;
    FLASH->CR |= FLASH_CR_PER; // Erase one page
    while (FLASH->SR & FLASH_SR_BSY)
    {
        __NOP();
    };                         // Wait untill memory ready for erase
    for (uint8_t Page = 0; Page < Pages; Page++) {
        FLASH->AR = (uint32_t)(EraseAddress + (Page * 1024)); // Erase address
        FLASH->CR |= FLASH_CR_STRT;
        while (FLASH->SR & FLASH_SR_BSY)// Wait untill memory ready
        {
            __NOP();
        }; 
    }

    FLASH->CR &= ~FLASH_CR_PER;     //This bit must be cleared. In other cases the flash will not work
    while (FLASH->SR & FLASH_SR_BSY)
    {
        __NOP();
    };
    FLASH->CR |= FLASH_CR_LOCK; /* Lock the flash back */
}

void FLASH_Copy(uint32_t Start, uint32_t Length) {
    uint32_t Address = 0, CurFWAddress = MAIN_PROGRAM_START_ADDRESS;
    uint16_t tVal;
    
    FLASH->KEYR = FLASH_FKEY1;
    FLASH->KEYR = FLASH_FKEY2;
    FLASH->CR |= FLASH_CR_PG;       // Allow write
    while(FLASH->SR & FLASH_SR_BSY) {__NOP();};

    for (Address = Start; Address < Start + Length; Address += 2) {
        tVal = *(__IO uint16_t*)Address;
        *(__IO uint16_t*)CurFWAddress  = tVal;
        while(FLASH->SR & FLASH_SR_BSY) {__NOP();};
        CurFWAddress += 2;
    }
    FLASH->CR &= ~FLASH_CR_PG;     //This bit must be cleared. In other cases the flash will not work
    FLASH->CR |= FLASH_CR_LOCK; /* Lock the flash back */
}

uint32_t Check_CRC32(uint32_t Start, uint32_t Length) {
    uint32_t Address = 0;
    RCC->AHBENR |= RCC_AHBENR_CRCEN;
    CRC->CR = CRC_CR_RESET;
    
    for (Address = Start; Address < Start + Length; Address += 4) {
        CRC->DR = *(__IO uint32_t *)Address;
    }
    return CRC->DR;
}

int main(void)
{
//    volatile uint32_t NewFlashSize_ = *(__IO uint32_t *)NEW_FW_SETTINGS_ADDRESS;
//    volatile uint32_t NewFlashCRC32 = *(__IO uint32_t *)(NEW_FW_SETTINGS_ADDRESS + 4);
//    volatile uint32_t NewFWStartAddress = NEW_FW_START_ADDRESS | NEW_FW_OFFSET_ADDRESS;// NEW_FW_START_ADDRESS | (*(__IO uint32_t *)(NEW_FW_SETTINGS_ADDRESS + 8));
    
    __disable_irq();
    
#if ENABLE_LED == 1
    /* PB12 - LED. Output PP */
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    GPIOB->CRH |= GPIO_CRH_MODE12_0;
    GPIOB->CRH &= ~GPIO_CRH_CNF12;
    GPIOB->ODR ^= GPIO_ODR_ODR12;
#endif
    
    if (NEW_FW_SETTINGS_ADDRESS_VAL != 0xffffffff && NEW_FW_SETTINGS_ADDRESS_VAL_4 != 0xffffffff) {
        if (Check_CRC32(NEW_FW_START_OFFSET, NEW_FW_SETTINGS_ADDRESS_VAL) == NEW_FW_SETTINGS_ADDRESS_VAL_4) {
#if ENABLE_LED == 1
            GPIOB->ODR ^= GPIO_ODR_ODR12;
#endif
            FLASH_Erase(MAIN_PROGRAM_START_ADDRESS, 30);    //Erase 30 pages
#if ENABLE_LED == 1
            GPIOB->ODR ^= GPIO_ODR_ODR12;
#endif
            FLASH_Copy(NEW_FW_START_OFFSET, NEW_FW_SETTINGS_ADDRESS_VAL);
#if ENABLE_LED == 1
            GPIOB->ODR ^= GPIO_ODR_ODR12;
#endif
            if (Check_CRC32(MAIN_PROGRAM_START_ADDRESS, NEW_FW_SETTINGS_ADDRESS_VAL) == NEW_FW_SETTINGS_ADDRESS_VAL_4) {
                //Seems all is ok, erase FW at NewFWStartAddress and settings page
                FLASH_Erase(NEW_FW_START_OFFSET, 31);    //Erase 31 pages (include config)
            } else {
                //CRC not valid, blink forever
#if BLINK_ON_CRCERR == 1 && ENABLE_LED != 1
                /* PB12 - LED. Output PP */
                RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
                GPIOB->CRH |= GPIO_CRH_MODE12_0;
                GPIOB->CRH &= ~GPIO_CRH_CNF12;
                GPIOB->ODR ^= GPIO_ODR_ODR12;
#endif
                while(1) {
#if BLINK_ON_CRCERR == 1
                    GPIOB->ODR ^= GPIO_ODR_ODR12;
                    for (uint16_t delay = 0; delay < 0xFFFF; delay++) {
                        __NOP();
                    }
#else
                    __NOP();
#endif
                }
            }
        }
    }

    //Jump to main app
    typedef void (*pFunction)(void);
    pFunction Jump_To_Application;
    uint32_t  jumpAddress;

    jumpAddress         = *(__IO uint32_t *)(MAIN_PROGRAM_START_ADDRESS + 4);
    Jump_To_Application = (pFunction)jumpAddress;
    __set_MSP(*(__IO uint32_t *)MAIN_PROGRAM_START_ADDRESS);

    Jump_To_Application();

    while (1)
    {
    }
}
