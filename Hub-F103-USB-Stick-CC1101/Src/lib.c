#include "lib.h"
#include "core.h"

uint32_t FWInitialAddress = SAE_FW_MEMORY_ADDRESS;

unsigned int htoi (uint8_t *ptr, uint8_t Length)
{
    unsigned int value = 0;
    char ch = *ptr;
    uint8_t i=0;
    for (i=0; i < Length; i++) {
        if (ch >= '0' && ch <= '9')
            value = (value << 4) + (ch - '0');
        else if (ch >= 'A' && ch <= 'F')
            value = (value << 4) + (ch - 'A' + 10);
        else if (ch >= 'a' && ch <= 'f')
            value = (value << 4) + (ch - 'a' + 10);
        else
            return value;
        ch = *(++ptr);
    }
    return value;
}

void FWWriteToFlash(uint8_t *cData) {
    uint32_t Len = htoi(cData, 2);
    uint32_t Addr = htoi(&cData[2], 4);
    uint32_t Type = htoi(&cData[6], 2);
    uint16_t HWord = 0;
    
    if (Len == 0x02 && Addr == 0 && Type == 0x04) {      //Set Extended Linear Address Record
        FWInitialAddress = SAE_FW_MEMORY_ADDRESS | ((uint32_t)htoi(&cData[8], 4)<<16);
        
        if (FWInitialAddress >= SAE_DEVICES_MEMORY_ADDRESS) {
            FWInitialAddress = SAE_FW_MEMORY_ADDRESS;
            QueueResponse("ER:011\n", OUSART1);    //Given address is more than SAE_DEVICES_MEMORY_ADDRESS
            return;
        }
    }
    
    if (Len > 0 && Type == 0) {              //Write data to flash
        if (Len > 0x10) {
            QueueResponse("ER:012\n", OUSART1);     //Length can't be more than 16 bytes
            return;
        }
        
        if ((FWInitialAddress | Addr) >= SAE_DEVICES_MEMORY_ADDRESS) {
            QueueResponse("ER:011\n", OUSART1);    //Given address is hit to SAE_DEVICES_MEMORY_ADDRESS
            return;
        }

        FLASH->KEYR = FLASH_FKEY1;      // Unlock flash
        FLASH->KEYR = FLASH_FKEY2;
        FLASH->CR |= FLASH_CR_PG;       // Allow write
        while(FLASH->SR & FLASH_SR_BSY) {__NOP();};
        for (uint8_t i = 0; i < Len; i=i+2) {
            HWord = htoi(&cData[8+(i*2)], 2) | (htoi(&cData[8+(i*2)+2], 2) << 8);
            *(__IO uint16_t*)((FWInitialAddress | Addr) + i)  = HWord;
            while(FLASH->SR & FLASH_SR_BSY) {__NOP();};
        }

        FLASH->CR |= FLASH_CR_LOCK;
    }
    
    char    Buf[16];
    sprintf(Buf, "OK:%2x,%4x,%2x\n", Len, Addr, Type);
    
    QueueResponse(Buf, OUSART1);
}

void FWPrepareFlash(void) {
    FLASH_Erase(SAE_FW_MEMORY_ADDRESS, ((SAE_DEVICES_MEMORY_ADDRESS - SAE_FW_MEMORY_ADDRESS) / 1024) - 1);
}
