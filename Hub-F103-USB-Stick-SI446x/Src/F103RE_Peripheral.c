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

#include "F103RE_Peripheral.h"

uint8_t  Usart1Buf[USART_BUF_LEN];
uint8_t  Usart2Buf[USART_BUF_LEN];
uint32_t counter = 0;

/**
  * @brief  Setup the RCC.
  * @param  None
  * @retval None
  */

void RCC_Configure(void)
{
    RCC->CR |= ((uint32_t)RCC_CR_HSEON);            //Enable HSE
    while (!(RCC->CR & RCC_CR_HSERDY)) {__NOP();};  //Wait till HSE ready
    RCC->CFGR &= ~RCC_CFGR_SW;                      //Change System Clock to HSI
    while ((RCC->CFGR & RCC_CFGR_SWS) != 0x00) {__NOP();};      //Wait
    RCC->CR &= ~RCC_CR_PLLON;                       //Disable Pll
    while ((RCC->CR & RCC_CR_PLLON)) {__NOP();};    //Wait
    RCC->CFGR &= ~0x3C0000;                         //Clear bits related to 
    RCC->CFGR |= RCC_CFGR_PLLMULL6;                 //Set Pll Mul to 6
//    RCC->CFGR |= RCC_CFGR_USBPRE;
    RCC->CFGR |= RCC_CFGR_PLLSRC;                   //Select source as PLL
    RCC->CR |= RCC_CR_PLLON;                        //Enable PLL 
    while (!(RCC->CR & RCC_CR_PLLON)) {__NOP();};   //Wait
    RCC->CFGR |= RCC_CFGR_SW_1;                     //Change System Clock to PLL
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_1) {__NOP();};    //Wait
//    RCC->CR &= ~RCC_CR_HSEON;                     //Disable HSE
}

void CRC_Configure(void)
{
    RCC->AHBENR |= RCC_AHBENR_CRCEN;                //Enable CRC clock
    CRC->CR = CRC_CR_RESET;                         //Reset CRC register
}

void DWT_Configure(void) 
{
  if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) //If DWT available
  {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  }
}

/**
  * @brief  Setup the System.
  * @param  None
  * @retval None
  */

void System_Configure(void)
{
    SysTick_Config(72000000 / 1000000);
    SysTick->CTRL |= ((uint32_t)0x00000004);
    NVIC_SetPriority(SysTick_IRQn, 5);
}

/**
  * @brief  Setup the RTC.
  * @param  None
  * @retval None
  */

void RTC_Configure(void)
{
}

/**
  * @brief  Setup the RTC Time.
  * @param  hh:mm:ss
  * @retval None
  */

void RTC_Time_Configure(uint8_t hh, uint8_t mm, uint8_t ss)
{
}

/**
  * @brief  Setup the RTC Alarm.
  * @param  hh:mm:ss (use 0xFF to ignore value)
  * @retval None
  */

void RTC_Alarm_Configure(uint8_t hh, uint8_t mm, uint8_t ss)
{
}

/**
  * @brief  Setup the RTC Alarm.
  * @param  None
  * @retval None
  */

void RTC_WakeUp_Configure(uint16_t Period)
{
}

/**
  * @brief  Setup the USART.
  *         TODO: Implement the PIN choose
  * @param  None
  * @retval None
  */

