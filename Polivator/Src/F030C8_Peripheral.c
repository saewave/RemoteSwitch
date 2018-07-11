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
 
#ifndef __Peripheral_H
#define __Peripheral_H

#include "F030C8_Peripheral.h"
#define SYSTEM_CORE_CLOCK 8000000UL
#define APBCLK 8000000UL
#define BAUDRATE 115200UL
uint32_t FWInitialAddress = SAE_FW_MEMORY_ADDRESS;
int32_t  tp               = 0;
uint16_t ADCCalVal        = 0;

/**
  * @brief  Setup the System.
  * @param  None
  * @retval None
  */

void System_Configure(void)
{
    SysTick_Config(SYSTEM_CORE_CLOCK / 1000000);
    SysTick->CTRL |= ((uint32_t)0x00000004);
    NVIC_SetPriority(SysTick_IRQn, 0);
}

void RCC_Configure(void)
{
    RCC->CFGR &= ~RCC_CFGR_SW; //Change System Clock to HSI
    while ((RCC->CFGR & RCC_CFGR_SWS) != 0x00) {
        __NOP();
    };
    RCC->CR &= ~RCC_CR_PLLON; //Disable Pll
    while ((RCC->CR & RCC_CR_PLLON)) {
        __NOP();
    };
    RCC->CFGR &= ~RCC_CFGR_PLLSRC;   //Set Pll Source to HSI
    RCC->CFGR |= RCC_CFGR_PLLMULL16; //Set Pll Mul to 16
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLON)) {
        __NOP();
    };
    RCC->CFGR |= RCC_CFGR_SW_1; //Change System Clock to HSI
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_1) {
        __NOP();
    };
    RCC->CR &= ~RCC_CR_HSEON; //Disable HSE
}

void ADC_Configure(void)
{
    RCC->AHBENR |= RCC_AHBENR_DMA1EN | RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    GPIOA->MODER |= GPIO_MODER_MODER1 | GPIO_MODER_MODER2 | GPIO_MODER_MODER3 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6 | GPIO_MODER_MODER7;

    ADC1->ISR &= ~ADC_ISR_ADRDY;
    //    ADC1->CR |= ADC_CR_ADEN;
    //    while(!(ADC1->ISR & ADC_ISR_ADRDY)){};
    ADC1->CHSELR |= ADC_CHSELR_CHSEL1 | ADC_CHSELR_CHSEL2 | ADC_CHSELR_CHSEL3; // | ADC_CHSELR_CHSEL4 | ADC_CHSELR_CHSEL5 | ADC_CHSELR_CHSEL6 | ADC_CHSELR_CHSEL7 | ADC_CHSELR_CHSEL8;
                                                                               //    ADC1->CFGR1 |= ADC_CFGR1_SCANDIR;     //Direction 18->0
    ADC1->SMPR |= ADC_SMPR1_SMPR;
    ADC1->CFGR1 |= ADC_CFGR1_CONT;
    ADC1->IER |= ADC_IER_EOSEQIE;
    //    ADC1->CR |= ADC_CR_ADCAL;
    //    while(!(ADC1->CR & ADC_CR_ADCAL)) {__NOP();};
    //    ADCCalVal = (uint16_t)ADC1->DR;

    //    ADC1->CR |= ADC_CR_

    DMA1_Channel1->CCR |= DMA_CCR_MSIZE_0; //16 bit
    DMA1_Channel1->CCR |= DMA_CCR_PSIZE_0; //16 bit
    DMA1_Channel1->CCR |= DMA_CCR_MINC;    //mem inc
    DMA1_Channel1->CCR |= DMA_CCR_CIRC;    //circular
    DMA1_Channel1->CCR &= ~DMA_CCR_DIR;
    //    DMA1_Channel1->CNDTR = ADC_CH_LEN * ADC_CH_CNT;

    //    DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;
    //    DMA1_Channel1->CMAR = (uint32_t)&ADCBuf;

    //DMA1_Channel1->CCR |= DMA_CCR_EN;           //enable
    //ADC1->CR |= ADC_CR_ADSTART;     //Start;
}

