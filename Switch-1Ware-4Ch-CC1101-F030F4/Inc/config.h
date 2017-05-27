#ifndef __CONFIG_H
#define __CONFIG_H

#define _DEBUG

#define configFLASH_CFG_ADDR 0x08003C00
#define configREAD_ADDR_ON_START 0x00
#define configSETUP_TIMEOUT 30 * 1000 // Setup timeout in ms (30 sec)
#define configALL_TIME_RX_MODE 0x01
#define DEVICE_TYPE 0x01

void readConfig(void);

#define MOVE_VECTOR_TABLE 0
#define MAIN_PROGRAM_START_ADDRESS (uint32_t)0x08000400
#define MAIN_PROGRAM_RAM_ADDRESS 0x20000000
#define USE_STOP_MODE 0
#define DEBUG_IN_STOP_MODE 0

#endif /* __CONFIG_H */
