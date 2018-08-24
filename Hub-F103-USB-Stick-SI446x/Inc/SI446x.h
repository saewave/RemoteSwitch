#include "stm32f10x.h"
#include <stdint.h>
#include <string.h>

#include "stm32f10x.h"
#include "F103RE_Peripheral.h"
#include "at_processor.h"
#include "rf_processor.h"
#include "core.h"
#include "xdebug.h"

/////////////////////// NAMES /////////////////////////
#define Transceiver_PH_STATUS_PH_PEND 0x00
#define Transceiver_PH_PEND 0x02
#define Transceiver_PH_STATUS 0x03
/////////////////////// BITS //////////////////////////
#define Transceiver_PH_STATUS_PACKET_RX_PEND 0x10
#define Transceiver_PH_STATUS_PACKET_SENT_PEND 0x20
#define Transceiver_PH_STATUS_PACKET_CRC_ERROR_PEND 0x08

#define Transceiver_PH_STATUS_PACKET_RX 0x10
#define Transceiver_PH_STATUS_RX_FIFO_ALMOST_FULL_PEND 0x01
#define Transceiver_PACKET_INFO      0x16

///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////

//#define CCxxx0_SYNC_WORD0 0xE3
//#define CCxxx0_SYNC_WORD1 0x66

#define Transceiver_ADDR_LENGTH 2
#define Transceiver_PAYLOAD_LENGTH 58
#define Transceiver_BUFFER_LENGTH 64
#define Transceiver_PH_CRC_LENGTH 2
#define Transceiver_ADD_RSSI_LQI 1

#define Transceiver_TR_STATE_IDLE 0
#define Transceiver_TR_STATE_TX 1
#define Transceiver_TR_STATE_RX 2
#define Transceiver_STATE_READY 0x03

///////////////////Commands
#define Transceiver_PART_INFO        0x01
#define Transceiver_FIFO_INFO        0x15
#define Transceiver_GET_INT_STATUS   0x20
#define Transceiver_GET_PH_STATUS    0x21
#define Transceiver_START_TX         0x31
#define Transceiver_START_RX         0x32
#define Transceiver_REQUEST_DEVICE_STATE 0x33
#define Transceiver_CHANGE_STATE     0x34
#define Transceiver_WRITE_TX_FIFO    0x66
#define Transceiver_READ_RX_FIFO     0x77

extern uint8_t Transceiver_HUB_ADDR[Transceiver_ADDR_LENGTH];
extern uint8_t Transceiver_DEVICE_DISCOVER_ADDR[Transceiver_ADDR_LENGTH];
extern uint8_t Transceiver_Data[Transceiver_BUFFER_LENGTH];
extern volatile uint8_t Transceiver_DataLength;

void Transceiver_Reset(void);
void Transceiver_Configure(void);
void Transceiver_TxMode(void);
void Transceiver_RxMode(void);
void Transceiver_WriteReg(uint8_t Reg, uint8_t Value);
void Transceiver_WriteCommand(uint8_t Command, uint8_t *Params, uint8_t Length);

uint8_t Transceiver_ReadConfigReg(uint8_t Reg);
uint8_t Transceiver_ReadFastReg(uint8_t Reg);
void Transceiver_ReadState(uint8_t Reg, uint8_t *Buff, uint8_t Length);
void Transceiver_ReadRegs(uint8_t Reg, uint8_t *Params, uint8_t pLength, uint8_t *Buff, uint8_t bLength, uint8_t WaitCTS);
void Transceiver_SetSyncWord(uint8_t Word0, uint8_t Word1);
uint8_t Transceiver_HandleStatus(void);
void Transceiver_TxTestData(void);
void Transceiver_WriteTXBuf(uint8_t *Buf, uint8_t Length);
void Transceiver_ReadRXBuf(uint8_t *Buf, uint8_t Length);
void Transceiver_StartTX(uint8_t Length);
void Transceiver_TxData(uint16_t Address, uint8_t * Buf, uint8_t Length);


