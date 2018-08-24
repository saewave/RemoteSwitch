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
 
#include "SI446x.h"
#include "F103RE_Peripheral.h"
#include "SI4463.h"
#include "rf_processor.h"

uint8_t          Transceiver_Data[Transceiver_BUFFER_LENGTH];
volatile uint8_t Transceiver_DataLength                                    = 0;
uint8_t          Transceiver_HUB_ADDR[Transceiver_ADDR_LENGTH]             = {0xEC, 0xA2};
uint8_t          Transceiver_DEVICE_DISCOVER_ADDR[Transceiver_ADDR_LENGTH] = {0xF0, 0xCF};
volatile uint8_t Transceiver_TransmiterState                               = Transceiver_TR_STATE_IDLE;
volatile uint8_t Configured                                                = 0;

void Transceiver_RxMode(void)
{
    Transceiver_WriteCommand(Transceiver_CHANGE_STATE, (uint8_t *)0x03, 1); // Change to READY state
    Transceiver_SetSyncWord(Transceiver_HUB_ADDR[0], Transceiver_HUB_ADDR[1]);
    uint8_t Params[7] = {
        0x00, //Channel
        0x00, //Start (immediately)
        0x00, //RX_LEN[12:8]
        0x00, //RX_LEN[7:0]
        8,    //RXTIMEOUT_STATE(RX state)
        3,    //RXVALID_STATE(READY state)
        8     //RXINVALID_STATE(RX state)
    };
    Transceiver_WriteCommand(Transceiver_START_RX, Params, 7);
}

void Transceiver_ClearFifo(uint8_t Fifo) {
    Transceiver_ReadRegs(Transceiver_FIFO_INFO, &Fifo, 1, NULL, 0, 1);
}

void    Transceiver_Configure(void)
{
    const uint8_t radioConfiguration[] = RADIO_CONFIGURATION_DATA_ARRAY;
    uint16_t      pos                  = 0;
    while (radioConfiguration[pos] > 0) {
        Transceiver_WriteCommand(radioConfiguration[pos + 1], (uint8_t *)&radioConfiguration[pos + 2], radioConfiguration[pos] - 1);
        pos += radioConfiguration[pos] + 1;
    };
    Configured = 1;
}

uint8_t Transceiver_IsConfigured(void)
{
    return Configured;
}

void Transceiver_WriteReg(uint8_t Reg, uint8_t Value)
{
    CSN_LOW();
    Delay_us(5);
    SPI_SendData(SPI1, &Reg, 1);
    SPI_SendData(SPI1, &Value, 1);
    Delay_us(5);
    CSN_HIGH();
}

void Transceiver_WriteCommand(uint8_t Command, uint8_t *Params, uint8_t Length)
{
    while ((Transceiver_CTS_PORT->IDR & Transceiver_CTS_PIN) == 0) {
        __NOP();
    };
    CSN_LOW();
    Delay_us(5);
    SPI_SendData(SPI1, &Command, 1);
    SPI_SendData(SPI1, Params, Length);
    Delay_us(5);
    CSN_HIGH();
    while ((Transceiver_CTS_PORT->IDR & Transceiver_CTS_PIN) == 0) {
        __NOP();
    };
}

uint8_t Transceiver_ReadFastReg(uint8_t Reg)
{
    uint8_t Value;
    CSN_LOW();
    SPI_SendData(SPI1, &Reg, 1);
    SPI_ReadData(SPI1, &Value, 1, 0x00);
    Delay_us(5);
    CSN_HIGH();
    return Value;
}

void Transceiver_ReadState(uint8_t Reg, uint8_t *Buff, uint8_t Length)
{
    while ((Transceiver_CTS_PORT->IDR & Transceiver_CTS_PIN) == 0) {
        __NOP();
    };
    CSN_LOW();
    SPI_SendData(SPI1, &Reg, 1);
    Delay_us(5);
    CSN_HIGH();
    while ((Transceiver_CTS_PORT->IDR & Transceiver_CTS_PIN) == 0) {
        __NOP();
    };
    CSN_LOW();
    SPI_ReadData(SPI1, Buff, Length, 0x00);
    Delay_us(5);
    CSN_HIGH();
    Delay_us(5);
}