void USART_Configure(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;   //Enable all necessary clocks
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    /* USART1 */

    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;         //Enable USART1
    // USART1 to PA9/PA10
//    AFIO->MAPR |= AFIO_MAPR_USART1_REMAP;                                         //Remap pins
    GPIOA->CRH &= ~(GPIO_CRH_CNF9 | GPIO_CRH_CNF10 | GPIO_CRH_MODE10);              //Clear all flags
    GPIOA->CRH |= GPIO_CRH_CNF9_1;                                                  //PB6 - TX, Output AF, PP
    GPIOA->CRH |= GPIO_CRH_MODE9;                                                   //Max. output speed 50 MHz
    GPIOA->CRH |= GPIO_CRH_CNF10_0;                                                 //PB7 - RX, Input floating
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_IDLEIE | USART_CR1_TCIE; //Enable IRQs
    USART1->BRR = UART_BRR_SAMPLING16(APB2CLK, BAUDRATE);                           //Set baudrate
    USART1->CR3 |= USART_CR3_DMAR;                                                  //Enable DMA Rx
    //Configure DMA Channel 5 for USART1_RX
    DMA1_Channel5->CPAR  = (uint32_t)&USART1->DR; //Source (USART->DR Peripheral)
    DMA1_Channel5->CMAR  = (uint32_t)&Usart1Buf;  //Dest (memory)
    DMA1_Channel5->CNDTR = USART_BUF_LEN;         //Max buf length
    
    DMA1_Channel5->CCR &= ~DMA_CCR1_DIR;          //Read from peripheral
    DMA1_Channel5->CCR |= DMA_CCR1_MINC;          //Increment memory
    DMA1_Channel5->CCR |= DMA_CCR1_CIRC;          //Circular mode
    DMA1_Channel5->CCR |= DMA_CCR1_EN;            //Enable Channel

    USART1->CR3 |= USART_CR3_DMAT; //Enable DMA Tx
    //Configure DMA Channel 4 for USART1_TX
    DMA1_Channel4->CPAR = (uint32_t)&USART1->DR; //Dest (USART->DR Peripheral)
    DMA1_Channel4->CCR |= DMA_CCR1_MINC;         //Increment memory
    DMA1_Channel4->CCR |= DMA_CCR1_DIR;          //Send to peripheral

    USART1->CR1 |= USART_CR1_UE;                 //Enable USART1

    NVIC_SetPriority(USART1_IRQn, 5);            //Set IRQ priorities
    NVIC_EnableIRQ(USART1_IRQn);                 //Enable IRQ

    /* USART2 */
/*
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;                                           //Enable USART2
    GPIOA->CRL &= ~(GPIO_CRL_CNF2 | GPIO_CRL_CNF3 | GPIO_CRL_MODE3);                //Clear all flags
    GPIOA->CRL |= GPIO_CRL_CNF2_1;                                                  //PA2 - TX, Output AF, PP
    GPIOA->CRL |= GPIO_CRL_MODE2;                                                   //Max. output speed 50 MHz
    GPIOA->CRL |= GPIO_CRL_CNF3_0;                                                  //PA3 - RX, Input floating
    USART2->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_IDLEIE | USART_CR1_TCIE; //Enable IRQs
    USART2->BRR = UART_BRR_SAMPLING16(APB1CLK, BAUDRATE);                        //Set baudrate
    USART2->CR3 |= USART_CR3_DMAR;                                                  //Enable DMA Rx
    //Configure DMA Channel 5 for USART2_RX
    DMA1_Channel6->CPAR = (uint32_t)&USART2->DR; //Source (USART->DR Peripheral)
    DMA1_Channel6->CMAR = (uint32_t)&Usart2Buf;  //Dest (memory)
    DMA1_Channel6->CNDTR = USART_BUF_LEN;        //Max buf length

    DMA1_Channel6->CCR &= ~DMA_CCR1_DIR;         //Read from peripheral
    DMA1_Channel6->CCR |= DMA_CCR1_MINC;         //Increment memory
    DMA1_Channel6->CCR |= DMA_CCR1_CIRC;         //Circular mode
    DMA1_Channel6->CCR |= DMA_CCR1_EN;           //Enable Channel

    USART2->CR3 |= USART_CR3_DMAT; //Enable DMA Tx
    //Configure DMA Channel 7 for USART2_TX
    DMA1_Channel7->CPAR = (uint32_t)&USART2->DR; //Dest (USART->DR Peripheral)
    DMA1_Channel7->CCR |= DMA_CCR1_MINC;         //Increment memory
    DMA1_Channel7->CCR |= DMA_CCR1_DIR;          //Send to peripheral

    USART2->CR1 |= USART_CR1_UE;                 //Enable USART2

    NVIC_SetPriority(USART2_IRQn, 5);            //Set IRQ priorities
    NVIC_EnableIRQ(USART2_IRQn);                 //Enable IRQ
    */
}

/**
  * @brief  Setup the SPI.
  * @param  None
  * @retval None
  */

void SPI_Configure(void)
{

    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; //Enable SPI clock
    SPI1->CR1 |= SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_BR_2;            //Select the baud rate according to RM table
    SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;              //Select master, Software slave management and Internal slave select
    /*    SPI1->CR2 |=
        SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2 | SPI_CR2_NSSP | SPI_CR2_FRXTH;*/

    GPIOA->CRL &= ~(GPIO_CRL_CNF5 | GPIO_CRL_CNF7);  //Clear all flags
    GPIOA->CRL |= GPIO_CRL_CNF5_1;                   //PA5 - SCK, Output AF, PP
    GPIOA->CRL |= GPIO_CRL_MODE5;                    //Max. output speed 50 MHz
    GPIOA->CRL |= GPIO_CRL_CNF6_0;                   //PA6 - MISO, Input floating
    GPIOA->CRL |= GPIO_CRL_CNF7_1;                   //PA7 - MOSI, Output AF, PP
    GPIOA->CRL |= GPIO_CRL_MODE7;                    //Max. output speed 50 MHz

    SPI1->CR1 |= SPI_CR1_SPE;                        //Enable SPI
}

