#include "stm32f0xx.h"

//#define _DEBUG

#include "xdebug.h"
#include "xprintf.h"
#include "nRF24L01P.h"
#include "config.h"
#include "rf_cmd_exec.h"
#include "1ware.h"

#include "F030f4_Peripheral.h"

int HandleStatus = 0;
uint8_t UpdateTemperature = 0;
uint16_t Temperature = 0;

void InitAll(void)
{
  F030f4_GPIO_Configure();
  F030f4_USART_Configure();
  F030f4_SPI_Configure();
  F030f4_TIM_Configure();
  F030f4_RTC_Configure();
//  F030f4_System_Configure();
  
  dxdev_out(USART_SendChar);
  
  //Set PA3 as 1 Ware GPIO Pin
  OneWire_Init(GPIOA, (uint16_t)0x0008);

  uint8_t pr = OneWire_CheckPresence();
  OneWire_SendByte(0xCC);
  OneWire_SendByte(0x4E);
  OneWire_SendByte(0x4B);
  OneWire_SendByte(0x46);
  OneWire_SendByte(0x5F);

  dxputs("All Done!\n\n");
  
  rfStartup();
}

int main(void) {
  InitAll();

  uint8_t Status = nRF24_GetStatus();
  
  if (configREAD_ADDR_ON_START) {
    readConfig();
  }
  
  CE_LOW();
  CSN_HIGH();

  nRF24_RXMode();
  nRF24_HandleStatus();
  
//  dxprintf("Status: %x\n", Status);
  dxputs("Done!\n");
  while(1)
  {
    if (HandleStatus) {
      HandleStatus = 0x00;
      dxputs("EXTI0_1_IRQHandler!\n");
      nRF24_HandleStatus();
    }
    if (UpdateTemperature) {
      UpdateTemperature = 0;
      uint16_t Temperature = OneWireReadTemp();
      uint8_t Data[2] = {(uint8_t)(Temperature >> 8), (uint8_t)Temperature};
      rfInternalCallback(Data, 2);
//      dxprintf("Temperature: %x\n", Temperature);
      dxprintf("Temperature: %d.%d\n", (int)Temperature/16, (int)((float)((float)Temperature/16 - (int)Temperature/16)*100));
      
      dxprintf("%d%d:%d%d:%d%d\n", (uint8_t)(((RTC->TR & RTC_TR_HT)  >> 4) >> 16), (uint8_t)((RTC->TR & RTC_TR_HU)>>16), 
                                   (uint8_t)(((RTC->TR & RTC_TR_MNT) >> 4) >> 8),  (uint8_t)((RTC->TR & RTC_TR_MNU)>>8),
                                   (uint8_t)((RTC->TR & RTC_TR_ST) >> 4),  (uint8_t)((RTC->TR & RTC_TR_SU)));
    }
  }
}

void EXTI0_1_IRQHandler(void){
  if( (EXTI->IMR & EXTI_IMR_MR0) && (EXTI->PR & EXTI_PR_PR0)){
    while(GPIOA->IDR & GPIO_IDR_0){}
    EXTI->PR |= EXTI_PR_PR0 ;
    HandleStatus = 1;
  }
}

void TIM14_IRQHandler(void) {
  TIM14->SR &= ~TIM_SR_UIF;
  UpdateTemperature = 1;
}

void SysTick_Handler(void) {
  //Not enabled by default
}
