
#ifndef __Peripheral_H
#define __Peripheral_H

#include "F030f4_Peripheral.h"

#define APBCLK 8000000UL
#define BAUDRATE 115200UL

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
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_DBP;

    RTC->PRER |= RTC_PRER_PREDIV_S;
    RCC->BDCR |= RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_LSI;
    RCC->CSR |= RCC_CSR_LSION;
    while (!(RCC->CSR & RCC_CSR_LSIRDY))
    {
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
    while (!(RTC->ISR & RTC_ISR_INITF))
    {
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
    while (!(RTC->ISR & RTC_ISR_ALRAWF))
    {
        __NOP();
    };

    RTC->ALRMAR |= RTC_ALRMAR_MSK4;
    uint32_t ALRMAR = 0;
    if (ss != 0xFF)
    {
        ALRMAR |= ((ss / 10) << 4) | (ss % 10);
        RTC->ALRMAR &= ~RTC_ALRMAR_MSK1;
    }
    else
    {
        RTC->ALRMAR |= RTC_ALRMAR_MSK1;
    }
    if (mm != 0xFF)
    {
        ALRMAR |= (((mm / 10) << 4) | (mm % 10)) << 8;
        RTC->ALRMAR &= ~RTC_ALRMAR_MSK2;
    }
    else
    {
        RTC->ALRMAR |= RTC_ALRMAR_MSK2;
    }
    if (hh != 0xFF)
    {
        ALRMAR |= (((hh / 10) << 4) | (hh % 10)) << 8;
        RTC->ALRMAR &= ~RTC_ALRMAR_MSK3;
    }
    else
    {
        RTC->ALRMAR |= RTC_ALRMAR_MSK3;
    }
    RTC->ALRMAR |= ALRMAR;
    RTC->CR |= RTC_CR_ALRAE;  //Enable alarm
    RTC->CR |= RTC_CR_ALRAIE; //Enable alarm interrupt

    while (!(RTC->ISR & RTC_ISR_ALRAWF))
    {
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
    while (!(RTC->ISR & RTC_ISR_WUTWF))
    {
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
    USART1->BRR = (APBCLK + BAUDRATE / 2) / BAUDRATE;
    USART1->CR1 |= USART_CR1_UE | USART_CR1_TE;
    USART1->CR3 |= USART_CR3_HDSEL;
}

/**
  * @brief  Setup the SPI.
  * @param  None
  * @retval None
  */

void SPI_Configure(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    //  SPI1->CR1 |= SPI_CR1_BR;
    SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;
    SPI1->CR2 |=
        SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2 | SPI_CR2_NSSP | SPI_CR2_FRXTH;

    SPI1->CR1 |= SPI_CR1_SPE;
}

/**
  * @brief  Setup the GPIO.
  * @param  None
  * @retval None
  */

void GPIO_Configure(void)
{
    /* Enable GPIOA Clock */
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    /* PA2 USART TX. AF, OD */
    /*
    GPIOA->MODER |= GPIO_MODER_MODER2_1;
    GPIOA->OTYPER |= GPIO_OTYPER_OT_2;
    GPIOA->AFR[0] |= 0x0100;
    */
    /* PA9 USART TX. AF, OD */
    GPIOA->MODER |= GPIO_MODER_MODER9_1;
    GPIOA->OTYPER |= GPIO_OTYPER_OT_9;
    GPIOA->AFR[1] |= 0x0110;

    /* PA5 SPI_SCK, PA6 SPI_MISO, PA6 SPI_MOSI. AF, PP */
    GPIOA->MODER |=
        GPIO_MODER_MODER5_1 | GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1;
    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT_5 | GPIO_OTYPER_OT_6 | GPIO_OTYPER_OT_7);

    /* PA1 SPI_CSN. Output PP */
    GPIOA->MODER |= GPIO_MODER_MODER3_0;
    GPIOA->BSRR = GPIO_BSRR_BS_3;

    /* PA4 SPI_CE. Output PP */
    GPIOA->MODER |= GPIO_MODER_MODER4_0;
    GPIOA->BSRR = GPIO_BSRR_BR_4;

    /* PA0 nRF24_IRQ. Input EXTI */
    GPIOA->MODER &= ~GPIO_MODER_MODER0;

    EXTI->IMR |= EXTI_IMR_MR0;
    EXTI->FTSR |= EXTI_FTSR_TR0;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PA;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    NVIC_EnableIRQ(EXTI0_1_IRQn);
    NVIC_SetPriority(EXTI0_1_IRQn, 1);
}

/**
  * @brief  Setup the TIM.
  * @param  None
  * @retval None
  */

void TIM_Configure(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;

    //  TIM14->CR1 |= TIM_CR1_CKD_1;
    TIM14->DIER |= TIM_DIER_UIE;
    TIM14->PSC = 8000;
    TIM14->ARR = 6000;
    TIM14->CNT = 1;

    NVIC_EnableIRQ(TIM14_IRQn);
    NVIC_SetPriority(TIM14_IRQn, 0);

    TIM14->CR1 |= TIM_CR1_CEN;
}

void SPI_SendData(uint8_t *Data, uint16_t Length)
{
    uint16_t Timeout = 1000;
    uint8_t  Pos     = 0;

    while (Length > Pos)
    {
        /* Wait until TXE flag is set to send data */
        if ((SPI1->SR & SPI_SR_TXE) == SPI_SR_TXE)
        {
            *((__IO uint8_t *)&SPI1->DR) = Data[Pos];
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
    while ((SPI1->SR & SPI_SR_BSY) != 0)
    {
        __NOP();
    };
}

void SPI_ReadRxFifoData(uint8_t *Data, uint16_t Length)
{
    uint16_t Timeout = 1000;

    do
    {
        if ((SPI1->SR & SPI_SR_RXNE) == SPI_SR_RXNE)
        {
            (*Data++) = *(__IO uint8_t *)&SPI1->DR;
            Length--;
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
    } while (Length > 0);

    while ((SPI1->SR & SPI_SR_BSY) != 0)
    {
        __NOP();
    };
}

void SPI_ReadData(uint8_t *Data, uint16_t Length, uint8_t Dummy)
{
    uint16_t Timeout = 10;
    while (Length > 0)
    {
        if ((SPI1->SR & SPI_SR_TXE) != 0)
        {
            *((__IO uint8_t *)&SPI1->DR) = Dummy;
            while (!(SPI1->SR & SPI_SR_RXNE))
                ;
            (*Data++) = *(__IO uint8_t *)&SPI1->DR;
            Length--;
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
    while ((SPI1->SR & SPI_SR_BSY) != 0)
    {
        __NOP();
    };
}

void SPI_FlushRxFifo(void)
{
    __IO uint32_t tmpreg;
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
    }
}

void USART_SendChar(unsigned char ch)
{
    while ((USART1->ISR & USART_ISR_TXE) == 0)
    {
        __NOP();
    };
    USART1->TDR = ch;
}

void USART_SendData(uint8_t *Data, uint16_t Length)
{
    do
    {
        while ((USART1->ISR & USART_ISR_TXE) == 0)
        {
            __NOP();
        };
        USART1->TDR = *Data;
        Data++;
        Length--;
    } while (Length > 0);
}

void FLASH_WriteData(uint32_t fAddress, uint8_t *Data, uint8_t Size,
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

    while (FLASH->SR & FLASH_SR_BSY)
    {
        __NOP();
    }; // Wait untill memory ready
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
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    dxputs("Going to sleep...");
    __WFI();
    dxputs("zzz....");
}

void GOTO_Stop(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
#if DEBUG_IN_STOP_MODE == 1
    //Before using DEBUG in Stop mode reduce the frequency in your debugger to lower than 500kHz
    DBGMCU->CR |= DBGMCU_CR_DBG_STOP;
#endif
    PWR->CR |= PWR_CR_LPDS;
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
    dxputs("Going to stop...");
    __WFI();
    dxputs("stop....\n");
}

void EXTI0_1_IRQHandler(void)
{
    if ((EXTI->IMR & EXTI_IMR_MR0) && (EXTI->PR & EXTI_PR_PR0))
    {
        EXTI->PR |= EXTI_PR_PR0;
    }
    uEXTI_IRQHandler(EXTI->IMR);
}

void TIM14_IRQHandler(void)
{
    TIM14->SR &= ~TIM_SR_UIF;
    uTIM_IRQHandler();
}

void RTC_IRQHandler(void)
{
    uint32_t RTC_ISR = RTC->ISR;
    if (RTC->ISR & RTC_ISR_ALRAF)
    {
        RTC->ISR &= ~RTC_ISR_ALRAF;
    }
    if (EXTI->PR & EXTI_PR_PR17)
    {
        EXTI->PR |= EXTI_PR_PR17;
    }
    uRTC_IRQHandler(RTC_ISR);
}

void SysTick_Handler(void)
{
    // Not enabled by default
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
#endif /* __Peripheral_H */