/**
  * @brief  Setup the GPIO.
  * @param  None
  * @retval None
  */

void GPIO_Configure(void)
{
    /* Enable Clock */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN;

    /* PB12 - LED. Output PP */
    GPIOB->CRH |= GPIO_CRH_MODE12_0;
    GPIOB->CRH &= ~GPIO_CRH_CNF12;
    
    /* PB13 - USB Enable. Output PP */
    GPIOB->CRH |= GPIO_CRH_MODE13_0;
    GPIOB->CRH &= ~GPIO_CRH_CNF13;

    /* PC13 - NSEL(CSN). Output PP */
    GPIOC->BSRR = GPIO_BSRR_BS13;
    GPIOC->CRH |= GPIO_CRH_MODE13_0;
    GPIOC->CRH &= ~GPIO_CRH_CNF13;
    
    /* PA3 - CTS. Input */
    // Must be as reset value
    GPIOA->CRL &= ~GPIO_CRL_MODE3_0;
    GPIOA->CRL &= ~GPIO_CRL_CNF3;
    GPIOA->CRL |= GPIO_CRL_CNF3_0;
    
    /* PA4 - nIRQ. Input */
    // Must be as reset value
    GPIOA->CRL &= ~GPIO_CRL_MODE4_0;
    GPIOA->CRL &= ~GPIO_CRL_CNF4;
    GPIOA->CRL |= GPIO_CRL_CNF4_0;
}

void EXTI_Configure(void)
{
    /* PA2 - IRQ, EXTI, Fall*/
/*    GPIOA->CRL &= ~GPIO_CRL_MODE2_0;
    EXTI->IMR |= EXTI_IMR_MR2;
    EXTI->FTSR |= EXTI_FTSR_TR2;
    AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI2_PA;

    NVIC_SetPriority(EXTI2_IRQn, 5);
    NVIC_EnableIRQ(EXTI2_IRQn);
*/
    
    /* PA4 - IRQ, EXTI, Fall*/
    GPIOA->CRL &= ~GPIO_CRL_MODE4_0;
    EXTI->IMR |= EXTI_IMR_MR4;
    EXTI->FTSR |= EXTI_FTSR_TR4;
    AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI4_PA;

    NVIC_SetPriority(EXTI4_IRQn, 5);
    NVIC_EnableIRQ(EXTI4_IRQn);
}

void WWDG_Configure(void)
{
    WWDG->CR = WWDG_CR_WDGA | WWDG_CR_T; //Start WWDG
}

void WWDG_Update(void)
{
    WWDG->CR |= WWDG_CR_T; //Update WWDG
}

/**
  * @brief  Setup the TIM.
  * @param  None
  * @retval None
  */

void TIM_Configure(void)
{
    /* Enable Clock */
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    
    TIM1->PSC = 9000 - 1;                   //Set prescaler
    TIM1->ARR = 8000 - 1;                   //Set auto reload register
    TIM1->DIER |= TIM_DIER_UIE;             //Enable IRQ by update
    TIM1->CR1 = TIM_CR1_CEN | TIM_CR1_ARPE; //Enable timer and set TIMx_ARR register as buffered
    NVIC_SetPriority(TIM1_UP_IRQn, 15);     //Ser priority
    NVIC_EnableIRQ(TIM1_UP_IRQn);           //Enable IRQ
    
    TIM2->PSC = 900 - 1;
    TIM2->ARR = 800 - 1;
    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->CR1 = TIM_CR1_ARPE;
    NVIC_SetPriority(TIM2_IRQn, 15);
    NVIC_EnableIRQ(TIM2_IRQn);
}

uint32_t DWT_Get(void)
{
  return DWT->CYCCNT;
}

void SPI_SendData(SPI_TypeDef *SPI, uint8_t *Data, uint16_t Length)
{
    uint16_t Timeout = 1000;
    uint16_t  Pos     = 0;

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
    while ((SPI->SR & SPI_SR_BSY) != 0)
    {
        __NOP();
    };
}

