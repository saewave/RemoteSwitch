#include "config.h"
#include "stdint.h"
#include "stm32f0xx.h"
#include "xdebug.h"

#define SYS_FREQUENCY 8000000UL
#define SET_BIT(REG, BIT) ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT) ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT) ((REG) & (BIT))
#define CLEAR_REG(REG) ((REG) = (0x0))
#define WRITE_REG(REG, VAL) ((REG) = (VAL))
#define READ_REG(REG) ((REG))
#define MODIFY_REG(REG, CLEARMASK, SETMASK) WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))
#define UNUSED(x) ((void)(x))

#define CSN_LOW() GPIOF->BSRR  = GPIO_BSRR_BR_6
#define CSN_HIGH() GPIOF->BSRR = GPIO_BSRR_BS_6

#define Transceiver_CTS_PORT GPIOB
#define Transceiver_CTS_PIN GPIO_IDR_8

#define UART_DIV_SAMPLING16(_PCLK_, _BAUD_) (((_PCLK_)*25) / (4 * (_BAUD_)))
#define UART_DIVMANT_SAMPLING16(_PCLK_, _BAUD_) (UART_DIV_SAMPLING16((_PCLK_), (_BAUD_)) / 100)
#define UART_DIVFRAQ_SAMPLING16(_PCLK_, _BAUD_) (((UART_DIV_SAMPLING16((_PCLK_), (_BAUD_)) - (UART_DIVMANT_SAMPLING16((_PCLK_), (_BAUD_)) * 100)) * 16 + 50) / 100)

#define UART_BRR_SAMPLING16(_PCLK_, _BAUD_) (((UART_DIVMANT_SAMPLING16((_PCLK_), (_BAUD_)) << 4) +    \
                                              (UART_DIVFRAQ_SAMPLING16((_PCLK_), (_BAUD_)) & 0xF0)) + \
                                             (UART_DIVFRAQ_SAMPLING16((_PCLK_), (_BAUD_)) & 0x0F))

#define CH_GND() GPIOA->BSRR = GPIO_BSRR_BS_0
#define CH_VDD() GPIOA->BSRR = GPIO_BSRR_BR_0

void System_Configure(void);
void ADC_Configure(void);
void ADC_Start(uint32_t *Buf, uint16_t Length);
void ADC_Stop(void);
void CRC_Configure(void);
void RTC_Configure(void);
void RTC_Time_Configure(uint8_t hh, uint8_t mm, uint8_t ss);
void RTC_Alarm_Configure(uint8_t hh, uint8_t mm, uint8_t ss);
void RTC_WakeUp_Configure(uint16_t Period);
void USART_Configure(void);
void SPI_Configure(void);
void GPIO_Configure(void);
void TIM_Configure(void);
void EXTI_Configure(void);

void SPI_SendData(SPI_TypeDef *SPI, uint8_t *Data, uint16_t Length);
void SPI_ReadRxFifoData(SPI_TypeDef *SPI, uint8_t *Data, uint16_t Length);
void SPI_ReadData(SPI_TypeDef *SPI, uint8_t *Data, uint16_t Length, uint8_t Dummy);
void SPI_FlushRxFifo(SPI_TypeDef *SPI);

void USART_SendChar(unsigned char ch);
void USART_SendData(uint8_t *Data, uint16_t Length);
void FLASH_Erase(uint32_t EraseAddress, uint8_t Pages);
void FLASH_WriteData(uint32_t fAddress, uint8_t *Data, uint8_t Size,
                     uint32_t EraseAddress);
uint8_t FWWriteToFlash(uint8_t *cData);

void GOTO_Sleep(void);
void GOTO_Stop(void);

void uEXTI_IRQHandler(uint32_t Pin);
void uTIM_IRQHandler(TIM_TypeDef *Tim);
void uRTC_IRQHandler(uint32_t RTC_ISR);
void Delay_us(uint32_t Delay);
