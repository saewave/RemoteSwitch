#include "stm32f0xx.h"

//#define _DEBUG

#include "1ware.h"
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

    //Set PA3 as 1 Ware GPIO Pin
    OneWire_Init(GPIOA, (uint16_t)0x0008);

    uint8_t pr = OneWire_CheckPresence();
    OneWire_SendByte(0xCC);
    OneWire_SendByte(0x4E);
    OneWire_SendByte(0x4B);
    OneWire_SendByte(0x46);
    OneWire_SendByte(0x5F);

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
            dxputs("EXTI0_1_IRQHandler!\n");
            nRF24_HandleStatus();
        }
        if (UpdateTemperature)
        {
            UpdateTemperature    = 0;
            uint16_t Temperature = OneWireReadTemp();
            uint8_t  Data[2]     = {(uint8_t)(Temperature >> 8), (uint8_t)Temperature};
            rfInternalCallback(Data, 2);
            //      dxprintf("Temperature: %x\n", Temperature);
            dxprintf("Temperature: %d.%d\n", (int)Temperature / 16, (int)((float)((float)Temperature / 16 - (int)Temperature / 16) * 100));

            dxprintf("%d%d:%d%d:%d%d\n", (uint8_t)(((RTC->TR & RTC_TR_HT) >> 4) >> 16), (uint8_t)((RTC->TR & RTC_TR_HU) >> 16),
                     (uint8_t)(((RTC->TR & RTC_TR_MNT) >> 4) >> 8), (uint8_t)((RTC->TR & RTC_TR_MNU) >> 8),
                     (uint8_t)((RTC->TR & RTC_TR_ST) >> 4), (uint8_t)((RTC->TR & RTC_TR_SU)));
        }
    }
}

void EXTI_IRQHandler(uint8_t Pin)
{
    HandleStatus = 1;
}

void TIM_IRQHandler(void)
{
    UpdateTemperature = 1;
}
