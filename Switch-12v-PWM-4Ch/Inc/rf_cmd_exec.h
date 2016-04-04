#include "stm32f0xx_hal.h"
#include <stdint.h>
#include <stdio.h>

#ifndef __rf_CMD_EXEC_H
#define __rf_CMD_EXEC_H

#ifdef __cplusplus
 extern "C" {
#endif 

extern uint32_t MCUFreq;

void rfCmdExec(uint8_t *Data, uint8_t Length);

#ifdef __cplusplus
}
#endif

#endif /* __rf_CMD_EXEC_H */
