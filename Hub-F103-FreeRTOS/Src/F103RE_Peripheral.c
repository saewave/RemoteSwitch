#ifndef __Peripheral_H
#define __Peripheral_H

#include "F103RE_Peripheral.h"

#define APBCLK 8000000UL
#define BAUDRATE 115200UL

#define USART_BUF_LEN 50
uint8_t  Usart1Buf[USART_BUF_LEN];
uint8_t  Usart2Buf[USART_BUF_LEN];
uint32_t counter = 0;
/**
  * @brief  Setup the System.
  * @param  None
  * @retval None
  */

void System_Configure(void)
{
    SysTick_Config(SystemCoreClock / 1000000);
    SysTick->CTRL |= ((uint32_t)0x00000004);
    NVIC_SetPriority(SysTick_IRQn, 0);
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
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    /* USART1 */

    RCC->APB2ENR |= RCC_APB2ENR_USART1EN; //Enable USART1
    // Remap USART1 to PB6/PB7
    AFIO->MAPR |= AFIO_MAPR_USART1_REMAP;                                           //Remap pins
    GPIOB->CRL &= ~(GPIO_CRL_CNF6 | GPIO_CRL_CNF7 | GPIO_CRL_MODE7);                //Clear all flags
    GPIOB->CRL |= GPIO_CRL_CNF6_1;                                                  //PB6 - TX, Output AF, PP
    GPIOB->CRL |= GPIO_CRL_MODE6;                                                   //Max. output speed 50 MHz
    GPIOB->CRL |= GPIO_CRL_CNF7_0;                                                  //PB7 - RX, Input floating
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_IDLEIE | USART_CR1_TCIE; //Enable IRQs
    USART1->BRR = (APBCLK + BAUDRATE / 2) / BAUDRATE;                               //Set baudrate
    USART1->CR3 |= USART_CR3_DMAR;                                                  //Enable DMA Rx
    //Configure DMA Channel 5 for USART1_RX
    DMA1_Channel5->CCR |= DMA_CCR1_MINC;          //Increment memory
    DMA1_Channel5->CCR |= DMA_CCR1_CIRC;          //Circular mode
    DMA1_Channel5->CPAR  = (uint32_t)&USART1->DR; //Source (USART->DR Peripheral)
    DMA1_Channel5->CMAR  = (uint32_t)&Usart1Buf;  //Dest (memory)
    DMA1_Channel5->CNDTR = USART_BUF_LEN;         //Max buf length
    DMA1_Channel5->CCR &= ~DMA_CCR1_DIR;          //Read from peripheral
    DMA1_Channel5->CCR |= DMA_CCR1_EN;            //Enable Channel

    USART1->CR3 |= USART_CR3_DMAT; //Enable DMA Tx
    //Configure DMA Channel 4 for USART1_TX
    DMA1_Channel4->CCR |= DMA_CCR1_MINC;         //Increment memory
    DMA1_Channel4->CPAR = (uint32_t)&USART1->DR; //Dest (USART->DR Peripheral)
    DMA1_Channel4->CCR |= DMA_CCR1_DIR;          //Send to peripheral

    USART1->CR1 |= USART_CR1_UE; //Enable usart

    NVIC_SetPriority(USART1_IRQn, 0);
    NVIC_EnableIRQ(USART1_IRQn);

    /* USART2 */
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;                                           //Enable USART2
    GPIOA->CRL &= ~(GPIO_CRL_CNF2 | GPIO_CRL_CNF3 | GPIO_CRL_MODE3);                //Clear all flags
    GPIOA->CRL |= GPIO_CRL_CNF2_1;                                                  //PA2 - TX, Output AF, PP
    GPIOA->CRL |= GPIO_CRL_MODE2;                                                   //Max. output speed 50 MHz
    GPIOA->CRL |= GPIO_CRL_CNF3_0;                                                  //PA3 - RX, Input floating
    USART2->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_IDLEIE | USART_CR1_TCIE; //Enable IRQs
    USART2->BRR = (APBCLK + BAUDRATE / 2) / BAUDRATE;                               //Set baudrate
    USART2->CR3 |= USART_CR3_DMAR;                                                  //Enable DMA Rx
    //Configure DMA Channel 5 for USART2_RX
    DMA1_Channel6->CCR |= DMA_CCR1_MINC;         //Increment memory
    DMA1_Channel6->CCR |= DMA_CCR1_CIRC;         //Circular mode
    DMA1_Channel6->CPAR = (uint32_t)&USART2->DR; //Source (USART->DR Peripheral)
    DMA1_Channel6->CMAR = (uint32_t)&Usart2Buf;  //Dest (memory)
    DMA1_Channel6->CCR &= ~DMA_CCR1_DIR;         //Read from peripheral
    DMA1_Channel6->CNDTR = USART_BUF_LEN;        //Max buf length
    DMA1_Channel6->CCR |= DMA_CCR1_EN;           //Enable Channel

    USART2->CR3 |= USART_CR3_DMAT; //Enable DMA Tx
    //Configure DMA Channel 7 for USART2_TX
    DMA1_Channel7->CCR |= DMA_CCR1_MINC;         //Increment memory
    DMA1_Channel7->CPAR = (uint32_t)&USART2->DR; //Dest (USART->DR Peripheral)
    DMA1_Channel7->CCR |= DMA_CCR1_DIR;          //Send to peripheral

    USART2->CR1 |= USART_CR1_UE; //Enable usart

    NVIC_SetPriority(USART2_IRQn, 5);
    NVIC_EnableIRQ(USART2_IRQn);
}

