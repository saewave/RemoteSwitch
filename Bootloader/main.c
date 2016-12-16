#include "stm32f0xx.h"

#define MAIN_PROGRAM_START_ADDRESS (uint32_t)0x08000400

int main(void)
{
    typedef void (*pFunction)(void);
    pFunction Jump_To_Application;
    uint32_t  jumpAddress;
    __disable_irq();

    jumpAddress         = *(__IO uint32_t *)(MAIN_PROGRAM_START_ADDRESS + 4);
    Jump_To_Application = (pFunction)jumpAddress;
    __set_MSP(*(__IO uint32_t *)MAIN_PROGRAM_START_ADDRESS);

    Jump_To_Application();

    while (1)
    {
    }
}
