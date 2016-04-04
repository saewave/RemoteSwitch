#ifndef __CONFIG_H
#define __CONFIG_H

//#include "stm32f0xx_hal.h"

#define configFLASH_CFG_ADDR        0x08003000
#define configREAD_ADDR_ON_START    0x00
#define configSETUP_TIMEOUT         30 * 1000    // Setup timeout in ms (30 sec)

void readConfig(void);

#endif /* __CONFIG_H */