/**
  * @brief  Setup the SPI.
  * @param  None
  * @retval None
  */

void SPI_Configure(void)
{

    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN; //Enable SPI clock
    SPI2->CR1 |= SPI_CR1_BR;            //Select the baud rate
    SPI2->CR1 |= SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;
    /*    SPI1->CR2 |=
        SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2 | SPI_CR2_NSSP | SPI_CR2_FRXTH;*/

    GPIOB->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_CNF15); //Clear all flags
    GPIOB->CRH |= GPIO_CRH_CNF13_1;                   //PB13 - SCK, Output AF, PP
    GPIOB->CRH |= GPIO_CRH_MODE13;                    //Max. output speed 50 MHz
    GPIOB->CRH |= GPIO_CRH_CNF14_0;                   //PB14 - MISO, Input floating
    GPIOB->CRH |= GPIO_CRH_CNF15_1;                   //PB15 - MOSI, Output AF, PP
    GPIOB->CRH |= GPIO_CRH_MODE15;                    //Max. output speed 50 MHz

    SPI2->CR1 |= SPI_CR1_SPE;
}

/**
  * @brief  Setup the GPIO.
  * @param  None
  * @retval None
  */

void GPIO_Configure(void)
{
    /* Enable GPIOA Clock */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN;

    /* PA5 - LED. Output PP */
    GPIOA->CRL |= GPIO_CRL_MODE5_0;
    GPIOA->CRL &= ~GPIO_CRL_CNF5;

    /* PA9 - R_CE. Output PP */
    GPIOA->CRH |= GPIO_CRH_MODE9_0;
    GPIOA->CRH &= ~GPIO_CRH_CNF9;

    /* PA10 - R_CSN. Output PP */
    GPIOA->CRH |= GPIO_CRH_MODE10_0;
    GPIOA->CRH &= ~GPIO_CRH_CNF10;

    /* PC6 - T_CE. Output PP */
    GPIOC->CRL |= GPIO_CRL_MODE6_0;
    GPIOC->CRL &= ~GPIO_CRL_CNF6;

    /* PB10 - T_CSN. Output PP */
    GPIOB->CRH |= GPIO_CRH_MODE10_0;
    GPIOB->CRH &= ~GPIO_CRH_CNF10;
}

