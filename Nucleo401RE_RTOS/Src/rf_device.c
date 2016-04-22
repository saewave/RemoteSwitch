#include "stm32f4xx_hal.h"
#include "rf_device.h"
#include "at_processor.h"
#include "FreeRTOS.h"
#include "task.h"
#include "nRF24L01P.h"
#include "spi.h"
#include "cmsis_os.h"

dLink rfDevices = NULL;

uint16_t rfDeviceAddrCounter = 0x0001;

dLink rfCreateDevice(void) {
  printf("rfCreateDevice\n");
  dLink newDevice = ( dLink ) pvPortMalloc ( sizeof (xDevice));
  if (newDevice == NULL) {
    printf("Error: can't allocate memory!\n");
    return NULL;
  }
  printf("New device on: %p!\n", newDevice);
  newDevice->Next = NULL;
  newDevice->Prev = NULL;
  newDevice->Type = 0x00;
  newDevice->Config = 0x00;
  newDevice->Salt = rfGetSalt();

  if (rfDevices == NULL) {    // If list is empty
    rfDevices = newDevice;
    rfDevices->Address = rfDeviceAddrCounter;
  } else {                    // If list is not empty, find last elem, and add to it.
    dLink Cur = rfDevices;
    rfDeviceAddrCounter = Cur->Address;   // TODO: Change the code below to get find Address more accurate
    while (Cur->Next != NULL) {
      Cur = Cur->Next;
      if (rfDeviceAddrCounter < Cur->Address) {
        rfDeviceAddrCounter = Cur->Address;
      }
    };
    newDevice->Address = ++rfDeviceAddrCounter;
    newDevice->Prev = Cur;
    Cur->Next = newDevice;
  }
  printf("rf: %p, ND: %p\n", rfDevices, newDevice);
  return newDevice;
}

void rfListDevices (void) {
  dLink Cur = rfDevices;
  uint8_t cnt = 1;
  char Buf[100];
  while (Cur != NULL) {
    sprintf(Buf, "%d, Device: Adr: %04x Type: %02x, Conf: %x, mA: %p, mP: %p, mN: %p\n", cnt, Cur->Address, Cur->Type, Cur->Config, Cur, Cur->Prev, Cur->Next);
    //sprintf(Buf, "%d, Device: Adr: 0x%x Type: 0x%x\n", cnt, Cur->Address, Cur->Type);
    QueueResponse(&Buf[0]);
    Cur = Cur->Next;
    cnt++;
  };
}

dLink rfUpdateDevice(uint16_t Address, uint8_t Type, uint8_t Config) {
  dLink Cur = rfDevices;
  uint8_t isUpdated = 0;
  while (Cur != NULL) {
//    printf("fd A: %x, P: %p, N: %p\n", Cur->Address, Cur->Prev, Cur->Next);
    if (Cur->Address == Address) {
      if (Type != NULL) {
        Cur->Type = Type;
      }
      Cur->Config = Config;
      isUpdated = 1;
      break;
    }
    Cur = Cur->Next;
  }
  if (isUpdated)
    return Cur;
  else
    return NULL;
}

dLink rfGetDevice(uint16_t Address) {
  dLink Cur = rfDevices;
  while (Cur != NULL) {
//    printf("fd A: %x, P: %p, N: %p\n", Cur->Address, Cur->Prev, Cur->Next);
    if (Cur->Address == Address) {
      return Cur;
    }
    Cur = Cur->Next;
  }
  return NULL;
}

uint8_t rfRemoveDevices (uint16_t Address) {
  dLink Cur = rfDevices;
  uint8_t isRemoved = 0;
  while (Cur != NULL) {
//    printf("fd A: %x, P: %p, N: %p\n", Cur->Address, Cur->Prev, Cur->Next);
    if (Cur->Address == Address) {
      Cur->Prev->Next = Cur->Next;
      Cur->Next->Prev = Cur->Prev;
      vPortFree(Cur);
      isRemoved = 1;
      break;
    }
    Cur = Cur->Next;
  }
  return isRemoved;
}

