#include "stm32f0xx.h"

//#define _DEBUG

#include "xdebug.h"
#include "xprintf.h"

#include "F030f4_Peripheral.h"

uint8_t tData[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
uint8_t rData[4] = {0x00};
void InitAll(void)
{
  F030f4_GPIO_Configure();
  F030f4_USART_Configure();
  F030f4_SPI_Configure();
  
  dxdev_out(USART_SendChar);
  dxputs("Init All Done!\n\n");
}

int main(void) {
  InitAll();
  
  GPIOA->BSRR = GPIO_BSRR_BR_1;
  SPI_SendData(&tData[0], 2);
  GPIOA->BSRR = GPIO_BSRR_BS_1;
  SPI_FlushRxFifo();
  
  SPI_ReadRxFifoData(&rData[0], 4);
  
  GPIOA->BSRR = GPIO_BSRR_BR_1;
  SPI_ReadData(&rData[0], 4, 0xAA);
  GPIOA->BSRR = GPIO_BSRR_BS_1;
  
  rData[3] = 0x00;
  USART_SendData(&rData[0], 3);
  
//  dxputs("MC still alive!\n");
  while(1)
  {
    
  }
}

