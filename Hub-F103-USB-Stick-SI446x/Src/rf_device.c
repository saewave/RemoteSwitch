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
 
#include "rf_device.h"
#include "at_processor.h"
#include "config.h"
#include "SI446x.h"
#include "xdebug.h"
#include "core.h"

xDevice rfDevices[SAE_DEVICE_LIST_LENGTH] = {0};
uint8_t rfDevicePosition = 0xFF;

uint8_t rfFindFreeDevicePosition() {
    for (rfDevicePosition = 0; rfDevicePosition < SAE_DEVICE_LIST_LENGTH; rfDevicePosition++) {
        if (rfDevices[rfDevicePosition].Address == 0x0000) {
            return rfDevicePosition;
        }
    }
    return SAE_DEVICE_LIST_LENGTH;
}

xDevice * rfCreateDevice(void)
{
    uint8_t CurPos = rfFindFreeDevicePosition();
    if (CurPos == SAE_DEVICE_LIST_LENGTH) {
        dxprintf("ER: can't allocate memory!\n");
        return NULL;
    }
    rfDevices[CurPos].Address = rfDEVICE_START_ADDRESS + CurPos;
    rfDevices[CurPos].Type = 0x00;
    rfDevices[CurPos].Config = 0x00;
    rfDevices[CurPos].Salt   = rfGetSalt();
    return &rfDevices[CurPos];
}

xDevice * rfRegisterDevice(uint16_t Address, uint8_t Type)
{
    uint8_t CurPos = rfFindFreeDevicePosition();
    if (CurPos == SAE_DEVICE_LIST_LENGTH) {
        dxprintf("ER: can't allocate memory!\n");
        return NULL;
    }
    if (Address != 0) {
        rfDevices[CurPos].Address = Address;
    } else {
        rfDevices[CurPos].Address = rfDEVICE_START_ADDRESS + CurPos;
    }
    rfDevices[CurPos].Type = 0x00;
    rfDevices[CurPos].Config = 0x00;
    rfDevices[CurPos].Salt   = rfGetSalt();
    return &rfDevices[CurPos];
}

void rfListDevices(void)
{
    char    Buf[100];
    QueueResponse("OK:LIST_DEVICES\n", OUSART1);
    for (rfDevicePosition = 0; rfDevicePosition < SAE_DEVICE_LIST_LENGTH; rfDevicePosition++) {
        if (rfDevices[rfDevicePosition].Address == 0) break;
        sprintf(Buf, "%d, Adr: %04x Type: %02x, Conf: %x, Sa: %x\n", rfDevicePosition, rfDevices[rfDevicePosition].Address, rfDevices[rfDevicePosition].Type, rfDevices[rfDevicePosition].Config, rfDevices[rfDevicePosition].Salt);
        QueueResponse(Buf, OUSART1);
    };
    sprintf(Buf, "COUNT:%d\n", rfDevicePosition - 1);
    QueueResponse(Buf, OUSART1);
}

xDevice * rfUpdateDevice(uint16_t Address, uint8_t Type, uint8_t Config)
{
    xDevice * Device = rfGetDevice(Address);
    if (Device != NULL) {
        Device->Config |= Config;
    }
    return Device;
}

xDevice * rfGetDevice(uint16_t Address)
{
    for (rfDevicePosition = 0; rfDevicePosition < SAE_DEVICE_LIST_LENGTH; rfDevicePosition++) {
        if (rfDevices[rfDevicePosition].Address == Address) {
            return &rfDevices[rfDevicePosition];
        }
    }
    return NULL;
}

uint8_t rfRemoveDevices(uint16_t Address)
{
    for (rfDevicePosition = 0; rfDevicePosition < SAE_DEVICE_LIST_LENGTH; rfDevicePosition++) {
        if (rfDevices[rfDevicePosition].Address == Address) {
            rfDevices[rfDevicePosition].Address = 0;
            rfDevices[rfDevicePosition].Type = 0;
            rfDevices[rfDevicePosition].Config = 0;
            rfDevices[rfDevicePosition].Salt = 0;
            return rfDevicePosition;
        }
    }
    return SAE_DEVICE_LIST_LENGTH;
}

