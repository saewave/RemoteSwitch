#ifndef __rf_device_H
#define __rf_device_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define rfDEVICE_START_ADDRESS 0xA9D1

#define rfCONFIG_MASK_CONFIRMED 0x01
#define rfCONFIG_MASK_ONLINE    0x02
#define rfCONFIG_MASK_ENABLED   0x04

#define rfCMD_PING              0x00
#define rfCMD_DISCOVER          0x01
#define rfCMD_W_DATA            0x02
#define rfCMD_R_DATA            0x03
#define rfCMD_W_CONFIG          0x04
#define rfCMD_R_CONFIG          0x05
#define rfCMD_FW_PREPARE        0x06
#define rfCMD_FW_UPDATE         0x07

#define rfDEVICE_TYPE_1         0x01
#define rfDEVICE_TYPE_2         0x02
#define rfCMD_DATA_MASK_DEC  "%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d"
#define rfCMD_DATA_MASK_HEX  "%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x"

typedef struct Device
{
  uint16_t Address;   //only last 2 bytes of device address
  uint8_t Type;
  uint8_t Config;     //[0] - Confirmed, [1] - Online, [2] - Enabled
  uint32_t Salt;      //Random salt for CRC32 checksum
} xDevice;

xDevice * rfCreateDevice(void);
xDevice * rfRegisterDevice(uint16_t Address, uint8_t Type);
void rfListDevices (void);
xDevice * rfUpdateDevice(uint16_t Address, uint8_t Type, uint8_t Config);
uint8_t rfRemoveDevices (uint16_t Address);
void rfSaveDevices(void);
void rfLoadDevices(void);
void rfPingDevice(uint16_t Address);
void rfPingAllDevices(void);
void rfSendData(uint8_t Cmd, xDevice * Device, char *Parameters);
void rfSendCommad(uint8_t Command, uint16_t Address, uint8_t *Data, uint8_t Length, uint32_t Salt);
xDevice * rfGetDevice(uint16_t Address);
void rfPrepareTestDevices(void);
uint32_t rfGetSalt(void);
uint32_t rfCalcCRC32(uint8_t *Data, uint8_t Length);
void rfSetFWUpdateType(int Address);
void rfSendFWHex(uint16_t Address, uint8_t *Data, uint8_t Length);
    
#endif /* __rf_device_H */
