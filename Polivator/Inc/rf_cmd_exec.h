#include "stm32f0xx.h"

#ifndef __rf_CMD_EXEC_H
#define __rf_CMD_EXEC_H

void rfStartup(void);
void rfCmdWriteData(uint8_t *Data, uint8_t Length);
void rfCmdReadData(uint8_t *Data, uint8_t Length, uint8_t *ResponseData, uint8_t *ResponseLength);
void rfCmdWriteConfig(uint8_t *Data, uint8_t Length);
void rfCmdReadConfig(uint8_t *Data, uint8_t Length, uint8_t *ResponseData, uint8_t *ResponseLength);

void rfInternalCallback(uint8_t *Data, uint8_t Size);

#endif /* __rf_CMD_EXEC_H */
