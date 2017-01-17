#include "stm32f10x.h"
#include "F103RE_Peripheral.h"
#include "freertos.h"
#include "nRF24L01P.h"
#include "rf_device.h"
#include "xdebug.h"


int main (void) {
    
    GPIO_Configure();
    SPI_Configure();
    
    nRF24_Configure();
    nRF24_TXMode(SPI2);
    nRF24_RXMode(SPI2);
    rfLoadDevices();

    nRF24_HandleStatus(SPI2, CHIP_Rx);

    FreeRTOS_Init();

    while(1){};
}
