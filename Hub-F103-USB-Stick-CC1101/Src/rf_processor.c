#include "F103RE_Peripheral.h"
#include "rf_processor.h"
#include "at_processor.h"
#include "rf_device.h"
#include "CC1101.h"
#include "core.h"
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
void rfProcessCommand(char *Data, uint8_t Length)
{

    xprintf("DATA: \n");
    for (uint8_t i = 0; i < Length; i++) {
        dxprintf("%x ", Data[i]);
    }
    xprintf("\n");

        
    uint8_t Header = Data[0]; // Header of command

    uint8_t Command = Data[CC1101_ADDR_LENGTH+1]; // Command we assume that command length = 1 byte
    dxprintf("H: %02x, C: %02x, L: %02x\n", Header, Command, Length);
    switch (Command)
    {
        case rfCMD_PING: //PING answer
        {
            if (!(Header & rfHEADER_MASK_RESPONSE))
                return;
            dLink updDevice = rfUpdateDevice((((uint16_t)Data[1] << 8) | Data[2]), NULL, rfCONFIG_MASK_ONLINE);
            char  Buf[60];
            if (updDevice == NULL)
            {
                sprintf(Buf, "WR:%02x%02x:%02x:NOT REGISTERED, BUT PINGED!\n", Data[4], Data[5], Data[7]);
                QueueResponse(Buf, OUSART1);
            }
            else
            {
                sprintf(Buf, "OK:PING:%02x%02x:%02x:ONLINE\n", Data[4], Data[5], Data[7]);
                QueueResponse(Buf, OUSART1);
            }
        }
        break;
        case rfCMD_DISCOVER: //DISCOVER answer
        {
            if (!(Header & rfHEADER_MASK_RESPONSE))
                return;
            //        dxprintf("Addr: %04x, Type: %02x\n", (((uint16_t)Data[4] << 8) | Data[5]), Data[7]);
            dLink updDevice = rfUpdateDevice((((uint16_t)Data[4] << 8) | Data[5]), Data[6], rfCONFIG_MASK_CONFIRMED | rfCONFIG_MASK_ONLINE | rfCONFIG_MASK_ENABLED);
            char  Buf[55];
            if (updDevice)
            {
                sprintf(Buf, "OK:DISCOVER:%02x%02x:%02x:CONNECTED\n", Data[4], Data[5], Data[6]);
                QueueResponse(Buf, OUSART1);
            }
            else
            {
                sprintf(Buf, "OK:DISCOVER:%02x%02x:%02x:NOT CONNECTED\n", Data[4], Data[5], Data[6]);
                QueueResponse(Buf, OUSART1);
            };
        }
        break;

        case rfCMD_R_DATA: //Read Data from device
        {
            if (!(Header & rfHEADER_MASK_RESPONSE))
                return;
            dLink xDevice = rfGetDevice((((uint16_t)Data[1] << 8) | Data[2]));
            char  Buf[CC1101_BUFFER_LENGTH+6];
            if (xDevice)
            {
                sprintf(Buf, "OK:DATA:%04x:%02x:", xDevice->Address, xDevice->Type);
                char tBuf[3];
                for (uint8_t i = 0; i < Length - CC1101_ADDR_LENGTH - 2; i++)
                {
                    snprintf(tBuf, 3, "%02x", Data[i + CC1101_ADDR_LENGTH + 2]);
                    strcat(Buf, tBuf);
                    //          if (i < Length-7-1) {
                    //            strcat(Buf, ",");
                    //          }
                }
                strcat(Buf, "\n");
                QueueResponse(Buf, OUSART1);
            }
            else
            {
                sprintf(Buf, "WR:%02x%02x:%02x:NOT REGISTERED!\n", Data[1], Data[2], Data[3]);
                QueueResponse(Buf, OUSART1);
            };
        }
        break;
        case rfCMD_FW_PREPARE: //Device Flash for Firmware prepared
        {
            if (!(Header & rfHEADER_MASK_RESPONSE))
                return;
            dLink xDevice = rfGetDevice((((uint16_t)Data[1] << 8) | Data[2]));
            char  Buf[23];
            if (xDevice)
            {
                sprintf(Buf, "OK:FW_PREPARE:%04x:%02x\n", xDevice->Address, xDevice->Type);
                QueueResponse(Buf, OUSART1);
            }
            else
            {
                sprintf(Buf, "WR:%02x%02x:%02x:NOT REGISTERED!\n", Data[1], Data[2], Data[3]);
                QueueResponse(Buf, OUSART1);
            };
        }
        break;
    }
}

void rfProcessError(char *Data, uint8_t Length)
{
}
