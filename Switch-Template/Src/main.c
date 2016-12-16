#include "stm32f0xx.h"
#include "config.h"

#if MOVE_VECTOR_TABLE==1
volatile uint32_t *VectorTable = (volatile uint32_t *)MAIN_PROGRAM_RAM_ADDRESS;
#endif

#include "nRF24L01P.h"
#include "rf_cmd_exec.h"
#include "xdebug.h"
#include "xprintf.h"

#include "F030f4_Peripheral.h"
uint8_t i;
int      HandleStatus      = 1;
uint32_t Counter = 0;

void InitAll(void)
{
    GPIO_Configure();
    USART_Configure();
    dxdev_out(USART_SendChar);
    SPI_Configure();
    RTC_Configure();
    RTC_Time_Configure(0,0,0);
    RTC_Alarm_Configure(0xFF,0xFF,5);

    dxputs("InitAll Done!\n\n");

    rfStartup();
}

int main(void)
{
#if MOVE_VECTOR_TABLE==1
    for (i = 0; i < 48; i++)
    {
        VectorTable[i] = *(__IO uint32_t *)(MAIN_PROGRAM_START_ADDRESS + (i << 2));
    }
    SYSCFG->CFGR1 |= SYSCFG_CFGR1_MEM_MODE;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;
#endif
    __enable_irq();
    
    InitAll();

    if (configREAD_ADDR_ON_START)
    {
        readConfig();
    }

    CE_LOW();
    CSN_HIGH();

    nRF24_RXMode();

#if USE_STOP_MODE==1
    GOTO_Stop();
    //We should never been here!
    dxputs("What I'm doing here?\n");
#endif
    while (1)
    {
#if USE_STOP_MODE==0
        if (HandleStatus)
        {
            HandleStatus = 0x00;
            nRF24_HandleStatus();
        }
#endif
    }
}

void uEXTI_IRQHandler(uint32_t Pin)
{
#if USE_STOP_MODE==1
    nRF24_HandleStatus();
#else
    HandleStatus = 1;
#endif
}

void uTIM_IRQHandler(void)
{
    dxputs("TIM_IRQHandler!\n");
}


void uRTC_IRQHandler(uint32_t RTC_ISR)
{
    dxprintf("uRTC_IRQHandler: 0x%h\n", RTC_ISR);
}
