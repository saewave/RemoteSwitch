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

#include "rf_cmd_exec.h"
#include "core.h"
#include "rf_cmd.h"
#include "xdebug.h"

void rfCmdWriteData(uint8_t *Data, uint8_t Length)
{
    //******** Put your code here ********

    tUpdateChannelSettings *_tmp = (tUpdateChannelSettings *)Data;
    SetParamsToChannel(_tmp);
    SendCommandToHub(rfCMD_R_DATA, Data, Length);
}

void rfCmdReadData(uint8_t *Data, uint8_t Length, uint8_t *ResponseData, uint8_t *ResponseLength)
{
    //Response and ResponseLength should be filled to delivery to HUB
    //Return ResponseLength = 0 not allowed. Max ResponseLength = 24
    //******** Put your code here ********
}

void rfCmdWriteConfig(uint8_t *Data, uint8_t Length)
{
    //******** Put your code here ********
    tUpdateChannelSettings *_tmp = (tUpdateChannelSettings *)&Data[1];
    SetParamsToChannel(_tmp);
}

void rfCmdReadConfig(uint8_t *Data, uint8_t Length, uint8_t *ResponseData, uint8_t *ResponseLength)
{
    //Response and ResponseLength should be filled to delivery to HUB
    //Return ResponseLength = 0 not allowed. Max ResponseLength = 24
    //******** Put your code here ********
}

void rfInternalCallback(uint8_t *Data, uint8_t Size)
{
    //Internal callback. Used for periodical calling from timers and etc.
    //******** Put your code here ********
    SendCommandToHub(rfCMD_R_DATA, Data, Size);
}

void rfStartup(void)
{
    //This method called rigth after initialization.
    //******** Put your code here ********

    uint8_t Param = 0x01;
    SendCommandToHub(rfCMD_PING, &Param, 1);
    SwitchToState(STATE_GND);
}
