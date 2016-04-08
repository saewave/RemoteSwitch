#include "config.h"
#include "rf_cmd.h"
#include "rf_cmd_exec.h"
#include "nRF24L01P.h"
#include <stdio.h>
#include <string.h>

void ProcessData(uint8_t *Data, uint8_t Length) {
  uint8_t HUBAddress[5] = {0x00};
  uint8_t Header  = Data[0];
  uint8_t Pos = 1;
  if (!(Header & PD_H_MASK_HUB_ID_LEN)) {     //  If HUB address length = 5Bytes
    memcpy(HUBAddress, &Data[Pos], 5);
    Pos +=5;
  } else {                                    //  Other HUB address length not implemented!
    //  Not implemented
  }
  
  uint8_t Command;
  
  if (!(Header & PD_H_MASK_COMMAND_LEN)) {     //  If command length = 1Byte
    Command = Data[Pos];
    Pos++;
  } else {                                    //  Other command length not implemented!
    //  Not implemented
  }

  switch (Command) {
    case 0x00: {
        uint8_t *DeviceAddress = nRF24_GetDeviceAddress();
        uint8_t tBuf[9] = {0x00};
        tBuf[0]   = 0x10;
        memcpy(&tBuf[1], DeviceAddress, 5);
        tBuf[6]   = 0x00;       //Command
        tBuf[7]   = Data[7];
        tBuf[8]   = Data[8];
        nRF24_SwitchTo = nRF24_SWITCH_TO_RX;
        nRF24_TXPacket(&HUBAddress[0], &tBuf[0], 9);
    } break;
    case 0x01: {
        nRF24_SetHUBAddress(&HUBAddress[0]);
//        uint8_t NewAddress[5] = {0x00};
//        memcpy(NewAddress, &Data[Pos], 5);

        nRF24_SetDeviceAddress(&Data[Pos], Data[Pos+5]);
        Pos +=5;
        
        uint8_t *NewAddressAfter = nRF24_GetDeviceAddress();

        nRF24_TXMode();
        
        uint8_t tBuf[14] = {0x00};
        tBuf[0] = 0x10; // Response header, set bit [5]
        memcpy(&tBuf[1], NewAddressAfter, 5);
        tBuf[6] = 0x01; // Cmd 0x01
        tBuf[7] = DEVICE_TYPE; // Type 0x01 - simple switch
        memcpy(&tBuf[8], NewAddressAfter, 5);
        tBuf[13] = 0x01; // Set new address
        nRF24_SwitchTo = nRF24_SWITCH_TO_RX;
        nRF24_TXPacket(&HUBAddress[0], &tBuf[0], 14);
        
        SaveAddress(NewAddressAfter, &HUBAddress[0]);
      } break;
    case 0x02: {
      rfCmdExec(&Data[Pos], Length - Pos);
    } break;
  }

}

void SaveAddress(uint8_t *Address, uint8_t *HubAddress) {
  
  uint8_t Data[10] = {0x00};
  memcpy(&Data[0], Address, 5);
  memcpy(&Data[5], HubAddress, 5);
  
  FLASH->KEYR = FLASH_KEY1;
  FLASH->KEYR = FLASH_KEY2;
  while(FLASH->SR & FLASH_SR_BSY) {};    //Wait untill memory ready for erase
  FLASH->CR |= FLASH_CR_PER;              //Erase one page
  FLASH->AR |= configFLASH_CFG_ADDR;                //Erase last page
  FLASH->CR |= FLASH_CR_STRT;

  while(FLASH->SR & FLASH_SR_BSY) {};    //Wait untill memory ready
  FLASH->CR &= ~FLASH_CR_PER;
  while(FLASH->SR & FLASH_SR_BSY) {};
  FLASH->CR |= FLASH_CR_PG;                 //Allow flash programming
  while(FLASH->SR & FLASH_SR_BSY) {};
  uint32_t fAddress = configFLASH_CFG_ADDR;
  for(int i=0; i<10; i+=2) {
    *(__IO uint16_t*)fAddress = ((uint16_t)Data[i] << 8) | (uint16_t)Data[i+1];
    fAddress +=2;
  }
  while(FLASH->SR & FLASH_SR_BSY) {};
  FLASH->CR &= ~(FLASH_CR_PG);
}
