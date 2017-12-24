#include "core.h"

struct CMDMessage CMDMessageQueue[SAE_CMD_QUEUE_LENGTH];
struct OutMessage OutMessageQueue[2][SAE_OUTPUT_QUEUE_LENGTH];
struct DeviceData DeviceDataQueue[SAE_DEVICE_DATA_QUEUE_LENGTH];

volatile uint8_t CMDCurrentMessagePos = 0;
volatile uint8_t CMDProcessMessagePos = 0;

volatile uint8_t OutCurrentMessagePos[2] = {0,0};
volatile uint8_t OutDMAMessagePos[2] = {0,0};

volatile uint8_t DDCurrentPos = 0;
volatile uint8_t DDProcessPos = 0;

volatile int FWUpdateAddress = 0x0000;

void QueueResponse(char *Response, uint8_t USART)
{
    uint8_t  Length = strlen(Response);
    if (Length > SAE_OUTPUT_DATA_LENGTH)
        Length = SAE_OUTPUT_DATA_LENGTH;

    if (OutMessageQueue[USART][OutCurrentMessagePos[USART]].Length > 0)
        return;

    memcpy(OutMessageQueue[USART][OutCurrentMessagePos[USART]].Message, Response, Length);
    OutMessageQueue[USART][OutCurrentMessagePos[USART]].Length = Length;
    OutMessageQueue[USART][OutCurrentMessagePos[USART]].Status = OUT_MESSAGE_STATUS_READY;
    OutCurrentMessagePos[USART]++;
    if (OutCurrentMessagePos[USART] == SAE_OUTPUT_QUEUE_LENGTH)
        OutCurrentMessagePos[USART] = 0;
}

void ProcessResponseQueue(void) {
    USART_TypeDef *USART;
    for (uint8_t u = 0; u <= OUSART2; u++) {
        USART = u == OUSART1 ? USART1 : USART2;
        if (OutMessageQueue[u][OutDMAMessagePos[u]].Length > 0 && OutMessageQueue[u][OutDMAMessagePos[u]].Status == OUT_MESSAGE_STATUS_READY) {
            OutMessageQueue[u][OutDMAMessagePos[u]].Status = OUT_MESSAGE_STATUS_TX;
            USART_DMASendData(USART, OutMessageQueue[u][OutDMAMessagePos[u]].Message, OutMessageQueue[u][OutDMAMessagePos[u]].Length);
        }
    }
}

void ProcessCMDQueue(void) {
    if (CMDMessageQueue[CMDProcessMessagePos].Length > 0) {
        if (CMDMessageQueue[CMDProcessMessagePos].CMD[0] == ':') {      //Process Intel HEX data
            if (FWUpdateAddress == 0x0000) {
                FWWriteToFlash((uint8_t *)&CMDMessageQueue[CMDProcessMessagePos].CMD[1]);
            } else {
                rfSendFWHex(FWUpdateAddress, (uint8_t *)&CMDMessageQueue[CMDProcessMessagePos].CMD[1], CMDMessageQueue[CMDProcessMessagePos].Length - 1);
            }
        } else {
            ProcessATCommand(CMDMessageQueue[CMDProcessMessagePos].CMD, CMDMessageQueue[CMDProcessMessagePos].Length);
        }
        CMDMessageQueue[CMDProcessMessagePos].Length = 0;
        CMDProcessMessagePos++;
        if (CMDProcessMessagePos == SAE_CMD_QUEUE_LENGTH)
            CMDProcessMessagePos = 0;
    }
}

void ProcessDeviceDataQueue(void) {
    if (DeviceDataQueue[DDProcessPos].Length > 0) {
        rfProcessCommand(DeviceDataQueue[DDProcessPos].Data, DeviceDataQueue[DDProcessPos].Length);
        DeviceDataQueue[DDProcessPos].Length = 0;
        DDProcessPos++;
        if (DDProcessPos == SAE_DEVICE_DATA_QUEUE_LENGTH)
            DDProcessPos = 0;
    }
}

void SetFWUpdateType(int Address) {
    FWUpdateAddress = Address;
}
    
void uUSART_IRQHandler(USART_TypeDef *USART, uint32_t SR, uint8_t *Data, uint8_t Length) {
    if (SR & USART_SR_TC) {
        uint8_t u = USART == USART1 ? OUSART1 : OUSART2;
        if (OutMessageQueue[u][OutDMAMessagePos[u]].Length > 0) {
            OutMessageQueue[u][OutDMAMessagePos[u]].Length = 0;
            OutMessageQueue[u][OutDMAMessagePos[u]].Status = OUT_MESSAGE_STATUS_READY;
            OutDMAMessagePos[u]++;
            if (OutDMAMessagePos[u] == SAE_OUTPUT_QUEUE_LENGTH)
                OutDMAMessagePos[u] = 0;
        }
    }
    if (SR & USART_SR_IDLE && Length > 0)
    {
        if (CMDMessageQueue[CMDCurrentMessagePos].Length > 0){       //Queue if full
            QueueResponse("ER:014\n", OUSART1);
            return;                                                 //Drop incoming data
        }
        
        if (Data[0] == SAE_CMD_BEGIN_HEXFW) {      //assume that ':' is a first symbol of HEX firmware
            memcpy(CMDMessageQueue[CMDCurrentMessagePos].CMD, Data, Length);
            CMDMessageQueue[CMDCurrentMessagePos].Length = Length;
            CMDCurrentMessagePos++;
        } 
        else if (Data[0] == SAE_CMD_BEGIN_CHAR1 && Data[1] == SAE_CMD_BEGIN_CHAR2 && Data[2] == SAE_CMD_BEGIN_CHAR3)
        {
            memcpy(CMDMessageQueue[CMDCurrentMessagePos].CMD, &Data[3], Length-3);
            CMDMessageQueue[CMDCurrentMessagePos].Length = Length-3;
            CMDCurrentMessagePos++;
        }
        if (CMDCurrentMessagePos == SAE_CMD_QUEUE_LENGTH)
            CMDCurrentMessagePos = 0;
    }
}

void uEXTI_IRQHandler(uint32_t Pin)
{
    CC1101_HandleStatus();
    if (CC1101_DataLength > 0) {
        memcpy(DeviceDataQueue[DDCurrentPos].Data, CC1101_Data, CC1101_DataLength);
        DeviceDataQueue[DDCurrentPos].Length = CC1101_DataLength;
        CC1101_DataLength = 0;
        DDCurrentPos++;
        if (DDCurrentPos == SAE_DEVICE_DATA_QUEUE_LENGTH)
            DDCurrentPos = 0;
    }
//    rfProcessCommand(CC1101_Data, Length);
}
