#include "rf_processor.h"
#include "at_processor.h"
#include "rf_device.h"
#include "FreeRTOS.h"

#define rfHEADER_MASK_RESPONSE 0x10

void rfProcessCommand (uint8_t *Data, uint8_t Length) {
  uint8_t Header = Data[0];       // Header of command

  uint8_t Command = Data[6];     // Command we assume that command length = 1 byte
  printf("rfCMD: %02x\n", Command);
  switch (Command) {
    case rfCMD_PING:     //PING answer
      {
        if (!(Header & rfHEADER_MASK_RESPONSE)) return;
        dLink updDevice = rfUpdateDevice((((uint16_t)Data[4] << 8) | Data[5]), NULL, rfCONFIG_MASK_ONLINE);
        char Buf[60];
        if (updDevice == NULL) {
          sprintf(Buf, "WARN! Device with adr: 0x%x%x NOT registered, but pinged!\n", Data[4], Data[5]);
          QueueResponse(&Buf[0]);
        } else {
          sprintf(Buf, "Device with adr: 0x%x%x is ONLINE!\n", Data[4], Data[5]);
          QueueResponse(&Buf[0]);
        }
      } break;
    case rfCMD_DISCOVER:  //DISCOVER answer
      {
        if (!(Header & rfHEADER_MASK_RESPONSE)) return;
//        printf("Addr: %04x, Type: %02x\n", (((uint16_t)Data[4] << 8) | Data[5]), Data[7]);
        dLink updDevice = rfUpdateDevice((((uint16_t)Data[4] << 8) | Data[5]), Data[7], rfCONFIG_MASK_CONFIRMED | rfCONFIG_MASK_ONLINE | rfCONFIG_MASK_ENABLED);
        char Buf[55] = {0x00};
        if (updDevice) {
          sprintf(Buf, "Device: Adr: 0x%x%x Type: 0x%x connected\n", Data[4], Data[5], Data[7]);
          QueueResponse(&Buf[0]);
        } else {
          sprintf(Buf, "Device: Adr: 0x%x%x Type: 0x%x NOT connected\n", Data[4], Data[5], Data[7]);
          QueueResponse(&Buf[0]);
        };
      } break;
  }
}

void rfProcessError (uint8_t *Data, uint8_t Length) {
  
}