void ADC_Start(uint32_t *Buf, uint16_t Length)
{
    if (!(ADC1->CR & ADC_CR_ADSTART)) {
        ADC1->CFGR1 &= ~(ADC_CFGR1_DMAEN | ADC_CFGR1_DMACFG); //Disable DMA
        ADC1->CR |= ADC_CR_ADCAL;
        while (!(ADC1->CR & ADC_CR_ADCAL)) {
            __NOP();
        };
        ADCCalVal = (uint16_t)ADC1->DR;

        ADC1->CFGR1 |= ADC_CFGR1_DMAEN | ADC_CFGR1_DMACFG;
        DMA1_Channel1->CNDTR = Length;
        DMA1_Channel1->CMAR  = (uint32_t)Buf;
        DMA1_Channel1->CPAR  = (uint32_t)&ADC1->DR;
        DMA1_Channel1->CCR |= DMA_CCR_EN; //enable

        ADC1->CR |= ADC_CR_ADEN; //Start;
        ADC1->CR |= ADC_CR_ADSTART;
    }
}

void ADC_Stop(void)
{
    if (ADC1->CR & ADC_CR_ADSTART) {
        DMA1_Channel1->CCR &= ~DMA_CCR_EN; //disable
        ADC1->CR |= ADC_CR_ADSTP;          //Stop;
        while (ADC1->CR & ADC_CR_ADSTP) {
            __NOP();
        };
        ADC1->CR |= ADC_CR_ADDIS;
        while (ADC1->CR & ADC_CR_ADDIS) {
            __NOP();
        };
    }
}

void CRC_Configure(void)
{
    RCC->AHBENR |= RCC_AHBENR_CRCEN;
    CRC->CR = CRC_CR_RESET;
}

/**
  * @brief  Setup the RTC.
  * @param  None
  * @retval None
  */

void RTC_Configure(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_DBP;

    RTC->PRER |= RTC_PRER_PREDIV_S;
    RCC->BDCR |= RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_LSI;
    RCC->CSR |= RCC_CSR_LSION;
    while (!(RCC->CSR & RCC_CSR_LSIRDY)) {
        __NOP();
    };
}

/**
  * @brief  Setup the RTC Time.
  * @param  hh:mm:ss
  * @retval None
  */

void RTC_Time_Configure(uint8_t hh, uint8_t mm, uint8_t ss)
{
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    RTC->ISR |= RTC_ISR_INIT;
    while (!(RTC->ISR & RTC_ISR_INITF)) {
        __NOP();
    };
    RTC->TR = ((((hh / 10) << 4) | (hh % 10)) << 16) | ((((mm / 10) << 4) | (mm % 10)) << 8) | (((ss / 10) << 4) | (ss % 10));

    RTC->ISR &= ~RTC_ISR_INIT;
    RTC->WPR = 0xFF;
}

/**
  * @brief  Setup the RTC Alarm.
  * @param  hh:mm:ss (use 0xFF to ignore value)
  * @retval None
  */

void RTC_Alarm_Configure(uint8_t hh, uint8_t mm, uint8_t ss)
{
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
    RTC->CR &= ~RTC_CR_ALRAE; //Disable alarm
    RTC->CR |= RTC_CR_BKP;
    while (!(RTC->ISR & RTC_ISR_ALRAWF)) {
        __NOP();
    };

    RTC->ALRMAR |= RTC_ALRMAR_MSK4;
    uint32_t ALRMAR = 0;
    if (ss != 0xFF) {
        ALRMAR |= ((ss / 10) << 4) | (ss % 10);
        RTC->ALRMAR &= ~RTC_ALRMAR_MSK1;
    } else {
        RTC->ALRMAR |= RTC_ALRMAR_MSK1;
    }
    if (mm != 0xFF) {
        ALRMAR |= (((mm / 10) << 4) | (mm % 10)) << 8;
        RTC->ALRMAR &= ~RTC_ALRMAR_MSK2;
    } else {
        RTC->ALRMAR |= RTC_ALRMAR_MSK2;
    }
    if (hh != 0xFF) {
        ALRMAR |= (((hh / 10) << 4) | (hh % 10)) << 8;
        RTC->ALRMAR &= ~RTC_ALRMAR_MSK3;
    } else {
        RTC->ALRMAR |= RTC_ALRMAR_MSK3;
    }
    RTC->ALRMAR |= ALRMAR;
    RTC->CR |= RTC_CR_ALRAE;  //Enable alarm
    RTC->CR |= RTC_CR_ALRAIE; //Enable alarm interrupt

    while (!(RTC->ISR & RTC_ISR_ALRAWF)) {
        __NOP();
    };

    RTC->WPR = 0xFF;

    EXTI->IMR |= EXTI_IMR_MR17;
    EXTI->RTSR |= EXTI_RTSR_TR17;
    NVIC_EnableIRQ(RTC_IRQn);
    NVIC_SetPriority(RTC_IRQn, 1);
}

