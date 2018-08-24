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
 
#include "at_processor.h"
#include "SI446x.h"
#include "rf_device.h"
#include "lib.h"
#include "core.h"
#include <stdio.h>
#include <stdlib.h>

/**
 *  \brief Parse user entered command and process it
 *  
 *  \param [in] Data   User entered command with parameters
 *  \param [in] Length Length of command
 *  \return void
 *  
 */
void ProcessATCommand(char *Data, uint8_t Length)
{
    char *  Parameters    = strchr(Data, ':');
    int CommandProcessed = 0;
    uint8_t ParametersPos = 0x00;
    if (Parameters == NULL)
    {
        dxprintf("Len: %d, no params\n", Length);
    }
    else
    {
        ParametersPos = Parameters - Data;
        dxprintf("Len: %d, Params: %d, %s\n", Length, ParametersPos, Parameters);
    }

    /**
    *  CMD: PING - ping myself
    */
    if (strncmp(Data, "PING_ME", 7) == 0)
    {
        CmdPING();
        CommandProcessed = 1;
    }

    /**
    *  CMD: FIND_NEW_DEVICE or FIND - find new device. Device should be able to accept 'discover' command.
    */
    if (strncmp(Data, "FIND_NEW_DEVICE", 15) == 0 ||
        strncmp(Data, "FIND", 4) == 0)
    {
        CmdFindNewDevice();
        CommandProcessed = 1;
    }

    /**
    *  CMD: CHECK_INT_STATUS - Check current interrupt status
    */
    if (strncmp(Data, "CHECK_INT_STATUS", 16) == 0)
    {
        int     Reg = 0x00;
        if (sscanf((char*)Parameters+1, "%2x", &Reg) > 0)
        {
            CmdCheckRegister((uint8_t)Reg);
        }
        else
        {
            QueueResponse("ER:CHECK_INT_STATUS:001\n", OUSART1);
        }
        
        CommandProcessed = 1;
    }
    
    /**
    *  CMD: READ_RX - Read from RX buf
    */
    if (strncmp(Data, "READ_RX", 7) == 0)
    {
        int     Length = 0x00;
        if (sscanf((char*)Parameters+1, "%2d", &Length) > 0)
        {
            CmdReadRX((uint8_t)Length);
        }
        else
        {
            QueueResponse("ER:READ_RX:001\n", OUSART1);
        }
        
        CommandProcessed = 1;
    }
    
    /**
    *  CMD: READ_RX - Read from RX buf
    */
    if (strncmp(Data, "DEVICE_STATE", 12) == 0)
    {
        uint8_t Buf[2] = {0};
        Transceiver_ReadRegs(Transceiver_REQUEST_DEVICE_STATE, NULL, 0, Buf, 2, 1);
        dxprintf("State: %02x %02x\n", Buf[0], Buf[1]);
        
        CommandProcessed = 1;
    }
    
    /**
    * CMD: SET_UPD_FW_TYPE - Set update firmware flag
    * param: Device Address XXXX - if set then firmware will send to given device, 
    *        if 0 then will be stored to local flash
    */
    if (strncmp(Data, "SET_UPD_FW_TYPE", 15) == 0)
    {
        if (Parameters == NULL) {
            rfSetFWUpdateType(0x00);
            QueueResponse("OK:SET_UPD_FW_TYPE:LOCAL\n", OUSART1);
        } else {
            int     DevAddress = 0x00;
            if (sscanf((char*)Parameters+1, "%4x", &DevAddress) > 0)
            {
                rfSetFWUpdateType(DevAddress);
                QueueResponse("OK:SET_UPD_FW_TYPE:DEVICE\n", OUSART1);
            } else {
                QueueResponse("ER:015\n", OUSART1);
            }
        }
        CommandProcessed = 1;
    }

    /**
    *  CMD: HANDLE_RTX_STATUS - Check and handle current transmitter status
    */
    if (strncmp(Data, "HANDLE_RTX_STATUS", 16) == 0)
    {
        CmdHandleTxRxStatus();
        CommandProcessed = 1;
    }

    /**
    *  CMD: ADD_DEVICE - Add new device into device list. This command doesn't send any data to device.
    */
    if (strncmp(Data, "ADD_DEVICE", 10) == 0)
    {
        rfCreateDevice();
        QueueResponse("OK:ADD_DEVICE\n", OUSART1);
        CommandProcessed = 1;
    }

    /**
    *  CMD: LIST_DEVICES - print loaded devices 
    */
    if (strncmp(Data, "LIST_DEVICES", 12) == 0)
    {
        rfListDevices();
        CommandProcessed = 1;
    }

    /**
    *  CMD: SAVE_DEVICES - save current devices 
    */
    if (strncmp(Data, "SAVE_DEVICES", 12) == 0)
    {
        rfSaveDevices();
        QueueResponse("OK:SAVE_DEVICES\n", OUSART1);
        CommandProcessed = 1;
    }

    /**
    *  CMD: LOAD_DEVICES - load saved devices 
    */
    if (strncmp(Data, "LOAD_DEVICES", 12) == 0)
    {
        rfLoadDevices();
        rfListDevices();
        QueueResponse("OK:LOAD_DEVICES\n", OUSART1);
        CommandProcessed = 1;
    }

    /**
    *  CMD: PREPARE_TEST_DEVICES - generate list of dummy devices
    */
    if (strncmp(Data, "PREPARE_TEST_DEVICES", 20) == 0)
    {
        rfPrepareTestDevices();
        QueueResponse("OK:PREPARE_TEST_DEVICES\n", OUSART1);
        CommandProcessed = 1;
    }

    /**
    *  CMD: PING_DEVICE - ping one device
    */
    if (strncmp(Data, "PING_DEVICE", 11) == 0)
    {
        int     DevAddress = 0x00;
        if (sscanf((char*)Parameters+1, "%4x", &DevAddress) > 0)
        {
            rfPingDevice(DevAddress);
        }
        QueueResponse("OK:PING_DEVICE\n", OUSART1);
        CommandProcessed = 1;
    }

    /**
    *  CMD: PING_ALL_DEVICES - ping all loaded devices
    */
    if (strncmp(Data, "PING_ALL_DEVICES", 16) == 0)
    {
        rfPingAllDevices();
        QueueResponse("OK:PING_ALL_DEVICES\n", OUSART1);
        CommandProcessed = 1;
    }

    /**
    *  CMD: SELF_TEST - run self tests - not implemented yet!
    */
    if (strncmp(Data, "TUNE_RF", 7) == 0)
    {
        int     CapBank = 0x00;
        if (sscanf((char*)Parameters+1, "%2x", &CapBank) > 0)
        {
            uint8_t Data[5] = {0x00, 0x02, 0x00, (uint8_t)CapBank, 0x00};
            Transceiver_WriteCommand(0x11, Data, 5);
        }
        QueueResponse("OK:TUNE_RF", OUSART1);
        CommandProcessed = 1;
    }

    /**
    *  CMD: PREPARE_FW_LFLASH - erase LOCAL (NOT DEVICE!) flash for new firmware
    */
    if (strncmp(Data, "PREPARE_FW_LFLASH", 17) == 0)
    {
        FWPrepareFlash();
        QueueResponse("OK:PREPARE_FW_LFLASH", OUSART1);
        CommandProcessed = 1;
    }
    
    /**
    *  CMD: APPLY_FW_LFLASH - erase LOCAL (NOT DEVICE!) flash for new firmware
    */
    if (strncmp(Data, "APPLY_FW_LFLASH", 15) == 0)
    {
        FWCheckFlash();
        QueueResponse("OK:APPLY_FW_LFLASH", OUSART1);
        CommandProcessed = 1;
    }
    
        
    /**
    *  CMD: ENABLE_NOISE - send noise to air to check carrier freq.
    */
    if (strncmp(Data, "ENABLE_NOISE", 12) == 0)
    {
        TIM2->CR1 ^= TIM_CR1_CEN;
        QueueResponse("OK:ENABLE_NOISE", OUSART1);
        CommandProcessed = 1;
    }
    
    /**
    *  CMD: PFW_DFLASH - erase DEVICE flash for new firmware.
    */
    if (strncmp(Data, "PFW_DFLASH", 10) == 0)
    {
        int     DevAddress = 0x00;
        if (sscanf((char*)Parameters+1, "%4x", &DevAddress) > 0)
        {
            xDevice * Device = rfGetDevice(DevAddress);
            if (Device == NULL)
            {
                QueueResponse("ER:PFW_DFLASH:002\n", OUSART1);  //Device with given address not linked
            }
            else if (Device->Type == 0x00)
            {
                QueueResponse("ER:PFW_DFLASH:003\n", OUSART1);  //Device type not valid
            }
            else
            {
                dxprintf("DA: %4x\n", Device->Address);
                rfSendData(rfCMD_FW_PREPARE, Device, 0);
                QueueResponse("OK:PFW_DFLASH\n", OUSART1);
            }
        }
        else
        {
            QueueResponse("ER:PFW_DFLASH:001\n", OUSART1);  //Device address not set!
        }
        CommandProcessed = 1;
    }

    /**
    *  CMD: SEND_DATA - send data to given device
    */
    if (strncmp(Data, "SEND_DATA", 9) == 0)
    {
        int     DevAddress                             = 0x00;
        uint8_t ArgumentsLength                        = Length - ParametersPos - 6;
        char    Arguments[ArgumentsLength];
        if (sscanf((char*)Parameters+1, "%4x:%s", &DevAddress, Arguments) > 0)
        {
            xDevice * Device = rfGetDevice(DevAddress);
            if (Device == NULL)
            {
                QueueResponse("ER:SEND_DATA:002\n", OUSART1);  //Device with given address not linked
            }
            else if (Device->Type == 0x00)
            {
                QueueResponse("ER:SEND_DATA:003\n", OUSART1);  //Device type not valid
            }
            else
            {
                dxprintf("DA: %4x, A: %s\n", Device->Address, Arguments);
                rfSendData(rfCMD_W_DATA, Device, Arguments);
                QueueResponse("OK:SEND_DATA\n", OUSART1);
            }
        }
        else
        {
            QueueResponse("ER:SEND_DATA:001\n", OUSART1);  //Device address not set!
        }
        CommandProcessed = 1;
    }

    /**
    *  CMD: REGISTER_DEVICE - register new device manually
    */
    if (strncmp(Data, "REGISTER_DEVICE", 15) == 0)
    {
        int     DevAddress                             = 0x00;
        int     DevType                                = 0x00;
        if (sscanf((char*)Parameters+1, "%4x:%2x", &DevAddress, &DevType) > 0)
        {
            xDevice * Device = rfGetDevice(DevAddress);
            if (Device == NULL)
            {
//                QueueResponse(">ER: Device with given address not linked!\n\n");
                xDevice * newDevice = rfRegisterDevice(DevAddress, DevType);
                QueueResponse("OK:REGISTER_DEVICE\n", OUSART1);
            }
            else
            {
                QueueResponse("ER:REGISTER_DEVICE:004\n", OUSART1);      //Device already registered!
            }
        }
        else
        {
            QueueResponse("ER:REGISTER_DEVICE:001\n", OUSART1);          //Device address not set
        }
        CommandProcessed = 1;
    }

    if (CommandProcessed == 0)
    {
        QueueResponse("ER:REGISTER_DEVICE:005\n", OUSART1);                 //Unknown command!
    }
}

