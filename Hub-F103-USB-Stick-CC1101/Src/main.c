#include "stm32f10x.h"
#include "F103RE_Peripheral.h"
#include "CC1101.h"
#include "rf_device.h"
#include "xdebug.h"
#include "core.h"
#include "usblib.h"

int main (void) {
    
    RCC_Configure();
    CRC_Configure();
    DWT_Configure();
    GPIO_Configure();

    SPI_Configure();
    USART_Configure();
    TIM_Configure();
    USBLIB_Init();
    GPIOB->ODR |= GPIO_ODR_ODR13; //USB UP
    
    CC1101_Reset();
    CC1101_Configure();
    CC1101_RxMode();

    rfLoadDevices();
    rfListDevices();

    QueueResponse("OK, I'm fine.\n", OUSART1);
    EXTI_Configure();
    while(1){
        ProcessDeviceDataQueue();
        ProcessCMDQueue();
        ProcessResponseQueue();
    };
}