void EXTI_Configure(void)
{
        /* PA11 - R_IRQ, EXTI*/
    GPIOA->CRH &= ~GPIO_CRH_MODE11_0;
    EXTI->IMR |= EXTI_IMR_MR11;
    EXTI->FTSR |= EXTI_FTSR_TR11;
    AFIO->EXTICR[2] |= AFIO_EXTICR3_EXTI11_PA;

    /* PB12 - T_IRQ, EXTI*/
    GPIOB->CRH &= ~GPIO_CRH_MODE12_0;
    EXTI->IMR |= EXTI_IMR_MR12;
    EXTI->FTSR |= EXTI_FTSR_TR12;
    AFIO->EXTICR[3] |= AFIO_EXTICR4_EXTI12_PB;

    NVIC_SetPriority(EXTI15_10_IRQn, 5);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
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
    /*    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;

    //  TIM14->CR1 |= TIM_CR1_CKD_1;
    TIM14->DIER |= TIM_DIER_UIE;
    TIM14->PSC = 8000;
    TIM14->ARR = 6000;
    TIM14->CNT = 1;

    NVIC_EnableIRQ(TIM14_IRQn);
    NVIC_SetPriority(TIM14_IRQn, 0);

    TIM14->CR1 |= TIM_CR1_CEN;*/
}

void SPI_SendData(SPI_TypeDef *SPI, uint8_t *Data, uint16_t Length)
{
    uint16_t Timeout = 1000;
    uint8_t  Pos     = 0;

    while (Length > Pos)
    {
        /* Wait until TXE flag is set to send data */
        if ((SPI->SR & SPI_SR_TXE) == SPI_SR_TXE)
        {
            *((__IO uint8_t *)&SPI->DR) = Data[Pos];
            Pos++;
        }
        else
        {
            /* Timeout management */
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

//void SPI_ReadRxFifoData(SPI_TypeDef *SPI, uint8_t *Data, uint16_t Length)
//{
//    uint16_t Timeout = 1000;

//    do
//    {
//        if ((SPI->SR & SPI_SR_RXNE) == SPI_SR_RXNE)
//        {
//            (*Data++) = *(__IO uint8_t *)&SPI->DR;
//            Length--;
//        }
//        else
//        {
//            /* Timeout management */
//            Timeout--;
//            if (Timeout == 0)
//            {
//                break;
//            }
//        }
//    } while (Length > 0);

//    while ((SPI->SR & SPI_SR_BSY) != 0)
//    {
//        __NOP();
//    };
//}

void SPI_ReadData(SPI_TypeDef *SPI, uint8_t *Data, uint16_t Length, uint8_t Dummy)
{
    while (SPI->SR & SPI_SR_RXNE)
    {
        (void)SPI->DR;
    };
    while (Length > 0)
    {
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
    while ((SPI->SR & SPI_SR_BSY) != 0)
    {
        __NOP();
    };
}

void SPI_FlushRxFifo(void)
{
    /*    __IO uint32_t tmpreg;
    uint8_t       count = 0xFF;
    while (((SPI1->SR & SPI_SR_FRLVL) != 0))
    {
        count--;
        tmpreg = SPI1->DR;
        UNUSED(tmpreg);
        if (count == 0)
        {
            break;
        }
    }*/
}

void USART_SendChar(USART_TypeDef *USART, unsigned char ch)
{
    while (!(USART->SR & USART_SR_TXE))
    {
        __NOP();
    };
    USART->DR = ch;
    while (!(USART->SR & USART_SR_TC))
    {
        __NOP();
    };
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

void USART_DMASendData(USART_TypeDef *USART, uint8_t *Data, uint16_t Length)
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
    FLASH->CR &= ~FLASH_CR_PER;
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

__weak void uEXTI_IRQHandler(uint32_t Pin)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(Pin);
    /* NOTE: This function Should not be modified, when the callback is needed,
           the uEXTI_IRQHandler could be implemented in the user file
   */
}

__weak void uTIM_IRQHandler(void)
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
