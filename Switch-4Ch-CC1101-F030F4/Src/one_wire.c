#include "one_wire.h"
#include "F030f4_Peripheral.h"

uint32_t PRESENCE_TIMEOUT = 5000;

const uint16_t ONE_WIRE_BIT_0 = 130;
const uint16_t ONE_WIRE_BIT_1 = 10;

const uint16_t ONE_WIRE_REST_BIT    = 750;
const uint16_t ONE_WIRE_RESET_DELAY = 700;

const uint16_t ONE_WIRE_1_US  = 1;
const uint16_t ONE_WIRE_15_US = 1;

OneWireGPIO_t OneWireGPIO;

void OneWire_Init(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin)
{
    OneWireGPIO.GPIOx    = GPIOx;
    OneWireGPIO.GPIO_Pin = GPIO_Pin;
};

void OneWire_SetPin(uint8_t GPIO_PinState)
{
    if (GPIO_PinState != 0)
    {
        OneWireGPIO.GPIOx->BSRR = (uint32_t)OneWireGPIO.GPIO_Pin;
    }
    else
    {
        OneWireGPIO.GPIOx->BRR = (uint32_t)OneWireGPIO.GPIO_Pin;
    }
}

uint8_t * OneWireReadTemp(void)
{
    static uint8_t Temp[2] = {0xFF, 0xFF};
    /* Start temp */
    if (OneWire_CheckPresence() != 1)
    {
        return Temp;
    }

    for (uint16_t i = 0; i < ONE_WIRE_BIT_0; i++)
    {
    };
    OneWire_SendByte(0xCC);
    OneWire_SendByte(0x44);

    OneWire_SetPin(GPIO_PIN_RESET);
    //  OneWire.GPIOx->BRR = (uint32_t)OneWire.GPIO_Pin;    //Reset pin
    Delay_us(ONE_WIRE_1_US);
    //    for(uint8_t del = 0; del < 10; del++){};
    //  OneWire.GPIOx->BSRR = (uint32_t)OneWire.GPIO_Pin;   //Set pin
    OneWire_SetPin(GPIO_PIN_SET);

//    ChangePinDirection(OneWireInputMode);
    while ((OneWireGPIO.GPIOx->IDR & OneWireGPIO.GPIO_Pin) == (uint32_t)GPIO_PIN_RESET)
        ;

    if (OneWire_CheckPresence() != 1)
    {
        return Temp;
    }

    Delay_us(ONE_WIRE_BIT_0);
    OneWire_SendByte(0xCC);
    OneWire_SendByte(0xBE);
    
    Temp[1] = OneWire_ReadByte();
    Temp[0] = OneWire_ReadByte();

    return Temp;
}

uint8_t OneWire_CheckPresence(void)
{
//    ChangePinDirection(OneWireOutputMode);
    uint32_t Timeout = PRESENCE_TIMEOUT;
    OneWire_SetPin(GPIO_PIN_RESET);
    for (uint16_t i = 0; i < ONE_WIRE_RESET_DELAY; i++)
    {
    };
    OneWire_SetPin(GPIO_PIN_SET);
//    ChangePinDirection(OneWireInputMode);

    uint16_t ResponseTime = 0;
    do
    {
        if ((OneWireGPIO.GPIOx->IDR & OneWireGPIO.GPIO_Pin) != (uint32_t)GPIO_PIN_RESET)
        {
            if (ResponseTime > 0)
                break;
        }
        else
        {
            ResponseTime++;
        }
        Timeout--;
    } while (Timeout > 0);
//    ChangePinDirection(OneWireOutputMode);
    return (ResponseTime > 0 && Timeout > 0) ? 1 : 0;
}

void OneWire_SendBit(uint8_t bit)
{
    uint16_t delay1 = ONE_WIRE_BIT_0, delay2 = ONE_WIRE_BIT_1;
    if (bit == 1)
    {
        delay1 = ONE_WIRE_BIT_1, delay2 = ONE_WIRE_BIT_0;
    }
    OneWire_SetPin(GPIO_PIN_RESET);
    Delay_us(delay1);
    OneWire_SetPin(GPIO_PIN_SET);
    Delay_us(delay2);
};

void OneWire_SendByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++) OneWire_SendBit(byte >> i & 1);
}

uint8_t OneWire_ReadByte(void)
{
    uint8_t byte = 0;
    for (uint8_t i = 0; i < 8; i++) byte |= (OneWire_ReadBit() << i);
    return byte;
}

uint8_t OneWire_ReadBit(void)
{
//    ChangePinDirection(OneWireOutputMode);
    OneWire_SetPin(GPIO_PIN_RESET);
    Delay_us(ONE_WIRE_1_US);
    OneWire_SetPin(GPIO_PIN_SET);
//    ChangePinDirection(OneWireInputMode);
    //  Delay_us(ONE_WIRE_1_US);

    if ((OneWireGPIO.GPIOx->IDR & OneWireGPIO.GPIO_Pin) != (uint32_t)GPIO_PIN_RESET)
    {
        Delay_us(ONE_WIRE_REST_BIT);
        return 1;
    }
    else
    {
        Delay_us(ONE_WIRE_REST_BIT);
        return 0;
    }
}
