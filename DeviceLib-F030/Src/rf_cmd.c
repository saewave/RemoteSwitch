/**
  ******************************************************************************
  * @file    rf_cmd.c
  * @author  SaeWave Application Team: alexsam
  * @version V1.0
  * @brief   SaeWave Remote Command Control Source File.
  *          This file contains the parser for data that delivered by air
  *          and contain Header, Remote HUB address, useful data and CRC32 checksum.
  *          Parser implements next standard command: CMD_PING, CMD_DISCOVER, CMD_W_DATA, CMD_R_DATA, CMD_W_CONFIG, CMD_R_CONFIG
  *          for more information please refer to http://saewave.com/
  *
*/

#include "rf_cmd.h"
#include "config.h"
#include "nRF24L01P.h"
#include "rf_cmd_exec.h"
#include <string.h>

void ProcessData(uint8_t *Data, uint8_t Length)
{
    dxputs("ProcessData\n");
    for (int i = 0; i < Length; i++)
    {
        dxprintf("%x ", Data[i]);
    }
    dxputs("\n.....\n");

    uint8_t HUBAddress[5];
    uint8_t Header = Data[0]; //  Read Header
    uint8_t Pos    = 1;
    if (!(Header & PD_H_MASK_HUB_ID_LEN))
    { //  If HUB address length = 5Bytes
        memcpy(HUBAddress, &Data[Pos], 5);
        Pos += 5;
    }
    else
    {   //  Other HUB address length not implemented!
        //  Not implemented yet
    }

    uint8_t Command;

    if (!(Header & PD_H_MASK_COMMAND_LEN))
    { //  If command length = 1Byte
        Command = Data[Pos];
        Pos++;
    }
    else
    {   //  Other command length not implemented!
        //  Not implemented yet
    }

    switch (Command)
    {

    case rfCMD_PING:
    { // CMD: Ping
        /*
        uint8_t *DeviceAddress = nRF24_GetDeviceAddress();
        uint8_t tBuf[9];
        tBuf[0]   = 0x10;
        memcpy(&tBuf[1], DeviceAddress, 5);
        tBuf[6]   = rfCMD_PING;                         //Command
        tBuf[7]   = Data[7];
        tBuf[8]   = Data[8];
        nRF24_SetSwitchTo(nRF24_SWITCH_TO_RX);
        nRF24_TXPacket(HUBAddress, tBuf, 9);
        */
        SendCommandToHub(rfCMD_PING, &Data[7], 2);
    }
    break;

    case rfCMD_DISCOVER:
    { // CMD: Assign new address
        nRF24_SetHUBAddress(HUBAddress);

        nRF24_SetDeviceAddress(&Data[Pos], Data[Pos + 5]);
        Pos += 5;
        uint8_t *NewAddressAfter = nRF24_GetDeviceAddress();
        SaveAddress(NewAddressAfter, HUBAddress);

        uint8_t tBuf[6];
        memcpy(&tBuf[0], NewAddressAfter, 5);
        tBuf[5] = 0x01;                         // Set new address flag
        
        SendCommandToHub(rfCMD_DISCOVER, tBuf, 6);
    }
    break;

    case rfCMD_W_DATA:
    { // CMD: Write Data
        rfCmdWriteData(&Data[Pos], Length - Pos);
    }
    break;

    case rfCMD_R_DATA:
    { // CMD: Read Data
        uint8_t Response[24];
        uint8_t ResponseLength;
        rfCmdReadData(&Data[Pos], Length - Pos, Response, &ResponseLength);
        SendCommandToHub(rfCMD_R_DATA, Response, ResponseLength);
    }
    break;

    case rfCMD_W_CONFIG:
    { // CMD: Write config
        rfCmdWriteConfig(&Data[Pos], Length - Pos);
    }
    break;

    case rfCMD_R_CONFIG:
    { // CMD: Read config
        uint8_t Response[24];
        uint8_t ResponseLength;
        rfCmdReadConfig(&Data[Pos], Length - Pos, Response, &ResponseLength);
        SendCommandToHub(rfCMD_R_DATA, Response, ResponseLength);
    }
    break;
    }
}

void SaveAddress(uint8_t *Address, uint8_t *HubAddress)
{
    uint8_t Data[10];
    memcpy(&Data[0], Address, 5);
    memcpy(&Data[5], HubAddress, 5);

    FLASH_WriteData(configFLASH_CFG_ADDR, Data, 10, configFLASH_CFG_ADDR);
}

void SendCommandToHub(uint8_t Command, uint8_t *Data, uint8_t Size)
{
    uint8_t *HUBAddress    = nRF24_GetHUBAddress();
    uint8_t *DeviceAddress = nRF24_GetDeviceAddress();
    uint8_t  tBuf[32];
    tBuf[0] = 0x10;
    memcpy(&tBuf[1], DeviceAddress, 5);
    tBuf[6] = Command; //Command
    memcpy(&tBuf[7], Data, Size);
    dxprintf("**** CMD: %x, S: %d\n", Command, Size);
    nRF24_SetSwitchTo(nRF24_SWITCH_TO_RX);
    nRF24_TXPacket(HUBAddress, tBuf, 7 + Size);
}
