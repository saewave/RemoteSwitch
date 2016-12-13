/**
  ******************************************************************************
  * @file    rf_cmd_exec.c
  * @author  SaeWave Application Team: alexsam
  * @version V1.0
  * @brief   SaeWave Command Execute Source File.
  *          This file contains the user functions to implement nessesary logic for each standart methods.
  *          User can change the methods related to program logic.
  *          
  *          Warning! Copy this file to your Device Source code folder. 
  *          DO NOT SHARE THIS FILE WITH OTHER DEVICES!
  *
*/

#include "rf_cmd_exec.h"
#include "rf_cmd.h"
#include "xdebug.h"

void rfCmdWriteData(uint8_t *Data, uint8_t Length)
{
    //******** Put your code here ********
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
}