/**
  * @brief  Setup the RTC Alarm.
  * @param  None
  * @retval None
  */

void RTC_WakeUp_Configure(uint16_t Period)
{
    RTC->WPR = 0xCA; //Disable write protection
    RTC->WPR = 0x53;

    RTC->CR &= ~RTC_CR_WUTE; //Disable wakeup timer
    while (!(RTC->ISR & RTC_ISR_WUTWF)) {
        __NOP();
    }; //Wait until it is set in RTC_ISR

    RTC->WUTR = (uint32_t)Period;

    RTC->CR |= RTC_CR_WUCKSEL_0;
    RTC->CR |= RTC_CR_WUTE | RTC_CR_WUTIE; //Enable wakeup timer
    RTC->CR |= RTC_CR_ALRAIE;              //Enable alarm interrupt

    RTC->WPR = 0xFF;

    EXTI->IMR |= EXTI_IMR_MR20;
    EXTI->RTSR |= EXTI_RTSR_TR20;
    NVIC_EnableIRQ(RTC_IRQn);
    NVIC_SetPriority(RTC_IRQn, 1);
}

/**
  * @brief  Setup the USART.
  *         TODO: Implement the PIN choose
  * @param  None
  * @retval None
  */

void USART_Configure(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    USART1->BRR = UART_BRR_SAMPLING16(APBCLK, BAUDRATE);
    USART1->CR3 |= USART_CR3_HDSEL;
    USART1->CR1 |= USART_CR1_TE;
    USART1->CR1 |= USART_CR1_UE;
}

/**
  * @brief  Setup the SPI.
  * @param  None
  * @retval None
  */

void SPI_Configure(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOFEN;

    // PB3 SPI_SCK, PB4 SPI_MISO, PB5 SPI_MOSI. AF, PP
    GPIOB->AFR[0] |= 0x0000;
    GPIOB->MODER |=
        GPIO_MODER_MODER3_1 | GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1;
    GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_3 | GPIO_OTYPER_OT_4 | GPIO_OTYPER_OT_5);
    GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR3 | GPIO_OSPEEDER_OSPEEDR4 | GPIO_OSPEEDER_OSPEEDR5;

    // PF6 CSN. Output PP
    GPIOF->BSRR = GPIO_BSRR_BS_6;
    GPIOF->MODER |= GPIO_MODER_MODER6_0;

    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    SPI1->CR1 |= SPI_CR1_BR_0 | SPI_CR1_BR_2;
    SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;
    SPI1->CR2 |=
        SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2 | SPI_CR2_FRXTH;

    SPI1->CR1 |= SPI_CR1_SPE;
}

/**
  * @brief  Setup the GPIO.
  * @param  None
  * @retval None
  */

void GPIO_Configure(void)
{
    /* Enable GPIOA, GPIOB, GPIOF Clock */
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOFEN;

    // PB12 LED. Output PP
    GPIOB->MODER |= GPIO_MODER_MODER12_0;

    // PA0 CH-ON/OFF. Output PP
    GPIOA->BSRR = GPIO_BSRR_BS_0;
    GPIOA->MODER |= GPIO_MODER_MODER0_0;

    // PA10-12 CH-OUT*. Output PP
    GPIOA->BSRR = GPIO_BSRR_BR_10 | GPIO_BSRR_BR_11 | GPIO_BSRR_BR_12;
    GPIOA->MODER |= GPIO_MODER_MODER10_0 | GPIO_MODER_MODER11_0 | GPIO_MODER_MODER12_0;

    
    // PB6 USART TX. AF, Output OD

    GPIOB->MODER |= GPIO_MODER_MODER6_1;
    GPIOB->OTYPER |= GPIO_OTYPER_OT_6;
    GPIOB->PUPDR |= GPIO_PUPDR_PUPDR6_0;
    //GPIOB->AFR[0] |= 0x0000;
/*
    // PA9 USART TX. AF, PP
//    GPIOA->MODER |= GPIO_MODER_MODER9_1;
//    GPIOA->OTYPER &= ~GPIO_OTYPER_OT_9;
//    GPIOA->PUPDR |= GPIO_PUPDR_PUPDR9_0;
//    GPIOA->AFR[1] |= (0x01 << 4);

    // PA3 1wire. Output OD
    GPIOA->MODER |= GPIO_MODER_MODER3_0;
    GPIOA->OTYPER |= GPIO_OTYPER_OT_3;
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR3;
*/
    // PF7 SI4463 IRQ. Input EXTI
    GPIOF->MODER &= ~GPIO_MODER_MODER7;
}

