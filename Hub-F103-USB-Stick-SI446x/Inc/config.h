#ifndef __config_H
#define __config_H

#define SAE_CMD_BEGIN_CHAR1 0x41  // 'A'
#define SAE_CMD_BEGIN_CHAR2 0x54  // 'T'
#define SAE_CMD_BEGIN_CHAR3 0x2B  // '+'
#define SAE_CMD_BEGIN_HEXFW 0x3A  // ':'

#define SAE_CMD_LENGTH 50
#define SAE_CMD_QUEUE_LENGTH 5

#define SAE_DEVICE_LIST_LENGTH 10

#define SAE_ENABLE_USB 1

#define SAE_OUTPUT_DATA_LENGTH 100
#define SAE_OUTPUT_QUEUE_LENGTH 50

#define SAE_DEVICE_RX_QUEUE_LENGTH 5
#define SAE_DEVICE_TX_QUEUE_LENGTH 5

#define SAE_FW_UPDATE_LOCAL 0
#define SAE_FW_UPDATE_DEVICE 1

#define SAE_DEVICES_MEMORY_ADDRESS 0x08007C00       //last page of mem
#define SAE_START_MEMORY_ADDRESS 0x08000400         //start of programm address
#define SAE_FW_MEMORY_ADDRESS 0x08008000            //mid of mem
#define SAE_FW_SET_MEMORY_ADDRESS 0x0800FC00        //FW settings address (Length, CRC32, etc)
#define SAE_END_MEMORY_ADDRESS 0x08010000           //End mem
#define SAE_BOOT_MEMORY_ADDRESS 0x08000000          //bootloader start addr
#define SAE_BOOT_Size 0x400                         //bootloader size ( 1024B )
#define SAE_CPU_FREQUENCY 72000000

//#define SAE_ENABLE_DEBUG

#endif /* __config_H */