void Transceiver_ReadRegs(uint8_t Reg, uint8_t *Params, uint8_t pLength, uint8_t *Buff, uint8_t bLength, uint8_t WaitCTS)
{
    uint8_t CTSBuf[2] = {0x44, 0x00};
    while ((Transceiver_CTS_PORT->IDR & Transceiver_CTS_PIN) == 0) {
        __NOP();
    };
    CSN_LOW();
    SPI_SendData(SPI1, &Reg, 1);
    if (pLength > 0) {
        SPI_SendData(SPI1, Params, pLength);
    }
    Delay_us(5);
    CSN_HIGH();
    while ((Transceiver_CTS_PORT->IDR & Transceiver_CTS_PIN) == 0) {
        __NOP();
    };
    if (bLength > 0) {
        CSN_LOW();
        SPI_SendData(SPI1, CTSBuf, 2);
        SPI_ReadData(SPI1, Buff, bLength, 0x00);
        Delay_us(5);
        CSN_HIGH();
        Delay_us(5);
    }
}

void Transceiver_WriteTXBuf(uint8_t *Buf, uint8_t Length)
{
    uint8_t CMD = Transceiver_WRITE_TX_FIFO;
    while ((Transceiver_CTS_PORT->IDR & Transceiver_CTS_PIN) == 0) {
        __NOP();
    };
    CSN_LOW();
    Delay_us(5);
    SPI_SendData(SPI1, &CMD, 1);
    SPI_SendData(SPI1, Buf, Length);
    Delay_us(5);
    CSN_HIGH();
    Delay_us(5);
}

void Transceiver_ReadRXBuf(uint8_t *Buf, uint8_t Length)
{
    uint8_t CMD = Transceiver_READ_RX_FIFO;
    while ((Transceiver_CTS_PORT->IDR & Transceiver_CTS_PIN) == 0) {
        __NOP();
    };
    CSN_LOW();
    SPI_SendData(SPI1, &CMD, 1);
    SPI_ReadData(SPI1, Buf, Length, 0x00);
    Delay_us(5);
    CSN_HIGH();
    Delay_us(5);
}

void Transceiver_StartTX(uint8_t Length)
{
    uint8_t CMD[5] = {
        Transceiver_START_TX,
        0,     //Channel
        0x30,  //READY state.
        0,     //TX_LEN[12:8]
        Length //TX_LEN[7:0]
    };

    while ((Transceiver_CTS_PORT->IDR & Transceiver_CTS_PIN) == 0) {
        __NOP();
    };
    CSN_LOW();
    Delay_us(5);
    SPI_SendData(SPI1, CMD, 5);
    Delay_us(5);
    CSN_HIGH();
    while ((Transceiver_CTS_PORT->IDR & Transceiver_CTS_PIN) == 0) {
        __NOP();
    };
}

void Transceiver_ChangeState(uint8_t State)
{
    Transceiver_WriteCommand(Transceiver_CHANGE_STATE, &State, 1);
}

void Transceiver_TxData(uint16_t Address, uint8_t *Buf, uint8_t Length)
{
    uint8_t Params = 0x01;

    uint8_t tBuf[Length + 1];
    tBuf[0] = Length + 2;
    if (Length > 0)
        memcpy(&tBuf[1], Buf, Length);

    Transceiver_ChangeState(Transceiver_STATE_READY);
    /* ********** Clear TX Buff ********* */
    Transceiver_ReadRegs(Transceiver_FIFO_INFO, &Params, 1, NULL, 0, 1);
    Transceiver_TransmiterState = Transceiver_TR_STATE_TX;
    Transceiver_SetSyncWord((uint8_t)(Address >> 8), (uint8_t)Address);
    Transceiver_WriteTXBuf(tBuf, Length + 1);
    Transceiver_StartTX(Length + 1);
}