void rfSaveDevices(void)
{
    //0x4002 3C00 - 0x4002 3FFF
    // Unlock flash to erase and write
    uint8_t HaveToStore = 0;
    for (rfDevicePosition = 0; rfDevicePosition < SAE_DEVICE_LIST_LENGTH; rfDevicePosition++) {
        if (rfDevices[rfDevicePosition].Address != 0x0000) {
            HaveToStore++;
        }
    }
    if (HaveToStore == 0)
    {
        QueueResponse((char *)"ER:010\n", OUSART1); //Nothing to store
        return;
    }
    /**  Flash pages */
    FLASH->KEYR = FLASH_FKEY1;
    FLASH->KEYR = FLASH_FKEY2;

    while (FLASH->SR & FLASH_SR_BSY)
    {
        __NOP();
    }; // Wait untill memory ready

    FLASH->CR |= FLASH_CR_PER;              // Erase one page
    FLASH->AR |= SAE_DEVICES_MEMORY_ADDRESS; // Erase address
    FLASH->CR |= FLASH_CR_STRT;

    while (FLASH->SR & FLASH_SR_BSY)
    {
        __NOP();
    }; // Wait untill memory ready
    FLASH->CR &= ~FLASH_CR_PER;
    while (FLASH->SR & FLASH_SR_BSY)
    {
        __NOP();
    };
    FLASH->CR |= FLASH_CR_PG; // Allow flash programming
    while (FLASH->SR & FLASH_SR_BSY)
    {
        __NOP();
    };

    uint32_t Address = SAE_DEVICES_MEMORY_ADDRESS;
    for (rfDevicePosition = 0; rfDevicePosition < SAE_DEVICE_LIST_LENGTH; rfDevicePosition++) {
        if (rfDevices[rfDevicePosition].Address == 0x0000) {
            continue;
        }
        *(__IO uint16_t *)Address = (uint16_t)rfDevices[rfDevicePosition].Address;
        while (FLASH->SR & FLASH_SR_BSY)
        {
            __NOP();
        };
        Address += 2;
        *(__IO uint16_t *)Address = (uint16_t)((uint16_t)rfDevices[rfDevicePosition].Type | ((uint16_t)rfDevices[rfDevicePosition].Config << 8));
        while (FLASH->SR & FLASH_SR_BSY)
        {
            __NOP();
        };
        Address += 2;
        *(__IO uint16_t *)Address = (uint16_t)rfDevices[rfDevicePosition].Salt;
        while (FLASH->SR & FLASH_SR_BSY)
        {
            __NOP();
        };
        Address += 2;
        *(__IO uint16_t *)Address = (uint16_t)(rfDevices[rfDevicePosition].Salt >> 16);
        while (FLASH->SR & FLASH_SR_BSY)
        {
            __NOP();
        };
    };

    while (FLASH->SR & FLASH_SR_BSY)
    {
        __NOP();
    };
    FLASH->CR &= ~(FLASH_CR_PG);
    FLASH->CR |= FLASH_CR_LOCK;
}

void rfLoadDevices(void)
{
    uint32_t Address       = SAE_DEVICES_MEMORY_ADDRESS;
    if (*(__IO uint16_t *)Address == 0xFFFF || *(__IO uint16_t *)Address == 0x0000) {
        return;
    }
    
    for (rfDevicePosition = 0; rfDevicePosition < SAE_DEVICE_LIST_LENGTH; rfDevicePosition++) {
        if (*(__IO uint16_t *)Address == 0xFFFF || *(__IO uint16_t *)Address == 0x0000) break;
        rfDevices[rfDevicePosition].Address = *(__IO uint16_t *)Address;
        rfDevices[rfDevicePosition].Type   = *(__IO uint8_t *)(Address + 2);
        rfDevices[rfDevicePosition].Config = *(__IO uint8_t *)(Address + 3);
        rfDevices[rfDevicePosition].Salt   = *(__IO uint32_t *)(Address + 4);
        Address += 8;
    }
}

void rfPingDevice(uint16_t Address)
{
    xDevice * Device = rfGetDevice(Address);
    if (Device == NULL)
    {
        QueueResponse((char *)"ER:006\n", OUSART1);   //Device not registered!
        return;
    }

    uint8_t Data[2] = {0x00, 0x01};
    rfSendCommad(rfCMD_PING, Address, Data, 2, Device->Salt);
}

void rfPingAllDevices(void)
{

    
    uint8_t Data[2] = {0x00, 0x01};
    for (rfDevicePosition = 0; rfDevicePosition < SAE_DEVICE_LIST_LENGTH; rfDevicePosition++) {
        if (rfDevices[rfDevicePosition].Address == 0x0000 || rfDevices[rfDevicePosition].Type == 0x00) {
            continue;
        }
        rfSendCommad(rfCMD_PING, rfDevices[rfDevicePosition].Address, Data, 2, rfDevices[rfDevicePosition].Salt);
    }
}

