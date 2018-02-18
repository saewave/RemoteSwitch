#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "F103RE_Peripheral.h"
#include "at_processor.h"
#include "lib.h"
#include "config.h"
#include "CC1101.h"
#include "rf_device.h"
#include "usblib.h"

#define OUT_MESSAGE_STATUS_READY 0
#define OUT_MESSAGE_STATUS_TX 1

struct CMDMessage {
    volatile uint8_t Length;
             char    CMD[SAE_CMD_LENGTH];
};

struct DeviceData {
    volatile uint8_t Length;
             char    Data[CC1101_BUFFER_LENGTH];
};

struct OutMessage {
    volatile uint8_t Length;
    volatile uint8_t Status;
             char    Message[SAE_OUTPUT_DATA_LENGTH];
};

void QueueResponse(char *Response, uint8_t USART);
void ProcessCMDQueue(void);
void ProcessResponseQueue(void);
void ProcessDeviceDataQueue(void);
void SetFWUpdateType(int Address);
