#include "stm32f0xx.h"
#include <stdint.h>

#ifndef __rf_CMD_H
#define __rf_CMD_H

#define PD_H_MASK_RESPONSE     0x10  //  Request/response bit
#define PD_H_MASK_CONFIRMATION 0x08  //  Needs confirmation 0 - no confirmation, 1 - confirmation needed
#define PD_H_MASK_N_PACKAGE    0x04  //  Number of package 0 - first, 1 second
#define PD_H_MASK_HUB_ID_LEN   0x02  //  hubID length 0 - 5 Bytes, 1 - 8 Bytes
#define PD_H_MASK_COMMAND_LEN  0x01  //  command length 0 - 1 Byte, 1 - more than 1 Byte

#define rfCMD_PING              0x00
#define rfCMD_DISCOVER          0x01
#define rfCMD_W_DATA            0x02
#define rfCMD_R_DATA            0x03
#define rfCMD_W_CONFIG          0x04
#define rfCMD_R_CONFIG          0x05


void ProcessData(uint8_t *Data, uint8_t Length);
void SaveAddress(uint8_t *Address, uint8_t *HubAddress);
void SendCommandToHub(uint8_t Command, uint8_t *Data, uint8_t Size);

#endif /* __rf_CMD_H */