void rfSendFWHex(uint16_t Address, uint8_t *Data, uint8_t Length)
{
    xDevice * Device = rfGetDevice(Address);
    if (Device == NULL)
    {
        QueueResponse((char *)"ER:006\n", OUSART1);   //Device not registered!
        return;
    }

    rfSendCommad(rfCMD_FW_UPDATE, Address, Data, Length, Device->Salt);
}

void rfSendData(uint8_t Cmd, xDevice * Device, char *Parameters)
{
    switch (Device->Type)
    {
    case rfDEVICE_TYPE_1:
    case rfDEVICE_TYPE_2:
    {
        if (Parameters == 0) {
            rfSendCommad(Cmd, Device->Address, 0, 0, Device->Salt);
        } else {
            int NewValue[21] = {0x00};
            int readVals     = sscanf(Parameters, rfCMD_DATA_MASK_HEX,
                                  &NewValue[0],
                                  &NewValue[1],
                                  &NewValue[2],
                                  &NewValue[3],
                                  &NewValue[4],
                                  &NewValue[5],
                                  &NewValue[6],
                                  &NewValue[7],
                                  &NewValue[8],
                                  &NewValue[9],
                                  &NewValue[10],
                                  &NewValue[11],
                                  &NewValue[12],
                                  &NewValue[13],
                                  &NewValue[14],
                                  &NewValue[15],
                                  &NewValue[16],
                                  &NewValue[17],
                                  &NewValue[18],
                                  &NewValue[19],
                                  &NewValue[20]);
            if (readVals > 0)
            {
                uint8_t Data[21] = {0x00};
                for (int i = 0; i < readVals; i++)
                {
                    Data[i] = (uint8_t)NewValue[i];
                }
                rfSendCommad(Cmd, Device->Address, Data, readVals, Device->Salt);
            }
        }
    }
    break;
    default:
    {
        char Buf[50];
        sprintf(Buf, "ER:009,%2x\n", Device->Type);
        QueueResponse(&Buf[0], OUSART1);
    }
    break;
    }
}

void rfSendCommad(uint8_t Command, uint16_t Address, uint8_t *Data, uint8_t Length, uint32_t Salt)
{
    if (Length > Transceiver_PAYLOAD_LENGTH)
    {
        dxprintf("ER:008\n");     //Data length can't be more than Transceiver_PAYLOAD_LENGTH bytes
    }

    uint8_t pBuf[Length + 4 + 4];
    pBuf[0]       = 0x00; //Header, default 0x00
    pBuf[1]       = Transceiver_HUB_ADDR[0];
    pBuf[2]       = Transceiver_HUB_ADDR[1];
    pBuf[3]       = Command; //Cmd

    if (Length > 0) {
        memcpy(&pBuf[4], Data, Length);
    }

    //uint32_t crcRes = rfCalcCRC32(pBuf, Length + 4);
    uint32_t crcRes = 0x12ABCDEF;
    memcpy(&pBuf[Length + 4], &crcRes, 4);

//    Transceiver_TxData(Address, pBuf, Length + 4 + 4);
    AddToDeviceTXQueue(Address, pBuf, Length + 4 + 4);
}

void rfPrepareTestDevices(void)
{
    //TODO: Must be refactored! {{{
    const int Length            = 2;
    uint16_t  Addresses[Length] = {0x0001, 0x0002};
    uint8_t   Types[Length]     = {0x01, 0x02};

    for (int i = 0; i < Length; i++)
    {
        xDevice * newDevice = rfCreateDevice();
        rfUpdateDevice(Addresses[i], Types[i], 0x00);
    }

    rfListDevices();
    // }}}
}

uint32_t rfCalcCRC32(uint8_t *Data, uint8_t Length)
{
    uint32_t cnt;
    CRC->CR = CRC_CR_RESET;
    /* Calculate number of 32-bit blocks */
    cnt = Length >> 2;
    /* Calculate */
    while (cnt--)
    {
        /* Set new value */
        CRC->DR = *(uint32_t *)Data;

        /* Increase by 4 */
        Data += 4;
    }
    /* Calculate remaining data as 8-bit */
    cnt = Length % 4;
    while (cnt--)
    {
        *((uint8_t *)&CRC->DR) = *Data++;
    }

    return CRC->DR;
}

void rfSetFWUpdateType(int Address) {
    SetFWUpdateType(Address);
}

uint32_t rfGetSalt(void)
{
    return 0xAABBCCDD; //TODO: Should be replaced to random 32bit number!
}
