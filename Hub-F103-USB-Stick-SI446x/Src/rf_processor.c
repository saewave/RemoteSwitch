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
 
#include "F103RE_Peripheral.h"
#include "rf_processor.h"
#include "at_processor.h"
#include "rf_device.h"
#include "SI446x.h"
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

    uint8_t Command = Data[Transceiver_ADDR_LENGTH+1]; // Command we assume that command length = 1 byte
    dxprintf("H: %02x, C: %02x, L: %02x\n", Header, Command, Length);
    switch (Command)
    {
        case rfCMD_PING: //PING answer
        {
            if (!(Header & rfHEADER_MASK_RESPONSE))
                return;
            xDevice * updDevice = rfUpdateDevice((((uint16_t)Data[1] << 8) | Data[2]), NULL, rfCONFIG_MASK_ONLINE);
            char  Buf[60];
            if (updDevice == NULL)
            {
                sprintf(Buf, "WR:%02x%02x:%02x:NOT REGISTERED, BUT PINGED!\n", Data[1], Data[2], 0);
                QueueResponse(Buf, OUSART1);
            }
            else
            {
                sprintf(Buf, "OK:PING:%02x%02x:%02x:ONLINE\n", Data[1], Data[2], Data[4]);
                QueueResponse(Buf, OUSART1);
            }
        }
        break;
        case rfCMD_DISCOVER: //DISCOVER answer
        {
            if (!(Header & rfHEADER_MASK_RESPONSE))
                return;
            //        dxprintf("Addr: %04x, Type: %02x\n", (((uint16_t)Data[4] << 8) | Data[5]), Data[7]);
            xDevice * updDevice = rfUpdateDevice((((uint16_t)Data[4] << 8) | Data[5]), Data[6], rfCONFIG_MASK_CONFIRMED | rfCONFIG_MASK_ONLINE | rfCONFIG_MASK_ENABLED);
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
            xDevice * xDevice = rfGetDevice((((uint16_t)Data[1] << 8) | Data[2]));
            char  Buf[Transceiver_BUFFER_LENGTH+6];
            if (xDevice)
            {
                sprintf(Buf, "OK:DATA:%04x:%02x:", xDevice->Address, xDevice->Type);
                char tBuf[3];
                for (uint8_t i = 0; i < Length - Transceiver_ADDR_LENGTH - 2; i++)
                {
                    snprintf(tBuf, 3, "%02x", Data[i + Transceiver_ADDR_LENGTH + 2]);
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
            xDevice * xDevice = rfGetDevice((((uint16_t)Data[1] << 8) | Data[2]));
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
