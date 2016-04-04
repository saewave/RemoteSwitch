#ifndef __UUID_H
#define __UUID_H

#include <stdint.h>

/**
 * The STM32 factory-programmed UUID memory.
 * Three values of 32 bits each starting at this address
 * Use like this: STM32_UUID[0], STM32_UUID[1], STM32_UUID[2]
 */
#define STM32_UUID ((uint8_t *)0x1FFF7A10)

#endif //__UUID_H
