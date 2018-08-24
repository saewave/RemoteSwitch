#ifndef __core_H
#define __core_H

#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "F103RE_Peripheral.h"
#include "at_processor.h"
#include "config.h"
#include "SI446x.h"
#include "rf_device.h"
#include "usblib.h"
#include "lib.h"

#define OUT_MESSAGE_STATUS_READY 0
#define OUT_MESSAGE_STATUS_TX 1

#define QUEUE_STATUS_OK 0
#define QUEUE_STATUS_FULL 1
#define QUEUE_STATUS_LENGTH 2

typedef struct sCMDMessage {
    volatile uint8_t Length;
             char    CMD[SAE_CMD_LENGTH];
} CMDMessage;

typedef struct rxDeviceData {
    volatile uint8_t Length;
             char    Data[Transceiver_BUFFER_LENGTH];
} DeviceRX;

typedef struct txDeviceData {
    volatile uint8_t Status;
    volatile uint16_t Address;
    volatile uint8_t Length;
             char    Data[Transceiver_BUFFER_LENGTH];
} DeviceTX;

typedef struct sOutMessage {
    volatile uint8_t Length;
    volatile uint8_t Status;
             char    Message[SAE_OUTPUT_DATA_LENGTH];
} OutMessage;

void QueueResponse(char *Response, uint8_t USART);
void ProcessCMDQueue(void);
void ProcessResponseQueue(void);
void ProcessDeviceRXQueue(void);
void ProcessDeviceTXQueue(void);
uint8_t AddToDeviceTXQueue(uint16_t Address, uint8_t * pBuf, uint8_t Length);
void IncreaseDeviceTXQueue(void);
void SetFWUpdateType(int Address);

#endif
