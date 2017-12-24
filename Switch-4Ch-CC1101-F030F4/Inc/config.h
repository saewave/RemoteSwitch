#ifndef __CONFIG_H
#define __CONFIG_H

#define _DEBUG

#define SAE_FLASH_CFG_ADDR 0x08004000       //last page of mem
#define SAE_FW_MEMORY_ADDRESS 0x08001C00            //mid of mem
#define SAE_READ_ADDR_ON_START 0x01
#define SAE_SETUP_TIMEOUT 30 * 1000 // Setup timeout in ms (30 sec)
#define SAE_DEVICE_TYPE 0x01

#define SAE_ALL_TIME_RX_MODE 0
#define SAE_ALL_TIME_SPWD_MODE 1
#define SAE_USE_STOP_MODE 1
#define SAE_DEBUG_IN_STOP_MODE 0

void readConfig(void);

#define SAE_MOVE_VECTOR_TABLE 0
#define SAE_MAIN_PROGRAM_START_ADDRESS (uint32_t)0x08000400
#define SAE_MAIN_PROGRAM_RAM_ADDRESS 0x20000000

#endif /* __CONFIG_H */