/**
 *  \brief Routine for CMD HANDLE_TX_STATUS and HANDLE_RX_STATUS
 *  
 *  \param [in] void
 *  \return void
 */
void CmdHandleTxRxStatus(void)
{
    Transceiver_HandleStatus();
    uint8_t Status = 0;
    char    Buf[25];
    sprintf(Buf, "OK:HANDLE_RTX_STATUS:%02x\n", Status);
    QueueResponse(Buf, OUSART1);
}

/**
 *  \brief Routine for CMD HANDLE_TX_STATUS and HANDLE_RX_STATUS
 *  
 *  \param [in] uint8_t Length
 *  \return void
 */
void CmdReadRX(uint8_t Length)
{
    uint8_t    Buf[Length+1];
    Buf[Length] = 0x00;
    Transceiver_ReadRXBuf(Buf, Length);
    dxprintf("Buf: %02x %02x\n", Buf[0], Buf[1]);
//    QueueResponse((char *)Buf, OUSART1);
}

/**
 *  \brief Routine for CHECK_TX_STATUS and CHECK_RX_STATUS
 *  
 *  \param [in] Reg - register
 *  \return void
 */
void CmdCheckRegister(uint8_t Reg)
{
    uint8_t Status[8] = {0};
    Transceiver_ReadRegs(Transceiver_GET_INT_STATUS, NULL, 0, Status, 8, 1);
    char    Buf[50];
    sprintf(Buf, "OK:GET_INT_STATUS:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n", Status[0], Status[1], Status[2], Status[3], Status[4], Status[5], Status[6], Status[7]);
    QueueResponse(Buf, OUSART1);
    
    Transceiver_ReadRegs(Transceiver_FIFO_INFO, NULL, 0, Status, 2, 1);
    sprintf(Buf, "OK:FIFO_INFO:%02d,%02d\n", Status[0], Status[1]);
    QueueResponse(Buf, OUSART1);
    
    Transceiver_ReadRegs(Transceiver_GET_PH_STATUS, NULL, 0, Status, 2, 1);
    sprintf(Buf, "OK:PHS:%02x,%02x\n", Status[0], Status[1]);
    QueueResponse(Buf, OUSART1);
}