void SPI_ReadData(SPI_TypeDef *SPI, uint8_t *Data, uint16_t Length, uint8_t Dummy)
{
    while (SPI->SR & SPI_SR_RXNE)       //Clear shift register for some reason
    {
        (void)SPI->DR;
    };
    while (Length > 0)
    {
        while (!SPI->SR & SPI_SR_TXE)   //Wait until PREVIOUS data clocked to MOSI, 
        {                               //in general TXE should be 1 all time here, but.. 
            __NOP();
        }

        *((__IO uint8_t *)&SPI->DR) = Dummy;

        while (!(SPI->SR & SPI_SR_TXE)) //Again, wait until data clocked to MOSI
        {
            __NOP();
        };
        while (!(SPI->SR & SPI_SR_RXNE)) //And wait when data from MISO will be ready
        {
            __NOP();
        };
        (*Data++) = *(__IO uint8_t *)&SPI->DR;  //Read new data

        Length--;
    }
    while ((SPI->SR & SPI_SR_BSY) != 0)  //Wait until SPI will be ready
    {
        __NOP();
    };
}

void USART_SendChar(USART_TypeDef *USART, unsigned char ch)
{
    while (!(USART->SR & USART_SR_TXE))
    {
        __NOP();
    };
    USART->DR = ch;
/*    while (!(USART->SR & USART_SR_TC))
    {
        __NOP();
    };*/
}

void USART_SendData(USART_TypeDef *USART, uint8_t *Data, uint16_t Length)
{
    do
    {
        while (!(USART->SR & USART_SR_TXE))
        {
            __NOP();
        };
        USART->DR = *Data;
        Data++;
        Length--;
    } while (Length > 0);
    
    while (!(USART->SR & USART_SR_TC))
    {
        __NOP();
    };
}

void USART_DMASendData(USART_TypeDef *USART, char *Data, uint16_t Length)
{
    if (USART == USART1)
    {
        DMA1_Channel4->CCR &= ~DMA_CCR1_EN;    //Disable Channel
        DMA1_Channel4->CMAR  = (uint32_t)Data; //Source (memory)
        DMA1_Channel4->CNDTR = Length;         //Buf length
        DMA1_Channel4->CCR |= DMA_CCR1_EN;     //Enable Channel
    }else if (USART == USART2)
    {
        DMA1_Channel7->CCR &= ~DMA_CCR1_EN;    //Disable Channel
        DMA1_Channel7->CMAR  = (uint32_t)Data; //Source (memory)
        DMA1_Channel7->CNDTR = Length;         //Buf length
        DMA1_Channel7->CCR |= DMA_CCR1_EN;     //Enable Channel
    }
}

