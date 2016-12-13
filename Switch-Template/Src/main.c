#include "stm32f0xx.h"

#define _DEBUG

//#include "1ware.h"
#include "config.h"
#include "nRF24L01P.h"
#include "rf_cmd_exec.h"
#include "xdebug.h"
#include "xprintf.h"

#include "F030f4_Peripheral.h"

int      HandleStatus      = 0;
uint8_t  UpdateTemperature = 0;
uint16_t Temperature       = 0;

void InitAll(void)
{
    GPIO_Configure();
    USART_Configure();
    SPI_Configure();
    TIM_Configure();
    RTC_Configure();
    //  System_Configure();

    dxdev_out(USART_SendChar);

    dxputs("InitAll Done!\n\n");

    rfStartup();
}

int main(void)
{
    InitAll();

    if (configREAD_ADDR_ON_START)
    {
        readConfig();
    }

    CE_LOW();
    CSN_HIGH();

    nRF24_RXMode();
    nRF24_HandleStatus();

    while (1)
    {
        if (HandleStatus)
        {
            HandleStatus = 0x00;
            nRF24_HandleStatus();
        }
    }
}

void EXTI_IRQHandler(uint8_t Pin)
{
    HandleStatus = 1;
}

void TIM_IRQHandler(void)
{
    dxputs("TIM_IRQHandler!\n");
}
