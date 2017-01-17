#include "stm32f10x.h"
#include "stdint.h"
#include "xdebug.h"
#include "config.h"

#define SET_BIT(REG, BIT)     ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)    ((REG) & (BIT))
#define CLEAR_REG(REG)        ((REG) = (0x0))
#define WRITE_REG(REG, VAL)   ((REG) = (VAL))
#define READ_REG(REG)         ((REG))
#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))
#define UNUSED(x) ((void)(x))
#define OUSART1 0x01
#define OUSART2 0x02

#define P_T_CSN_LOW() GPIOB->BSRR = GPIO_BSRR_BR10
#define P_T_CSN_HIGH() GPIOB->BSRR = GPIO_BSRR_BS10

#define P_T_CE_LOW() GPIOC->BSRR = GPIO_BSRR_BR6
#define P_T_CE_HIGH() GPIOC->BSRR = GPIO_BSRR_BS6

#define DWT_CYCCNT    *(volatile uint32_t *)0xE0001004
#define DWT_CONTROL   *(volatile uint32_t *)0xE0001000
#define SCB_DEMCR     *(volatile uint32_t *)0xE000EDFC
#define FLASH_FKEY1   0x45670123
#define FLASH_FKEY2   0xCDEF89AB

void System_Configure (void);
void RTC_Configure (void);
void RTC_Time_Configure(uint8_t hh, uint8_t mm, uint8_t ss);
void RTC_Alarm_Configure(uint8_t hh, uint8_t mm, uint8_t ss);
void RTC_WakeUp_Configure(uint16_t Period);
void USART_Configure(void);
void SPI_Configure (void);
void GPIO_Configure (void);
void EXTI_Configure(void);
void TIM_Configure (void);
  
void SPI_SendData(SPI_TypeDef * SPI, uint8_t *Data, uint16_t Length);
void SPI_ReadRxFifoData(SPI_TypeDef *SPI, uint8_t *Data, uint16_t Length);
void SPI_ReadData(SPI_TypeDef * SPI, uint8_t *Data, uint16_t Length, uint8_t Dummy);
void SPI_FlushRxFifo(void);

void USART_SendChar(USART_TypeDef * USART, unsigned char ch);
void USART_SendData(USART_TypeDef * USART, uint8_t *Data, uint16_t Length);
void USART_DMASendData(USART_TypeDef *USART, uint8_t *Data, uint16_t Length);
    
void FLASH_WriteData(uint32_t fAddress, uint8_t *Data, int Size,
                     uint32_t EraseAddress);
void WWDG_Configure(void);
void WWDG_Update(void);
    
void GOTO_Sleep(void);
void GOTO_Stop(void);

void uEXTI_IRQHandler(uint32_t Pin);
void uTIM_IRQHandler(void);
void uRTC_IRQHandler(uint32_t RTC_ISR);
void uUSART_IRQHandler(USART_TypeDef * USART, uint32_t SR, uint8_t * Data, uint8_t Length);
