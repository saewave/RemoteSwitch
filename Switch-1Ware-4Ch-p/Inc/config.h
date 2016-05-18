#ifndef __CONFIG_H
#define __CONFIG_H

#define configFLASH_CFG_ADDR        0x08003C00
#define configREAD_ADDR_ON_START    0x01
#define configSETUP_TIMEOUT         30 * 1000    // Setup timeout in ms (30 sec)
#define DEVICE_TYPE 0x01

void readConfig(void);

#endif /* __CONFIG_H */
