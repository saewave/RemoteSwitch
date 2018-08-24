/* 
 * This file is part of the RemoteSwitch distribution 
 * (https://github.com/saewave/RemoteSwitch).
 * Copyright (c) 2018 Samoilov Alexey.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
#include "core.h"

CMDMessage CMDMessageQueue[SAE_CMD_QUEUE_LENGTH];
OutMessage OutMessageQueue[2][SAE_OUTPUT_QUEUE_LENGTH];
DeviceRX DeviceRXQueue[SAE_DEVICE_RX_QUEUE_LENGTH];
DeviceTX DeviceTXQueue[SAE_DEVICE_TX_QUEUE_LENGTH];

volatile uint8_t CMDCurrentMessagePos = 0;
volatile uint8_t CMDProcessMessagePos = 0;

volatile uint8_t OutCurrentMessagePos[2] = {0,0};
volatile uint8_t OutDMAMessagePos[2] = {0,0};

volatile uint8_t DRXCurrentPos = 0;
volatile uint8_t DTXCurrentPos = 0;
volatile uint8_t DRXProcessPos = 0;
volatile uint8_t DTXProcessPos = 0;

volatile int FWUpdateAddress = 0x0000;

void QueueResponse(char *Response, uint8_t USART)
{
    if (USART == OUSART2) return;
    uint8_t  Length = strlen(Response);
    if (Length > SAE_OUTPUT_DATA_LENGTH)
        Length = SAE_OUTPUT_DATA_LENGTH;

    if (OutMessageQueue[USART][OutCurrentMessagePos[USART]].Length > 0)
        return;

    OutMessageQueue[USART][OutCurrentMessagePos[USART]].Length = Length;
    OutMessageQueue[USART][OutCurrentMessagePos[USART]].Status = OUT_MESSAGE_STATUS_READY;
    memcpy(OutMessageQueue[USART][OutCurrentMessagePos[USART]].Message, Response, Length);
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
#if SAE_ENABLE_USB == 1
            //Send data to USB. We assume that USART is more slowly as USB, so send data without additional queue
            USBLIB_Transmit((uint16_t *)OutMessageQueue[u][OutDMAMessagePos[u]].Message, (uint16_t)OutMessageQueue[u][OutDMAMessagePos[u]].Length);
#endif
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

void ProcessDeviceRXQueue(void) {
    if (DeviceRXQueue[DRXProcessPos].Length > 0) {
        rfProcessCommand(DeviceRXQueue[DRXProcessPos].Data, DeviceRXQueue[DRXProcessPos].Length);
        DeviceRXQueue[DRXProcessPos].Length = 0;
        DRXProcessPos++;
        if (DRXProcessPos == SAE_DEVICE_RX_QUEUE_LENGTH)
            DRXProcessPos = 0;
    }
}

uint8_t AddToDeviceTXQueue(uint16_t Address, uint8_t * pBuf, uint8_t Length) {
    if (Length > 0) {
        if (DeviceTXQueue[DTXCurrentPos].Length == 0) {
            memcpy(DeviceTXQueue[DTXCurrentPos].Data, pBuf, Length);
            DeviceTXQueue[DTXCurrentPos].Address = Address;
            DeviceTXQueue[DTXCurrentPos].Length = Length;
            DeviceTXQueue[DTXCurrentPos].Status = OUT_MESSAGE_STATUS_READY;
            DTXCurrentPos++;
            if (DTXCurrentPos == SAE_DEVICE_TX_QUEUE_LENGTH)
                DTXCurrentPos = 0;
        } else {
            return QUEUE_STATUS_FULL;
        }
    } else {
        return QUEUE_STATUS_LENGTH;
    }
    return QUEUE_STATUS_OK;
}

void ProcessDeviceTXQueue(void) {
    if (DeviceTXQueue[DTXProcessPos].Length > 0 && DeviceTXQueue[DTXProcessPos].Status == OUT_MESSAGE_STATUS_READY) {
        Transceiver_TxData(DeviceTXQueue[DTXProcessPos].Address, (uint8_t *)DeviceTXQueue[DTXProcessPos].Data, DeviceTXQueue[DTXProcessPos].Length);
        DeviceTXQueue[DTXProcessPos].Status = OUT_MESSAGE_STATUS_TX;
    }
}

void IncreaseDeviceTXQueue(void) {
    DeviceTXQueue[DTXProcessPos].Length = 0;
    DTXProcessPos++;
    if (DTXProcessPos == SAE_DEVICE_TX_QUEUE_LENGTH)
        DTXProcessPos = 0;
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
    uint8_t Status = Transceiver_HandleStatus();
    if (Transceiver_DataLength > 0) {
        memcpy(DeviceRXQueue[DRXCurrentPos].Data, Transceiver_Data, Transceiver_DataLength);
        DeviceRXQueue[DRXCurrentPos].Length = Transceiver_DataLength;
        Transceiver_DataLength = 0;
        DRXCurrentPos++;
        if (DRXCurrentPos == SAE_DEVICE_RX_QUEUE_LENGTH)
            DRXCurrentPos = 0;
    }
}

void uUSBLIB_DataReceivedHandler(uint16_t *Data, uint16_t Length)
{
    uint8_t *_Data = (uint8_t *)Data;
    if (CMDMessageQueue[CMDCurrentMessagePos].Length > 0){       //Queue if full
        QueueResponse("ER:014\n", OUSART1);
        return;                                                 //Drop incoming data
    }
    
    if (_Data[0] == SAE_CMD_BEGIN_HEXFW) {      //assume that ':' is a first symbol of HEX firmware
        memcpy(CMDMessageQueue[CMDCurrentMessagePos].CMD, Data, Length);
        CMDMessageQueue[CMDCurrentMessagePos].Length = Length;
        CMDCurrentMessagePos++;
    } 
    else if (_Data[0] == SAE_CMD_BEGIN_CHAR1 && _Data[1] == SAE_CMD_BEGIN_CHAR2 && _Data[2] == SAE_CMD_BEGIN_CHAR3)
    {
        memcpy(CMDMessageQueue[CMDCurrentMessagePos].CMD, &_Data[3], Length-3);
        CMDMessageQueue[CMDCurrentMessagePos].Length = Length-3;
        CMDCurrentMessagePos++;
    }
    if (CMDCurrentMessagePos == SAE_CMD_QUEUE_LENGTH)
        CMDCurrentMessagePos = 0;
}

/*void uTIM_IRQHandler(TIM_TypeDef *Tim)
{
    if (Tim == TIM1) {
        GPIOB->ODR ^= GPIO_ODR_ODR12;
    }
}
*/
