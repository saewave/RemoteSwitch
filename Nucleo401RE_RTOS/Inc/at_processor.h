#ifndef __at_processor_H
#define __at_processor_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  uint8_t cLength;
  uint8_t *cData;
} xCmd;

typedef struct {
  uint8_t cLength;
  uint8_t *cData;
} xCmdResponse;

void ProcessATCommand(char *Data, uint8_t Length);
/*     Commands     */
void CmdPING(void);
void CmdFindNewDevice(void);
void CmdCheckTxRxStatus(uint8_t Chip);
void CmdHandleTxRxStatus(uint8_t Chip);

void QueueResponse(char *Response);

#endif /* __at_processor_H */
