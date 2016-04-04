#ifndef __rf_device_H
#define __rf_device_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define rfCONFIG_MASK_CONFIRMED 0x01
#define rfCONFIG_MASK_ONLINE    0x02
#define rfCONFIG_MASK_ENABLED   0x04

#define rfCMD_PING              0x00
#define rfCMD_DISCOVER          0x01
#define rfCMD_W_DATA            0x02
#define rfCMD_R_DATA            0x03
#define rfCMD_W_CONFIG          0x04
#define rfCMD_R_CONFIG          0x05

#define rfDEVICE_TYPE_1         0x01
#define rfCMD_DATA_MASK_TYPE_1  "%3d,%3d,%3d,%3d,%3d"   //  State: 0-100 up to 4 times, last value is period 0-255 * 10 ms (if PWM supported)


typedef struct Device *dLink;

typedef struct Device
{
  uint16_t Address;   //only last 2 bytes of device address
  uint8_t Type;
  uint8_t Config;     //[0] - Confirmed, [1] - Online, [2] - Enabled
  uint32_t Salt;      //Random salt for CRC32 checksum
  dLink    Next;
  dLink    Prev;
} xDevice;

dLink rfCreateDevice(void);
void rfListDevices (void);
dLink rfUpdateDevice(uint16_t Address, uint8_t Type, uint8_t Config);
uint8_t rfRemoveDevices (uint16_t Address);
void rfSaveDevices(void);
void rfLoadDevices(void);
void rfPingDevice(uint16_t Address);
void rfPingAllDevices(void);
void rfSendData(uint8_t Cmd, dLink Device, char *Parameters);
void rfSendCommad(uint8_t Command, uint16_t Address, uint8_t *Data, uint8_t Length, uint32_t Salt);
dLink rfGetDevice(uint16_t Address);
void rfPrepareTestDevices(void);
uint32_t rfGetSalt(void);
uint32_t rfCalcCRC32(uint8_t *Data, uint8_t Length);



#endif /* __rf_device_H */