void rfSaveDevices(void) {
  //0x4002 3C00 - 0x4002 3FFF
  // Unlock flash to erase and write
  
  if (rfDevices == NULL) {
    QueueResponse((char *) "Nothing to store\n");
    return;
  }
  taskENTER_CRITICAL();

  FLASH->KEYR = 0x45670123;
  FLASH->KEYR = 0xCDEF89AB;
  while(FLASH->SR & FLASH_SR_BSY) {};    //Wait untill memory ready for erase

  FLASH->CR |= FLASH_CR_SER;              //Erase one sector
  FLASH->CR |= (FLASH_CR_SNB_0 | FLASH_CR_SNB_1 | FLASH_CR_SNB_2);    //Erase sector 7
  FLASH->CR |= FLASH_CR_STRT;

  while(FLASH->SR & FLASH_SR_BSY) {};    //Wait untill memory ready
  FLASH->CR &= ~FLASH_CR_SER;

  FLASH->CR |= FLASH_PSIZE_WORD;

  FLASH->CR |= FLASH_CR_PG;                 //Allow flash programming
  
  while(FLASH->SR & FLASH_SR_BSY) {};

  dLink Cur = rfDevices;

  uint32_t Address = 0x08060000;
  while (Cur != NULL) {
    if (Cur->Type == NULL) {
      Cur = Cur->Next;
      continue;
    }
    *(__IO uint32_t*)Address = ((uint32_t)Cur->Address) | ((uint32_t)Cur->Type << 16) | ((uint32_t)Cur->Config << 24);
    Address +=4;
    *(__IO uint32_t*)Address = (uint32_t)Cur->Salt;
    Address +=4;
    Cur = Cur->Next;
  };

  while(FLASH->SR & FLASH_SR_BSY) {};
  
  printf("SR: %d\n", FLASH->SR);

  FLASH->CR &= ~(FLASH_CR_PG);
  taskEXIT_CRITICAL();
}

void rfLoadDevices(void) {
  uint32_t Address = 0x08060000;
  uint16_t DeviceAddress = *(__IO uint16_t*)Address;
//  uint16_t DeviceConfAndType = 0x00;
  dLink Prev = NULL, Next = NULL;
  dLink newDevice = NULL;
  while (DeviceAddress != 0xFFFF) {
    Prev = newDevice;
    newDevice = ( dLink ) pvPortMalloc ( sizeof (xDevice));
    if (Prev != NULL) {
      Prev->Next = newDevice;
    } else {
      rfDevices = newDevice;
    }
    newDevice->Prev = Prev;
    newDevice->Next = Next;
    newDevice->Address = DeviceAddress;
//    DeviceConfAndType = *(__IO uint16_t*)(Address+2);
    newDevice->Type = *(__IO uint8_t*)(Address+2);
    newDevice->Config = *(__IO uint8_t*)(Address+3);
    newDevice->Salt = *(__IO uint32_t*)(Address+4);
    Address +=8;
    DeviceAddress = *(__IO uint16_t*)Address;
  }
}

void rfPingDevice(uint16_t Address) {
  dLink Device = rfGetDevice(Address);
  if (Device == NULL) {
    QueueResponse((char *) "Device not registered!");
    return;
  }
  
  uint8_t Data[2] = {0x00, 0x01};
  rfSendCommad(rfCMD_PING, Address, &Data[0], 2, Device->Salt);
}

void rfPingAllDevices(void) {
  
  dLink Cur = rfDevices;
  if (Cur == NULL) {
    QueueResponse((char *) "Devices not found!\n");
    return;
  }
  uint8_t Data[2] = {0x00, 0x01};
  while (Cur != NULL) {
//    printf("fd A: %x, P: %p, N: %p\n", Cur->Address, Cur->Prev, Cur->Next);
    rfSendCommad(rfCMD_PING, Cur->Address, &Data[0], 2, Cur->Salt);
    osDelay(20);
    Cur = Cur->Next;
  }
}

