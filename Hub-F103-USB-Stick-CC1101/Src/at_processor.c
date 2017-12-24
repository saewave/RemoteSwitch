#include "at_processor.h"
#include "CC1101.h"
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
    *  CMD: CHECK_REGISTER - Check current transmitter status
    */
    if (strncmp(Data, "CHECK_RTX_REGISTER", 14) == 0)
    {
        int     Reg = 0x00;
        if (sscanf((char*)Parameters+1, "%2x", &Reg) > 0)
        {
            CmdCheckRegister((uint8_t)Reg);
        }
        else
        {
            QueueResponse("ER:CHECK_RTX_REGISTER:001\n", OUSART1);
        }
        
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
    if (strncmp(Data, "SELF_TEST", 9) == 0)
    {
        QueueResponse("ER:SELF_TEST NOT IMPLEMENTED!", OUSART1);
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
    *  CMD: PFW_DFLASH - erase DEVICE flash for new firmware.
    */
    if (strncmp(Data, "PFW_DFLASH", 10) == 0)
    {
        int     DevAddress = 0x00;
        if (sscanf((char*)Parameters+1, "%4x", &DevAddress) > 0)
        {
            dLink Device = rfGetDevice(DevAddress);
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
            dLink Device = rfGetDevice(DevAddress);
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
            dLink Device = rfGetDevice(DevAddress);
            if (Device == NULL)
            {
//                QueueResponse(">ER: Device with given address not linked!\n\n");
                dLink newDevice = rfRegisterDevice(DevAddress, DevType);
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
    CC1101_HandleStatus();
    uint8_t Status = 0;
    char    Buf[25];
    sprintf(Buf, "OK:HANDLE_RTX_STATUS:%02x\n", Status);
    QueueResponse(Buf, OUSART1);
}

/**
 *  \brief Routine for CHECK_TX_STATUS and CHECK_RX_STATUS
 *  
 *  \param [in] Reg - register
 *  \return void
 */
void CmdCheckRegister(uint8_t Reg)
{
    uint8_t Status = CC1101_ReadStatusReg(Reg);
    char    Buf[23];
    sprintf(Buf, "OK:CHECK_RTX_REGISTER:%02x\n", Status);
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
    dLink newDevice = rfCreateDevice();

    uint8_t Data[11] = {0x00,
                        CC1101_HUB_ADDR[0],
                        CC1101_HUB_ADDR[1],
                        rfCMD_DISCOVER,
                        (uint8_t)(newDevice->Address >> 8),
                        (uint8_t)newDevice->Address,
                        0x01};

    *((uint32_t *)(Data + 7)) = newDevice->Salt;

    CC1101_TxData((uint16_t)CC1101_DEVICE_DISCOVER_ADDR[0]<<8 | CC1101_DEVICE_DISCOVER_ADDR[1], Data, 11);
}

/**
 *  \brief Routine for PING
 *  
 *  \return void
 */
void CmdPING(void) { QueueResponse("OK:PONG_ME\n", OUSART1); }
