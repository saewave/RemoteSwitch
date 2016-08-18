#include "rf_processor.h"
#include "FreeRTOS.h"
#include "at_processor.h"
#include "rf_device.h"
#include "xdebug.h"

#define rfHEADER_MASK_RESPONSE 0x10

/**
 *  \brief Process command from remote device
 *  
 *  \param [in] Data   Data from device
 *  \param [in] Length Length of data
 *  \return void
 *  
 *  \details This function take and parse the data received from remote device and call necessary routines to process this data.
 */
void rfProcessCommand(uint8_t *Data, uint8_t Length)
{
    uint8_t Header = Data[0]; // Header of command

    uint8_t Command = Data[6]; // Command we assume that command length = 1 byte
    dxprintf("rfCMD: %02x, L: %d\n", Command, Length);
    switch (Command)
    {
    case rfCMD_PING: //PING answer
    {
        if (!(Header & rfHEADER_MASK_RESPONSE))
            return;
        dLink updDevice = rfUpdateDevice((((uint16_t)Data[4] << 8) | Data[5]), NULL, rfCONFIG_MASK_ONLINE);
        char  Buf[60];
        if (updDevice == NULL)
        {
            sprintf(Buf, ">WARN:%02x%02x:%02x:NOT registered, but pinged!\n", Data[4], Data[5], Data[7]);
            QueueResponse(Buf);
        }
        else
        {
            sprintf(Buf, ">PING:%02x%02x:%02x:ONLINE\n", Data[4], Data[5], Data[7]);
            QueueResponse(Buf);
        }
    }
    break;
    case rfCMD_DISCOVER: //DISCOVER answer
    {
        if (!(Header & rfHEADER_MASK_RESPONSE))
            return;
        //        dxprintf("Addr: %04x, Type: %02x\n", (((uint16_t)Data[4] << 8) | Data[5]), Data[7]);
        dLink updDevice = rfUpdateDevice((((uint16_t)Data[4] << 8) | Data[5]), Data[7], rfCONFIG_MASK_CONFIRMED | rfCONFIG_MASK_ONLINE | rfCONFIG_MASK_ENABLED);
        char  Buf[55];
        if (updDevice)
        {
            sprintf(Buf, ">DISCOVER:%02x%02x:%02x:CONNECTED\n", Data[4], Data[5], Data[7]);
            QueueResponse(Buf);
        }
        else
        {
            sprintf(Buf, ">DISCOVER:%02x%02x:%02x:NOT CONNECTED\n", Data[4], Data[5], Data[7]);
            QueueResponse(Buf);
        };
    }
    break;

    case rfCMD_R_DATA: //Read Data
    {
        if (!(Header & rfHEADER_MASK_RESPONSE))
            return;
        dLink xDevice = rfGetDevice((((uint16_t)Data[4] << 8) | Data[5]));
        char  Buf[55];
        if (xDevice)
        {
            sprintf(Buf, ">DATA:%04x:%02x:", xDevice->Address, xDevice->Type);
            char tBuf[5];
            for (uint8_t i = 0; i < Length - 7; i++)
            {
                snprintf(tBuf, sizeof tBuf, "%02x", Data[i + 7]);
                strcat(Buf, tBuf);
                //          if (i < Length-7-1) {
                //            strcat(Buf, ",");
                //          }
            }
            strcat(Buf, "\n");
            QueueResponse(Buf);
        }
        else
        {
            sprintf(Buf, ">WARN:%02x%02x:%02x:NOT registered!\n", Data[4], Data[5], Data[7]);
            QueueResponse(Buf);
        };
    }
    break;
    }
}

void rfProcessError(uint8_t *Data, uint8_t Length)
{
}