void rfSendData(uint8_t Cmd, dLink Device, char *Parameters) {
  switch (Device->Type) {
    case rfDEVICE_TYPE_1 :
    case rfDEVICE_TYPE_2 :
    {
      int NewValue[21] = {0x00};
      int readVals = sscanf(Parameters, rfCMD_DATA_MASK, 
                            &NewValue[0],
                            &NewValue[1],
                            &NewValue[2],
                            &NewValue[3],
                            &NewValue[4],
                            &NewValue[5],
                            &NewValue[6],
                            &NewValue[7],
                            &NewValue[8],
                            &NewValue[9],
                            &NewValue[10],
                            &NewValue[11],
                            &NewValue[12],
                            &NewValue[13],
                            &NewValue[14],
                            &NewValue[15],
                            &NewValue[16],
                            &NewValue[17],
                            &NewValue[18],
                            &NewValue[19],
                            &NewValue[20]);
      if (readVals > 0) {
        uint8_t Data[21] = {0x00};
        for (int i = 0; i < readVals; i++){
          Data[i] = (uint8_t)NewValue[i];
//          printf("NewValue: %03x\n", Data[i]);
        }
        rfSendCommad(Cmd, Device->Address, &Data[0], readVals, Device->Salt);
      } else {
        QueueResponse((char *)"Error: Device address not set!\n\n");
      }
    } break;
    default:
    {
      char Buf[50];
      sprintf(Buf, "Error: Device type %d not supported yet!\n\n", Device->Type);
      QueueResponse(&Buf[0]);
    } break;
  }
}

void rfSendCommad(uint8_t Command, uint16_t Address, uint8_t *Data, uint8_t Length, uint32_t Salt) {
  if (Length > 21) {
    printf("Data can't be length than 20 bytes");
  }
  uint8_t DeviceAddress[5] = {0x00};
  nRF24_GetDeviceFullAddress(Address, &DeviceAddress[0]);
  
  uint8_t *pBuf = ( uint8_t *) pvPortMalloc ( Length + 7 + 4 );
  pBuf[0] = 0x00;                           //Header, default 0x00
  memcpy(&pBuf[1], nRF24_HUB_addr, 5);
  pBuf[6] = Command;                        //Cmd
  
  memcpy(&pBuf[7], Data, Length);
  
  uint32_t crcRes = rfCalcCRC32(&pBuf[0], Length + 7);
  *((uint32_t *)(pBuf+7+Length)) = crcRes;

  printf("TX:\n\n");
  for(int i = 0; i < Length + 7 + 4; i++) {
    printf("%x ", pBuf[i]);
  }
  printf("\n\n");
  nRF24_TXPacket(&hspi2, &DeviceAddress[0], &pBuf[0], Length+7);
  vPortFree(pBuf);
}

void rfPrepareTestDevices(void) {
  const int Length = 2;
  uint16_t Addresses[Length] = {0x0001, 0x0002};
  uint8_t Types[Length] = {0x01, 0x02};
  
  for (int i = 0; i < Length; i++) {
    dLink newDevice = rfCreateDevice();
    rfUpdateDevice(Addresses[i], Types[i], 0x00);
  }
  
  rfListDevices();
}

uint32_t rfCalcCRC32(uint8_t *Data, uint8_t Length) {
  uint32_t cnt;
  CRC->CR = CRC_CR_RESET;
  /* Calculate number of 32-bit blocks */
  cnt = Length >> 2;
  /* Calculate */
  while (cnt--) {
    /* Set new value */
    CRC->DR = *(uint32_t *)Data;
    
    /* Increase by 4 */
    Data += 4;
  }
  /* Calculate remaining data as 8-bit */
  cnt = Length % 4;
  while (cnt--) {
    *((uint8_t *)&CRC->DR) = *Data++;
  }

  return CRC->DR;
}

uint32_t rfGetSalt(void) {
  return 0xAABBCCDD;  //TODO: Should be replaced to random 32bit number!
}