void EXTI_Configure(void)
{
    /* PF7 SI4463. Input EXTI */
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    EXTI->IMR |= EXTI_IMR_MR7;
    EXTI->FTSR |= EXTI_FTSR_TR7;
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI7_PF;

    NVIC_EnableIRQ(EXTI4_15_IRQn);
    NVIC_SetPriority(EXTI4_15_IRQn, 1);
}

/**
  * @brief  Setup the TIM.
  * @param  None
  * @retval None
  */

void TIM_Configure(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM17EN | RCC_APB2ENR_TIM16EN;

    /* Manage Process and Blink */
    TIM14->DIER |= TIM_DIER_UIE;
    TIM14->PSC = 1000;
    TIM14->ARR = 8000 - 1;
    //    TIM14->CR1 |= TIM_CR1_CEN;

    NVIC_EnableIRQ(TIM14_IRQn);
    NVIC_SetPriority(TIM14_IRQn, 2);

    /* Manage noting */
    TIM16->DIER |= TIM_DIER_UIE;
    TIM16->PSC = 1000;
    TIM16->ARR = 8000 - 1;
    //    TIM16->CR1 |= TIM_CR1_CEN;

    NVIC_EnableIRQ(TIM16_IRQn);
    NVIC_SetPriority(TIM16_IRQn, 3);

    /* Manage Delay_us() */
    //    TIM17->DIER |= TIM_DIER_UIE;
    TIM17->PSC = 0;
    TIM17->ARR = 0xFFFF;
    TIM17->CNT = 0;

    //    TIM17->CR1 |= TIM_CR1_CEN;
}

void SPI_SendData(SPI_TypeDef *SPI, uint8_t *Data, uint16_t Length)
{
    uint16_t Timeout = 1000;
    uint8_t  Pos     = 0;

    while (Length > Pos)
    {
        Timeout = 1000;
        *((__IO uint8_t *)&SPI->DR) = Data[Pos];
        Pos++;
        while (!(SPI->SR & SPI_SR_TXE)) /* Wait until TXE flag is set to send data */
        {
            Timeout--;
            if (Timeout == 0)
            {
                break;
            }
        }
    }
    while ((SPI->SR & SPI_SR_BSY) != 0) {
        __NOP();
    };
}
/*
void SPI_ReadRxFifoData(SPI_TypeDef *SPI, uint8_t *Data, uint16_t Length)
{
    uint16_t Timeout = 1000;

    do {
        if ((SPI->SR & SPI_SR_RXNE) == SPI_SR_RXNE) {
            (*Data++) = *(__IO uint8_t *)&SPI->DR;
            Length--;
        } else {
            // Timeout management
            Timeout--;
            if (Timeout == 0) {
                break;
            }
        }
    } while (Length > 0);

    while ((SPI->SR & SPI_SR_BSY) != 0) {
        __NOP();
    };
}
*/
void SPI_ReadData(SPI_TypeDef *SPI, uint8_t *Data, uint16_t Length, uint8_t Dummy)
{
    SPI_FlushRxFifo(SPI);

    while (Length > 0) {
        while (!SPI->SR & SPI_SR_TXE)
        {
            __NOP();
        }

        *((__IO uint8_t *)&SPI->DR) = Dummy;

        while (!(SPI->SR & SPI_SR_TXE))
        {
            __NOP();
        };
        while (!(SPI->SR & SPI_SR_RXNE))
        {
            __NOP();
        };
        (*Data++) = *(__IO uint8_t *)&SPI->DR;

        Length--;
    }
    while ((SPI->SR & SPI_SR_BSY) != 0) {
        __NOP();
    };
}

void SPI_FlushRxFifo(SPI_TypeDef *SPI)
{
    __IO uint32_t tmpreg;
    uint8_t       count = 0xFF;
    while (((SPI->SR & SPI_SR_FRLVL) != 0)) {
        count--;
        tmpreg = SPI->DR;
        UNUSED(tmpreg);
        if (count == 0) {
            break;
        }
    }
}

