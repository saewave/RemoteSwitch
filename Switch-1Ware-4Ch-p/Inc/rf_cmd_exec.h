#include "stm32f0xx.h"

#ifndef __rf_CMD_EXEC_H
#define __rf_CMD_EXEC_H

void rfStartup(void);

void rfCmdExec(uint8_t *Data, uint8_t Length);
void rfInternalCallback(uint8_t *Data, uint8_t Size);

#endif /* __rf_CMD_EXEC_H */
