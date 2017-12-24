#ifndef __config_H
#define __config_H

#define SAE_CMD_BEGIN_CHAR1 0x41  // 'A'
#define SAE_CMD_BEGIN_CHAR2 0x54  // 'T'
#define SAE_CMD_BEGIN_CHAR3 0x2B  // '+'
#define SAE_CMD_BEGIN_HEXFW 0x3A  // ':'

#define SAE_CMD_LENGTH 50
#define SAE_CMD_QUEUE_LENGTH 5

#define SAE_OUTPUT_DATA_LENGTH 100
#define SAE_OUTPUT_QUEUE_LENGTH 50

#define SAE_DEVICE_DATA_QUEUE_LENGTH 5

#define SAE_FW_UPDATE_LOCAL 0
#define SAE_FW_UPDATE_DEVICE 1

#define SAE_DEVICES_MEMORY_ADDRESS 0x0800FC00       //last page of mem
#define SAE_FW_MEMORY_ADDRESS 0x08008000            //mid of mem
#define SAE_CPU_FREQUENCY 72000000

#define SAE_ENABLE_DEBUG

#endif /* __config_H */