void USART_SendChar(unsigned char ch)
{
    while ((USART1->ISR & USART_ISR_TXE) == 0) {
        __NOP();
    };
    USART1->TDR = ch;
}

void USART_SendData(uint8_t *Data, uint16_t Length)
{
    do {
        while ((USART1->ISR & USART_ISR_TXE) == 0) {
            __NOP();
        };
        USART1->TDR = *Data;
        Data++;
        Length--;
    } while (Length > 0);
}

void FLASH_Erase(uint32_t EraseAddress, uint8_t Pages)
{
    FLASH->KEYR = FLASH_FKEY1;
    FLASH->KEYR = FLASH_FKEY2;

    while (FLASH->SR & FLASH_SR_BSY) {
        __NOP();
    }; // Wait untill memory ready for erase
    for (uint8_t Page = 0; Page <= Pages; Page++) {
        FLASH->CR |= FLASH_CR_PER;                 // Erase one page
        FLASH->AR |= EraseAddress + (Page * 1024); // Erase address
        FLASH->CR |= FLASH_CR_STRT;
        while (FLASH->SR & FLASH_SR_BSY) // Wait untill memory ready
        {
            __NOP();
        };
    }

    FLASH->CR &= ~FLASH_CR_PER; //This bit must be cleared. In other cases the flash will not work
    while (FLASH->SR & FLASH_SR_BSY) {
        __NOP();
    };
    FLASH->CR |= FLASH_CR_LOCK; /* Lock the flash back */
}

unsigned int htoi(uint8_t *ptr, uint8_t Length)
{
    unsigned int value = 0;
    char         ch    = *ptr;
    uint8_t      i     = 0;
    for (i = 0; i < Length; i++) {
        if (ch >= '0' && ch <= '9')
            value = (value << 4) + (ch - '0');
        else if (ch >= 'A' && ch <= 'F')
            value = (value << 4) + (ch - 'A' + 10);
        else if (ch >= 'a' && ch <= 'f')
            value = (value << 4) + (ch - 'a' + 10);
        else
            return value;
        ch = *(++ptr);
    }
    return value;
}

uint8_t FWWriteToFlash(uint8_t *cData)
{
    uint32_t Len   = htoi(cData, 2);
    uint32_t Addr  = htoi(&cData[2], 4);
    uint32_t Type  = htoi(&cData[6], 2);
    uint16_t HWord = 0;

    if (Len == 0x02 && Addr == 0 && Type == 0x04) { //Set Extended Linear Address Record
        FWInitialAddress = SAE_FW_MEMORY_ADDRESS | ((uint32_t)htoi(&cData[8], 4) << 16);

        if (FWInitialAddress >= SAE_FLASH_CFG_ADDR) {
            FWInitialAddress = SAE_FW_MEMORY_ADDRESS;
            return 11; //Given address is more than SAE_FLASH_CFG_ADDR
        }
    }

    if (Len > 0 && Type == 0) { //Write data to flash
        if (Len > 0x10) {
            return 12; //Length can't be more than 16 bytes
        }

        if ((FWInitialAddress | Addr) >= SAE_FLASH_CFG_ADDR) {
            return 11; //Given address is more than SAE_FLASH_CFG_ADDR
        }

        FLASH->KEYR = FLASH_FKEY1; // Unlock flash
        FLASH->KEYR = FLASH_FKEY2;
        FLASH->CR |= FLASH_CR_PG; // Allow write
        while (FLASH->SR & FLASH_SR_BSY) {
            __NOP();
        };
        for (uint8_t i = 0; i < Len; i = i + 2) {
            HWord                                             = htoi(&cData[8 + (i * 2)], 2) | (htoi(&cData[8 + (i * 2) + 2], 2) << 8);
            *(__IO uint16_t *)((FWInitialAddress | Addr) + i) = HWord;
            while (FLASH->SR & FLASH_SR_BSY) {
                __NOP();
            };
        }

        FLASH->CR |= FLASH_CR_LOCK;
    }

    return 0;
}

