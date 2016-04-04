#include "stm32f4xx_hal.h"
#include <stdlib.h>
#include <stdio.h>

#ifndef __CMD_EXEC_H_H
#define __CMD_EXEC_H_H

#ifdef __cplusplus
 extern "C" {
#endif 

void SendCommand(uint8_t *DeviceAddr, uint8_t *Command, uint8_t *Data);

#ifdef __cplusplus
}
#endif

#endif /* __CMD_EXEC_H_H */