void Transceiver_SetSyncWord(uint8_t Word0, uint8_t Word1)
{
    uint8_t config[8] = {0x11, 0x05, 0x00, 0x01, Word0, Word1, Word0, Word1};
    Transceiver_WriteCommand(0x11, config, 8);
}

void Transceiver_ReadFRR(uint8_t Reg, uint8_t Length, uint8_t * Registers) {
    CSN_LOW();
    Delay_us(5);
    SPI_SendData(SPI1, &Reg, 1);
    SPI_ReadData(SPI1, Registers, Length, 0x00);
    Delay_us(5);
    CSN_HIGH();
}

uint8_t Transceiver_HandleStatus(void)
{
    uint8_t Params = 0x01;
    uint8_t ClearRX       = 0;
    uint8_t PacketInfo[2] = {0, 0};
    uint8_t FIFOStatus[2] = {0, 0};
    uint8_t PHStatus[2]   = {0};
    Transceiver_ReadRegs(Transceiver_GET_PH_STATUS, NULL, 0, PHStatus, 2, 1);

    if (PHStatus[Transceiver_PH_STATUS_PH_PEND] & Transceiver_PH_STATUS_PACKET_RX_PEND) {   //Got packet
        Transceiver_ReadRegs(Transceiver_PACKET_INFO, NULL, 0, PacketInfo, 2, 1);           //Read info about data received
        if (PacketInfo[1] > 0 && PacketInfo[1] < 64) {                                      //If git valid packet...
            Transceiver_ReadRXBuf(Transceiver_Data, PacketInfo[1]);                         //Read packet into Transceiver_Data
            Transceiver_DataLength = PacketInfo[1] - Transceiver_PH_CRC_LENGTH;                                         //And set current length
        } else {
            ClearRX = 1;
        }
    }

    if (PHStatus[Transceiver_PH_STATUS_PH_PEND] & Transceiver_PH_STATUS_PACKET_SENT_PEND) { //If packet transmited
        Transceiver_ReadRegs(Transceiver_FIFO_INFO, &Params, 1, NULL, 0, 1);        //Read data about FIFO
        IncreaseDeviceTXQueue();                                                            //Goto next packet in queue
//        Transceiver_ReadRegs(0x12, tParams, 3, cResponse, 12, 1);
    }
    
    if (PHStatus[Transceiver_PH_STATUS_PH_PEND] & Transceiver_PH_STATUS_PACKET_CRC_ERROR_PEND) { //If CRC error
        Transceiver_ReadRegs(Transceiver_FIFO_INFO, &Params, 1, FIFOStatus, 2, 1);               //Read data about FIFOs
        if (FIFOStatus[0] > 0) {
            uint8_t BrokenRX[65] = {0};
            Transceiver_ReadRXBuf(BrokenRX, FIFOStatus[0]);
            BrokenRX[64] = 0;
            QueueResponse("ER:CRC", OUSART1);
//            QueueResponse((char *)BrokenRX, OUSART1);
        }
    }
    
    if (PHStatus[Transceiver_PH_STATUS_PH_PEND] & Transceiver_PH_STATUS_RX_FIFO_ALMOST_FULL_PEND) {
        Params = 0x02;
        Transceiver_ReadRegs(Transceiver_FIFO_INFO, &Params, 1, NULL, 0, 1); //Clear RX FIFO. Generally, this should not happen, but for some safe side...
        #if SAE_ALL_TIME_RX_MODE == 1
            Transceiver_RxMode();
        #endif
    }

    UNUSED(ClearRX);
    if (ClearRX) {
        // Clear the RX buf
        Transceiver_ReadRegs(Transceiver_FIFO_INFO, (uint8_t *)0x02, 1, NULL, 0, 1);
        Transceiver_ReadRegs(Transceiver_GET_INT_STATUS, NULL, 0, NULL, 0, 1);
    }
    Transceiver_RxMode();

    return PacketInfo[1];
}