void FLASH_WriteData(uint32_t fAddress, uint8_t *Data, uint8_t Size,
                     uint32_t EraseAddress)
{
    FLASH->KEYR = FLASH_FKEY1;
    FLASH->KEYR = FLASH_FKEY2;

    if (EraseAddress != 0) {
        while (FLASH->SR & FLASH_SR_BSY) {
            __NOP();
        };                         // Wait untill memory ready for erase
        FLASH->CR |= FLASH_CR_PER; // Erase one page
        FLASH->AR |= EraseAddress; // Erase address
        FLASH->CR |= FLASH_CR_STRT;
    }

    while (FLASH->SR & FLASH_SR_BSY) {
        __NOP();
    }; // Wait untill memory ready
    FLASH->CR &= ~FLASH_CR_PER;
    while (FLASH->SR & FLASH_SR_BSY) {
        __NOP();
    };
    FLASH->CR |= FLASH_CR_PG; // Allow flash programming
    while (FLASH->SR & FLASH_SR_BSY) {
        __NOP();
    };
    for (int i = 0; i < 10; i += 2) {
        *(__IO uint16_t *)fAddress =
            ((uint16_t)Data[i] << 8) | (uint16_t)Data[i + 1];
        fAddress += 2;
    }
    while (FLASH->SR & FLASH_SR_BSY) {
        __NOP();
    };
    FLASH->CR &= ~(FLASH_CR_PG);
}

void GOTO_Sleep(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    //    dxputs("Going to sleep...");
    __WFI();
    //    dxputs("zzz....");
}

void GOTO_Stop(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
#if SAE_DEBUG_IN_STOP_MODE == 1
    //Before using DEBUG in Stop mode reduce the frequency in your debugger to lower than 500kHz
    DBGMCU->CR |= DBGMCU_CR_DBG_STOP;
#endif
    PWR->CR |= PWR_CR_LPDS;
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
    //    dxputs("Going to stop...");
    __WFI();
    //    dxputs("stop....\n");
}

void EXTI0_1_IRQHandler(void)
{
    if ((EXTI->IMR & EXTI_IMR_MR0) && (EXTI->PR & EXTI_PR_PR0)) {
        EXTI->PR |= EXTI_PR_PR0;
    }
    uEXTI_IRQHandler(EXTI->IMR);
}

void EXTI4_15_IRQHandler(void)
{
    if ((EXTI->IMR & EXTI_IMR_MR7) && (EXTI->PR & EXTI_PR_PR7)) {
        EXTI->PR |= EXTI_PR_PR7;
    }
    uEXTI_IRQHandler(EXTI->IMR);
}

void TIM14_IRQHandler(void)
{
    TIM14->SR &= ~TIM_SR_UIF;
    uTIM_IRQHandler(TIM14);
}

void TIM16_IRQHandler(void)
{
    TIM16->SR &= ~TIM_SR_UIF;
    uTIM_IRQHandler(TIM16);
}

void RTC_IRQHandler(void)
{
    uint32_t RTC_ISR = RTC->ISR;
    if (RTC->ISR & RTC_ISR_ALRAF) {
        RTC->ISR &= ~RTC_ISR_ALRAF;
    }
    if (EXTI->PR & EXTI_PR_PR17) {
        EXTI->PR |= EXTI_PR_PR17;
    }
    uRTC_IRQHandler(RTC_ISR);
}

void SysTick_Handler(void)
{
    // Not enabled by default
}

void Delay_us(uint32_t Delay)
{
    TIM17->CNT = 0;
    tp         = Delay * (SYS_FREQUENCY / 1000000);
    TIM17->CR1 |= TIM_CR1_CEN;
    while (((int32_t)TIM17->CNT - tp) < 0)
        ;
    TIM17->CR1 &= ~TIM_CR1_CEN;
}
__weak void uEXTI_IRQHandler(uint32_t Pin)
{
    /* Prevent unused argument(s) compilation warning */
    /* NOTE: This function Should not be modified, when the callback is needed,
           the uEXTI_IRQHandler could be implemented in the user file
   */
}

__weak void uTIM_IRQHandler(TIM_TypeDef *Tim)
{
    /* NOTE: This function Should not be modified, when the callback is needed,
           the uTIM_IRQHandler could be implemented in the user file
   */
}

__weak void uRTC_IRQHandler(uint32_t RTC_ISR)
{
    /* NOTE: This function Should not be modified, when the callback is needed,
           the uRTC_IRQHandler could be implemented in the user file
   */
}
#endif /* __Peripheral_H */
