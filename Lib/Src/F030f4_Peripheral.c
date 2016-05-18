
#ifndef __F030f4_Peripheral_H
#define __F030f4_Peripheral_H

#include "F030f4_Peripheral.h"

#define APBCLK 8000000UL
#define BAUDRATE 115200UL

/**
  * @brief  Setup the Syatem.
  * @param  None
  * @retval None
  */

void F030f4_System_Configure (void) {

  SysTick_Config(SystemCoreClock/1000000);
  SysTick->CTRL |= ((uint32_t)0x00000004);
  NVIC_SetPriority(SysTick_IRQn, 0);

}

/**
  * @brief  Setup the RTC.
  * @param  None
  * @retval None
  */

void F030f4_RTC_Configure (void) {

  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  PWR->CR |= PWR_CR_DBP;
  
//  if ((RCC->BDCR & RCC_BDCR_RTCEN) != RCC_BDCR_RTCEN) {
    RCC->BDCR |= RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_LSI;
    RCC->CSR|=RCC_CSR_LSION;
    while(!(RCC->CSR&RCC_CSR_LSIRDY));
//  }
//  RTC->CR |= RTC_CR_BYPSHAD;
  /*
  if (RTC->TR == 0) {
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    RTC->ISR = RTC_ISR_INIT;
    uint16_t Timeout = UINT16_MAX;
    while (((RTC->ISR & RTC_ISR_INITF) == 0) && Timeout > 0){Timeout--;};
    
  //  RTC->PRER = 0x007F0137;

    RTC->TR = RTC_TR_HT_0 | (RTC_TR_HU_0 | RTC_TR_HU_2) |   // Houts: 15
              RTC_TR_MNT_0 | RTC_TR_MNU_0 |                 // Minutes: 22
              RTC_TR_ST_0;                                  // Seconds: 10
    RTC->ISR &= ~RTC_ISR_INIT;
    RTC->WPR = 0xFF;
  }*/
}

/**
  * @brief  Setup the USART.
  *         TODO: Implement the PIN choose
  * @param  None
  * @retval None
  */

void F030f4_USART_Configure (void) {
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
  USART1->BRR = (APBCLK+BAUDRATE/2)/BAUDRATE;
  USART1->CR1 |= USART_CR1_UE | USART_CR1_TE;
  USART1->CR3 |= USART_CR3_HDSEL;
}

/**
  * @brief  Setup the SPI.
  * @param  None
  * @retval None
  */

void F030f4_SPI_Configure (void) {
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
//  SPI1->CR1 |= SPI_CR1_BR;
  SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;
  SPI1->CR2 |= SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2 | SPI_CR2_NSSP | SPI_CR2_FRXTH;
  
  SPI1->CR1 |= SPI_CR1_SPE;
}

/**
  * @brief  Setup the GPIO.
  * @param  None
  * @retval None
  */

void F030f4_GPIO_Configure (void) {
  
  /* Enable GPIOA Clock */
  RCC->AHBENR  |= RCC_AHBENR_GPIOAEN;

  /* PA2 USART TX. AF, OD */
  GPIOA->MODER |= GPIO_MODER_MODER2_1;
  GPIOA->OTYPER |= GPIO_OTYPER_OT_2;
  GPIOA->AFR[0] |= 0x0100;
  
  /* PA5 SPI_SCK, PA6 SPI_MISO, PA6 SPI_MOSI. AF, PP */
  GPIOA->MODER |= GPIO_MODER_MODER5_1 | GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1;
  GPIOA->OTYPER &= ~(GPIO_OTYPER_OT_5 | GPIO_OTYPER_OT_6 | GPIO_OTYPER_OT_7);

  /* PA1 SPI_CSN. Output PP */
  GPIOA->MODER |= GPIO_MODER_MODER1_0;
  GPIOA->BSRR = GPIO_BSRR_BS_1;
  
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
  
  /* PA3 One Wire. Output OD */
  GPIOA->MODER |= GPIO_MODER_MODER3_0;
  GPIOA->OTYPER |= GPIO_OTYPER_OT_3;
  GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR3;
  
  /* PA9. Output PP */
  GPIOA->MODER |= GPIO_MODER_MODER10_0;
//  GPIOA->OTYPER |= GPIO_OTYPER_OT_9;
  GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR10;
  
}

