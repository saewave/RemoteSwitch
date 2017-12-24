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
void CmdCheckRegister(uint8_t Reg);
void CmdHandleTxRxStatus(void);
void FWPrepareDeviceFlash(uint16_t Address);

#endif /* __at_processor_H */
