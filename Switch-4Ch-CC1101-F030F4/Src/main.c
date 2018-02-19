#include "stm32f0xx.h"
#include "F030f4_Peripheral.h"
#include "config.h"
#include "xdebug.h"
#include "xprintf.h"
#include "CC1101.h"
#include "one_wire.h"
#include "rf_cmd_exec.h"
#define TX_MODE 0

#if SAE_MOVE_VECTOR_TABLE==1
volatile uint32_t *VectorTable = (volatile uint32_t *)SAE_MAIN_PROGRAM_RAM_ADDRESS;
#endif

//uint8_t i;
int      HandleStatus      = 1;
uint32_t Counter = 0;
uint32_t Blink = GPIO_BSRR_BR_10;

void InitPeriph(void)
{
    CRC_Configure();
//    TIM_Configure();
    GPIO_Configure();
    USART_Configure();
//    USART_SendData((uint8_t *)"Test\n", 5);
    dxdev_out(USART_SendChar);
    SPI_Configure();
    OneWire_Init(GPIOA, GPIO_IDR_3);
    RTC_Configure();
    RTC_Time_Configure(0,0,0);
    RTC_Alarm_Configure(0xFF,0xFF,5);

//    dxputs("InitAll Done!\n\n");

    rfStartup();
}

int main(void)
{
/*
    RCC->CFGR &= ~RCC_CFGR_SW;      //Change System Clock to HSI
    while ((RCC->CFGR & RCC_CFGR_SWS) != 0x00) {__NOP();};
    RCC->CR &= ~RCC_CR_PLLON;        //Disable Pll
    while ((RCC->CR & RCC_CR_PLLON)) {__NOP();};
    RCC->CFGR &= ~RCC_CFGR_PLLSRC;  //Set Pll Source to HSI
    RCC->CFGR |= RCC_CFGR_PLLMULL16;//Set Pll Mul to 16
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLON)) {__NOP();};
    RCC->CFGR |= RCC_CFGR_SW_1;      //Change System Clock to HSI
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_1) {__NOP();};
    RCC->CR &= ~RCC_CR_HSEON;        //Disable HSE
*/
#if SAE_MOVE_VECTOR_TABLE==1
    for (i = 0; i < 48; i++)
    {
        VectorTable[i] = *(__IO uint32_t *)(SAE_MAIN_PROGRAM_START_ADDRESS + (i << 2));
    }
    SYSCFG->CFGR1 |= SYSCFG_CFGR1_MEM_MODE;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;
#endif
    __enable_irq();
    
    InitPeriph();

    if (SAE_READ_ADDR_ON_START)
    {
        readConfig();
    }

    Transceiver_Reset();
    Transceiver_Configure();

    dxputs("InitAll Done!\n\n");
    
//    CC1101_HandleStatus();
    
#if TX_MODE == 0
    Transceiver_RxMode();

    EXTI_Configure();
#endif

#if SAE_USE_STOP_MODE==1
    GOTO_Stop();
    //We should never been here!
    dxputs("What I'm doing here?\n");
#endif
    #if TX_MODE == 1
        CC1101_TxTestData();
    #endif
    while (1)
    {
//        USART_SendData(USART1, (char *)"Test\n", 5);
//        Delay_ms(1000);
#if SAE_USE_STOP_MODE==0
        if (HandleStatus)
        {
            HandleStatus = 0x00;
            Transceiver_HandleStatus();
        }
#endif
    }
}

void uEXTI_IRQHandler(uint32_t Pin)
{
#if SAE_USE_STOP_MODE==1
    Transceiver_HandleStatus();
#else
    HandleStatus = 1;
#endif
}

void uTIM_IRQHandler(void)
{
//    dxputs("TIM_IRQHandler!\n");
}


void uRTC_IRQHandler(uint32_t RTC_ISR)
{
    dxprintf("uRTC_IRQHandler: 0x%h\n", RTC_ISR);
//    RTC_Time_Configure(0,0,0);
    uint8_t *Temp;
    Temp = OneWireReadTemp();
    rfInternalCallback(Temp, 2);
}