/**
  * @brief  Setup the TIM.
  * @param  None
  * @retval None
  */

void F030f4_TIM_Configure (void) {
  
  RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
  
//  TIM14->CR1 |= TIM_CR1_CKD_1;
  TIM14->DIER |= TIM_DIER_UIE;
  TIM14->PSC = 8000;
  TIM14->ARR = 2000;
  
  NVIC_EnableIRQ(TIM14_IRQn); /* (1) */
  NVIC_SetPriority(TIM14_IRQn,0); /* (2) */
  
  TIM14->CR1 |= TIM_CR1_CEN;
}

void SPI_SendData(uint8_t *Data, uint16_t Length) {
  uint16_t Timeout = 1000;
  /*
  dxprintf("SPI_SendData\n");
  if (Length > 2){
    for(int i = 0; i< 2; i=i+2) {
      dxprintf("%d: %x\n", i, * ( ((uint16_t*)Data) + i));
    }
  }
    */
  while (Length > 0) {
    /* Wait until TXE flag is set to send data */
    if((SPI1->SR & SPI_SR_TXE) == SPI_SR_TXE)
    {
/*      if(Length > 1)
      {
        // write on the data register in packing mode
        SPI1->DR = *((uint16_t*)Data);
        Data += sizeof(uint16_t);
        Length -= sizeof(uint16_t);
      }
      else
      {*/
        *((__IO uint8_t*)&SPI1->DR) = (*Data++);
        Length--;
//      }
    }
    else
    {
      /* Timeout management */
      Timeout--;
      if (Timeout == 0) {
        break;
      }
    }
  }
  while ((SPI1->SR & SPI_SR_BSY) != 0){};
}

void SPI_ReadRxFifoData(uint8_t * Data, uint16_t Length) {
  uint16_t Timeout = 1000;

  do {
    if((SPI1->SR & SPI_SR_RXNE) == SPI_SR_RXNE) {
      (*Data++)= *(__IO uint8_t *)&SPI1->DR;
      Length--;
    } else {
      /* Timeout management */
      Timeout--;
      if (Timeout == 0) {
        break;
      }
    }
  } while (Length > 0);

  while ((SPI1->SR & SPI_SR_BSY) != 0){};
}

void SPI_ReadData(uint8_t *Data, uint16_t Length, uint8_t Dummy) {
  uint16_t Timeout = 10;
  while (Length > 0) {
    if((SPI1->SR & SPI_SR_TXE) != 0)
    {
      *((__IO uint8_t*)&SPI1->DR) = Dummy;
      while(!(SPI1->SR & SPI_SR_RXNE));
      (*Data++)= *(__IO uint8_t *)&SPI1->DR;
      Length--;
    }
    else
    {
      /* Timeout management */
      Timeout--;
      if (Timeout == 0) {
        break;
      }
    }
  }
  while ((SPI1->SR & SPI_SR_BSY) != 0){};
}

void SPI_FlushRxFifo(void)
{
  __IO uint32_t tmpreg;
  uint8_t  count = 0xFF;
  while(((SPI1->SR & SPI_SR_FRLVL) != 0))
  {
    count--;
    tmpreg = SPI1->DR;
    UNUSED(tmpreg);
    if(count == 0)
    {
      break;
    }
  }
}

void USART_SendChar( unsigned char ch) {
  while ((USART1->ISR & USART_ISR_TXE) == 0){};
  USART1->TDR = ch;
}

void USART_SendData(uint8_t * Data, uint16_t Length) {
  do {
    while ((USART1->ISR & USART_ISR_TXE) == 0){};
    USART1->TDR = *Data;
    Data++;
    Length--;
  } while (Length > 0);
}

#endif /* __F030f4_Peripheral_H */
