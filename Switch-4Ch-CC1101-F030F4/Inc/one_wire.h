#ifndef __1ware_H
#define __1ware_H

#include "stm32f0xx.h"

#define GPIO_PIN_RESET 0
#define GPIO_PIN_SET 1

typedef struct
{
    GPIO_TypeDef *GPIOx;
    uint16_t      GPIO_Pin;
} OneWireGPIO_t;

typedef enum {
    OneWireInputMode  = 1,
    OneWireOutputMode = 0
} OneWirePinDirection_t;

void OneWire_Init(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin);
uint8_t * OneWireReadTemp(void);
void ChangePinDirection(OneWirePinDirection_t direction);
uint8_t OneWire_CheckPresence(void);
void OneWire_SendBit(uint8_t bit);
void OneWire_SendByte(uint8_t byte);
uint8_t OneWire_ReadByte(void);
uint8_t OneWire_ReadBit(void);

#endif