void FLASH_Erase(uint32_t EraseAddress, uint8_t Pages)
{
    FLASH->KEYR = FLASH_FKEY1;
    FLASH->KEYR = FLASH_FKEY2;


    while (FLASH->SR & FLASH_SR_BSY)
    {
        __NOP();
    };                         // Wait untill memory ready for erase
    for (uint8_t Page = 0; Page < Pages; Page++) {
        FLASH->CR |= FLASH_CR_PER; // Erase one page
        FLASH->AR = EraseAddress + (Page * 1024); // Erase address
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

void FLASH_WriteData(uint32_t fAddress, uint8_t *Data, int Size,
                     uint32_t EraseAddress)
{
    FLASH->KEYR = FLASH_FKEY1;
    FLASH->KEYR = FLASH_FKEY2;

    if (EraseAddress != 0)
    {
        while (FLASH->SR & FLASH_SR_BSY)
        {
            __NOP();
        };                         // Wait untill memory ready for erase
        FLASH->CR |= FLASH_CR_PER; // Erase one page
        FLASH->AR |= EraseAddress; // Erase address
        FLASH->CR |= FLASH_CR_STRT;
    }

    while (FLASH->SR & FLASH_SR_BSY)// Wait untill memory ready
    {
        __NOP();
    }; 
    FLASH->CR &= ~FLASH_CR_PER;     //This bit must be cleared. In other cases the flash will not work
    while (FLASH->SR & FLASH_SR_BSY)
    {
        __NOP();
    };
    FLASH->CR |= FLASH_CR_PG; // Allow flash programming
    while (FLASH->SR & FLASH_SR_BSY)
    {
        __NOP();
    };
    for (int i = 0; i < 10; i += 2)
    {
        *(__IO uint16_t *)fAddress =
            ((uint16_t)Data[i] << 8) | (uint16_t)Data[i + 1];
        fAddress += 2;
    }
    while (FLASH->SR & FLASH_SR_BSY)
    {
        __NOP();
    };
    FLASH->CR &= ~(FLASH_CR_PG);
}

void GOTO_Sleep(void)
{
    /*    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    dxputs("Going to sleep...");
    __WFI();
    dxputs("zzz....");*/
}

void USART1_IRQHandler(void)
{
    uint32_t SR         = USART1->SR;
    uint8_t  dataLength = 0;
    if (SR & USART_SR_IDLE)
    {
        (void)USART1->DR;                                           //to Clear IDLE bit
        dataLength = USART_BUF_LEN - (uint8_t)DMA1_Channel5->CNDTR; //Calcualte data length
        DMA1_Channel5->CCR &= ~DMA_CCR1_EN;                         //Disable Channel
        DMA1_Channel5->CNDTR = USART_BUF_LEN;                       //Reset Max buf length
        DMA1_Channel5->CCR |= DMA_CCR1_EN;                          //Enable Channel
        uUSART_IRQHandler(USART1, SR, Usart1Buf, dataLength);       //Call user USART handler
    }
    else if (SR & USART_SR_TC)
    {
        USART1->SR &= ~USART_SR_TC;          //Clear TC bit
        uUSART_IRQHandler(USART1, SR, 0, 0); //Call user USART handler
    }
}

void USART2_IRQHandler(void)
{
    uint32_t SR         = USART2->SR;
    uint8_t  dataLength = 0;
    if (SR & USART_SR_IDLE)
    {
        (void)USART2->DR;                                           //to Clear IDLE bit
        dataLength = USART_BUF_LEN - (uint8_t)DMA1_Channel6->CNDTR; //Calcualte data length
        DMA1_Channel6->CCR &= ~DMA_CCR1_EN;                         //Disable Channel
        DMA1_Channel6->CNDTR = USART_BUF_LEN;                       //Reset Max buf length
        DMA1_Channel6->CCR |= DMA_CCR1_EN;                          //Enable Channel
        uUSART_IRQHandler(USART2, SR, Usart2Buf, dataLength);       //Call user USART handler
    }
    else if (SR & USART_SR_TC)
    {
        USART2->SR &= ~USART_SR_TC;          //Clear TC bit
        uUSART_IRQHandler(USART2, SR, 0, 0); //Call user USART handler
    }
}

void EXTI15_10_IRQHandler(void)
{
    uint32_t PR = EXTI->PR;
    EXTI->PR |= PR;
    uEXTI_IRQHandler(PR);
}

void EXTI0_1_IRQHandler(void)
{
    uint32_t PR = EXTI->PR;
    EXTI->PR |= PR;
    uEXTI_IRQHandler(PR);
}

void EXTI4_IRQHandler(void)
{
    uint32_t PR = EXTI->PR;
    EXTI->PR |= PR;
    uEXTI_IRQHandler(PR);
}

void EXTI3_IRQHandler(void)
{
    uint32_t PR = EXTI->PR;
    EXTI->PR |= PR;
    uEXTI_IRQHandler(PR);
}

void EXTI2_IRQHandler(void)
{
    uint32_t PR = EXTI->PR;
    EXTI->PR |= PR;
//    uEXTI_IRQHandler(PR);
}

void TIM1_UP_IRQHandler() {
    TIM1->SR &= ~TIM_SR_UIF;

    uTIM_IRQHandler(TIM1);
}

void TIM2_IRQHandler(void)
{
    TIM2->SR &= ~TIM_SR_UIF;
    uTIM_IRQHandler(TIM2);
}

void TIM14_IRQHandler(void)
{
    /*    TIM14->SR &= ~TIM_SR_UIF;
    uTIM_IRQHandler();*/
}

void RTC_IRQHandler(void)
{
    /*    uint32_t RTC_ISR = RTC->ISR;
    if (RTC->ISR & RTC_ISR_ALRAF)
    {
        RTC->ISR &= ~RTC_ISR_ALRAF;
    }
    if (EXTI->PR & EXTI_PR_PR17)
    {
        EXTI->PR |= EXTI_PR_PR17;
    }
    uRTC_IRQHandler(RTC_ISR);*/
}

void Delay_ms(uint32_t Delay) {
    /* Hmm. Looks like we do not need delay at all ;) */
}

void Delay_us(uint32_t Delay) {
    int32_t tp = DWT_Get() + Delay * (SYS_FREQUENCY/1000000);
    while (((int32_t)DWT_Get() - tp) < 0);
}

__weak void uEXTI_IRQHandler(uint32_t Pin)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(Pin);
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

__weak void uUSART_IRQHandler(USART_TypeDef *USART, uint32_t SR, uint8_t *Data, uint8_t Length)
{
    /* NOTE: This function Should not be modified, when the callback is needed,
           the uRTC_IRQHandler could be implemented in the user file
   */
}
#endif /* __Peripheral_H */