/**
 *  \brief Routine for PREPARE_FW_DFLASH
 *  
 *  \param [in] Send command to Device to prepare his flash for new firmware
 *  \return void
 */
void FWPrepareDeviceFlash(uint16_t Address)
{
    rfSendCommad(rfCMD_FW_PREPARE, Address, 0, 0, 0);
    QueueResponse("OK:PREPARE_FW_DFLASH\n", OUSART1);
}

/**
 *  \brief Routine for FIND_NEW_DEVICE
 *  
 *  \return void
 */
void CmdFindNewDevice(void)
{
    QueueResponse("OK:FIND_NEW_DEVICE:WAIT\n", OUSART1);

    // TODO: Generate new device address besed on table of current devices
    xDevice * newDevice = rfCreateDevice();
    if (newDevice == NULL)
        return;

    uint8_t Data[11] = {0x00,
                        Transceiver_HUB_ADDR[0],
                        Transceiver_HUB_ADDR[1],
                        rfCMD_DISCOVER,
                        (uint8_t)(newDevice->Address >> 8),
                        (uint8_t)newDevice->Address,
                        0x01};

    *((uint32_t *)(Data + 7)) = newDevice->Salt;

    Transceiver_TxData((uint16_t)Transceiver_DEVICE_DISCOVER_ADDR[0]<<8 | Transceiver_DEVICE_DISCOVER_ADDR[1], Data, 11);
}

/**
 *  \brief Routine for PING
 *  
 *  \return void
 */
void CmdPING(void) { QueueResponse("OK:PONG_ME\n", OUSART1); }
