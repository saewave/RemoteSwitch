#include "at_processor.h"
#include "FreeRTOS.h"
#include "nRF24L01P.h"
#include "queue.h"
#include "rf_device.h"
#include "xdebug.h"
#include <stdio.h>
#include <stdlib.h>

extern QueueHandle_t U2TxQueueHandle;

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
        dxprintf("Len: %d, Params: %d\n", Length, ParametersPos);
    }

    /**
    *  CMD: PING - ping myself
    */
    if (strncmp(Data, (char *)"PING", 4) == 0)
    {
        CmdPING();
        CommandProcessed = 1;
    }

    /**
    *  CMD: FIND_NEW_DEVICE or FIND - find new device. Device should be able to accept 'discover' command.
    */
    if (strncmp(Data, (char *)"FIND_NEW_DEVICE", 15) == 0 ||
        strncmp(Data, (char *)"FIND", 4) == 0)
    {
        CmdFindNewDevice();
        CommandProcessed = 1;
    }

    /**
    *  CMD: CHECK_TX_STATUS - Check current transmitter status
    */
    if (strncmp(Data, (char *)"CHECK_TX_STATUS", 15) == 0)
    {
        QueueResponse((char *)"<CHECK_TX_STATUS\n", OUSART2);
        CmdCheckTxRxStatus(CHIP_Tx);
        CommandProcessed = 1;
    }

    /**
    *  CMD: CHECK_RX_STATUS - Check current receiver status
    */
    if (strncmp(Data, (char *)"CHECK_RX_STATUS", 15) == 0)
    {
        QueueResponse((char *)"<CHECK_RX_STATUS\n", OUSART2);
        CmdCheckTxRxStatus(CHIP_Rx);
        CommandProcessed = 1;
    }

    /**
    *  CMD: HANDLE_TX_STATUS - Check and handle current transmitter status
    */
    if (strncmp(Data, (char *)"HANDLE_TX_STATUS", 16) == 0)
    {
        QueueResponse((char *)"<HANDLE_TX_STATUS\n", OUSART2);
        CmdHandleTxRxStatus(CHIP_Tx);
        CommandProcessed = 1;
    }

    /**
    *  CMD: HANDLE_RX_STATUS - Check and handle current receiver status
    */
    if (strncmp(Data, (char *)"HANDLE_RX_STATUS", 16) == 0)
    {
        QueueResponse((char *)"<HANDLE_RX_STATUS\n", OUSART2);
        CmdHandleTxRxStatus(CHIP_Rx);
        CommandProcessed = 1;
    }

    /**
    *  CMD: ADD_DEVICE - Add new device into device list. This command doesn't send any data to device.
    */
    if (strncmp(Data, (char *)"ADD_DEVICE", 10) == 0)
    {
        QueueResponse((char *)"<ADD_DEVICE\n", OUSART2);
        rfCreateDevice();
        CommandProcessed = 1;
    }

    /**
    *  CMD: LIST_DEVICES - print loaded devices 
    */
    if (strncmp(Data, (char *)"LIST_DEVICES", 12) == 0)
    {
        QueueResponse((char *)"<LIST_DEVICES\n", OUSART2);
        rfListDevices();
        CommandProcessed = 1;
    }

    /**
    *  CMD: SAVE_DEVICES - save current devices 
    */
    if (strncmp(Data, (char *)"SAVE_DEVICES", 12) == 0)
    {
        QueueResponse((char *)"<SAVE_DEVICES\n", OUSART2);
        rfSaveDevices();
        QueueResponse((char *)"<DONE\n", OUSART2);
        CommandProcessed = 1;
    }

    /**
    *  CMD: LOAD_DEVICES - load saved devices 
    */
    if (strncmp(Data, (char *)"LOAD_DEVICES", 12) == 0)
    {
        QueueResponse((char *)"<LOAD_DEVICES\n", OUSART2);
        rfLoadDevices();
        rfListDevices();
        QueueResponse((char *)"<DONE\n", OUSART2);
        CommandProcessed = 1;
    }

    /**
    *  CMD: PREPARE_TEST_DEVICES - generate list of dummy devices
    */
    if (strncmp(Data, (char *)"PREPARE_TEST_DEVICES", 20) == 0)
    {
        QueueResponse((char *)"<PREPARE_TEST_DEVICES\n", OUSART2);
        rfPrepareTestDevices();
        QueueResponse((char *)"<DONE\n", OUSART2);
        CommandProcessed = 1;
    }

    /**
    *  CMD: PING_DEVICE - ping one device
    */
    if (strncmp(Data, (char *)"PING_DEVICE", 11) == 0)
    {
        QueueResponse((char *)"***PING_DEVICE NOT IMPLEMENTED!!!***", OUSART2);
        CommandProcessed = 1;
    }

    /**
    *  CMD: PING_ALL_DEVICES - ping all loaded devices
    */
    if (strncmp(Data, (char *)"PING_ALL_DEVICES", 16) == 0)
    {
        QueueResponse((char *)"<PING_ALL_DEVICES\n", OUSART2);
        rfPingAllDevices();
        QueueResponse((char *)"<DONE\n", OUSART2);
        CommandProcessed = 1;
    }

    /**
    *  CMD: SELF_TEST - run self tests
    */
    if (strncmp(Data, (char *)"SELF_TEST", 9) == 0)
    {
        QueueResponse((char *)"***SELF_TEST NOT IMPLEMENTED!!!***", OUSART2);
        CommandProcessed = 1;
    }

    /**
    *  CMD: SEND_DATA - send data to given device
    */
    if (strncmp(Data, (char *)"SEND_DATA", 9) == 0)
    {
        QueueResponse((char *)"<SEND_DATA\n", OUSART2);
        char    Parameters[Length - ParametersPos + 1];
        int     DevAddress                             = 0x00;
        uint8_t ArgumentsLength                        = Length - ParametersPos - 6;
        char    Arguments[ArgumentsLength];
        strlcpy(Parameters, &Data[ParametersPos + 1], Length - ParametersPos);

        if (sscanf(Parameters, "%4x:%s", &DevAddress, Arguments) > 0)
        {
            dLink Device = rfGetDevice(DevAddress);
            if (Device == NULL)
            {
                QueueResponse((char *)">ERROR: Device with given address not linked!\n\n", OUSART2);
            }
            else if (Device->Type == 0x00)
            {
                QueueResponse((char *)">ERROR: Device type not valid!\n\n", OUSART2);
            }
            else
            {
                rfSendData(rfCMD_W_DATA, Device, Arguments);
            }
        }
        else
        {
            QueueResponse((char *)">ERROR: Device address not set!\n\n", OUSART2);
        }
        CommandProcessed = 1;
    }

    /**
    *  CMD: REGISTER_DEVICE - register new device manually
    */
    if (strncmp(&Data[3], (char *)"REGISTER_DEVICE", 15) == 0)
    {
        QueueResponse((char *)"<REGISTER_DEVICE\n", OUSART2);
        char    Parameters[Length - ParametersPos + 1];
        int     DevAddress                             = 0x00;
        int     DevType                                = 0x00;
//        uint8_t ArgumentsLength                        = Length - ParametersPos - 6;
//        char    Arguments[ArgumentsLength];
        strlcpy(Parameters, &Data[ParametersPos + 1], Length - ParametersPos);

        if (sscanf(Parameters, "%4x:%2x", &DevAddress, &DevType) > 0)
        {
            dLink Device = rfGetDevice(DevAddress);
            if (Device == NULL)
            {
//                QueueResponse((char *)">ERROR: Device with given address not linked!\n\n");
                dLink newDevice = rfRegisterDevice(DevAddress, DevType);
            }
            else
            {
                QueueResponse((char *)">ERROR: Device already registered!\n\n", OUSART2);
            }
        }
        else
        {
            QueueResponse((char *)">ERROR: Device address not set!\n\n", OUSART2);
        }
        CommandProcessed = 1;
    }

    if (CommandProcessed == 0)
    {
        QueueResponse((char *)">ERROR: Unknown command!\n\n", OUSART2);
    }
}

/**
 *  \brief Routine for CMD HANDLE_TX_STATUS and HANDLE_RX_STATUS
 *  
 *  \param [in] Chip Selected chip, can be CHIP_Tx or CHIP_Rx
 *  \return void
 */
void CmdHandleTxRxStatus(uint8_t Chip)
{
    nRF24_HandleStatus(SPI2, Chip);
    uint8_t Status = nRF24_GetStatus(SPI2, Chip);
    char    Buf[17];
    sprintf(Buf, "Done, status: %x\n", Status);
    QueueResponse(Buf, OUSART2);
}

/**
 *  \brief Routine for CHECK_TX_STATUS and CHECK_RX_STATUS
 *  
 *  \param [in] Chip Selected chip, can be CHIP_Tx or CHIP_Rx
 *  \return void
 */
void CmdCheckTxRxStatus(uint8_t Chip)
{
    uint8_t Status = nRF24_GetStatus(SPI2, Chip);
    char    Buf[10];
    sprintf(Buf, "Status: %x\n", Status);
    QueueResponse(Buf, OUSART2);
}

/**
 *  \brief Routine for FIND_NEW_DEVICE
 *  
 *  \return void
 */
void CmdFindNewDevice(void)
{
    QueueResponse((char *)"FIND-NEW-DEVICE\nPlease wait...\n", OUSART2);

    // TODO: Generate new device address besed on table of current devices
    dLink newDevice = rfCreateDevice();

    uint8_t Data[17] = {0x00,
                        nRF24_HUB_addr[0],
                        nRF24_HUB_addr[1],
                        nRF24_HUB_addr[2],
                        nRF24_HUB_addr[3],
                        nRF24_HUB_addr[4],
                        rfCMD_DISCOVER,
                        nRF24_NET_TX_START_addr[0],
                        nRF24_NET_TX_START_addr[1],
                        nRF24_NET_TX_START_addr[2],
                        (uint8_t)(newDevice->Address >> 8),
                        (uint8_t)newDevice->Address,
                        0x01};

    *((uint32_t *)(Data + 13)) = newDevice->Salt;

    nRF24_TXPacket(SPI2, nRF24_DEVICE_CFG_addr, Data, 17);
}

/**
 *  \brief Routine for PING
 *  
 *  \return void
 */
void CmdPING(void) { QueueResponse((char *)"PONG!\n", OUSART2); }
