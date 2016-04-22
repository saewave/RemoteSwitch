#include "at_processor.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "cmsis_os.h"
#include "nRF24L01P.h"
#include "UUID.h"
#include "spi.h"
#include "rf_device.h"
#include <stdlib.h>
#include <stdio.h>

extern osMessageQId U2TxQueueHandle;
//extern SPI_HandleTypeDef hspi2;

void ProcessATCommand(char *Data, uint8_t Length) {
//  printf("CMD: %s\n", Data);
  char *Parameters = strchr(Data, ':');
  uint8_t ParametersPos = 0x00;
  if ( Parameters == NULL ) {
//    printf("Len: %d, no params\n", Length);
  } else {
    ParametersPos = Parameters-Data;
    printf("Len: %d, Params: %d\n", Length, ParametersPos);
  }
  /*
  char *Command = (char*)malloc(ParametersPos - 3 + 1);
  if (Command == NULL) {
    printf("Can't allocate memmory for command: %s\n", Data);
    return;
  }
  strncpy(Command, &Data[3], (ParametersPos - 3));
  printf("CMD: %s\n", Command);
  */
  
  int CommandProcessed = 0;
  
  if (strncmp(&Data[3], (char *) "PING", 4) == 0) {
    CmdPING();
    CommandProcessed = 1;
  }
  
  if (strncmp(&Data[3], (char *) "FIND_NEW_DEVICE", 15) == 0 || strncmp(&Data[3], (char *) "FIND", 4) == 0) {
    CmdFindNewDevice();
    CommandProcessed = 1;
  }
  
  if (strncmp(&Data[3], (char *) "CHECK_TX_STATUS", 15) == 0) {
    QueueResponse((char *)"CHECK_TX_STATUS\n");
    CmdCheckTxRxStatus(CHIP_Tx);
    CommandProcessed = 1;
  }
  
  if (strncmp(&Data[3], (char *) "CHECK_RX_STATUS", 15) == 0) {
    QueueResponse((char *)"CHECK_RX_STATUS\n");
    CmdCheckTxRxStatus(CHIP_Rx);
    CommandProcessed = 1;
  }
  
  if (strncmp(&Data[3], (char *) "HANDLE_TX_STATUS", 16) == 0) {
    QueueResponse((char *)"HANDLE_TX_STATUS\n");
    CmdHandleTxRxStatus(CHIP_Tx);
    CommandProcessed = 1;
  }
  
  if (strncmp(&Data[3], (char *) "HANDLE_RX_STATUS", 16) == 0) {
    QueueResponse((char *)"HANDLE_RX_STATUS\n");
    CmdHandleTxRxStatus(CHIP_Rx);
    CommandProcessed = 1;
  }
  
  if (strncmp(&Data[3], (char *) "ADD_DEVICE", 10) == 0) {
    QueueResponse((char *)"ADD_DEVICE\n");
    rfCreateDevice();
    CommandProcessed = 1;
  }
  
  if (strncmp(&Data[3], (char *) "LIST_DEVICES", 12) == 0) {
    QueueResponse((char *)"LIST_DEVICES\n");
    rfListDevices();
    CommandProcessed = 1;
  }
  
  if (strncmp(&Data[3], (char *) "SAVE_DEVICES", 12) == 0) {
    QueueResponse((char *)"SAVE_DEVICES\nPlease wait...  ");
    rfSaveDevices();
    QueueResponse((char *)"Done!\n");
    CommandProcessed = 1;
  }
  
  if (strncmp(&Data[3], (char *) "LOAD_DEVICES", 12) == 0) {
    QueueResponse((char *)"LOAD_DEVICES\nPlease wait...  \n\n");
    rfLoadDevices();
    rfListDevices();
    QueueResponse((char *)"Done!\n");
    CommandProcessed = 1;
  }
  
  if (strncmp(&Data[3], (char *) "PREPARE_TEST_DEVICES", 20) == 0) {
    QueueResponse((char *)"PREPARE_TEST_DEVICES\n");
    rfPrepareTestDevices();
    QueueResponse((char *)"Done!\n");
    CommandProcessed = 1;
  }
  
  if (strncmp(&Data[3], (char *) "PING_DEVICE", 11) == 0) {
    QueueResponse((char *)"***PING_DEVICE NOT IMPLEMENTED!!!***");
    
    //ParametersPos
    
//    rfPingDevice();
//    QueueResponse((char *)"Done!\n");
    CommandProcessed = 1;
  }
  
  if (strncmp(&Data[3], (char *) "PING_ALL_DEVICES", 16) == 0) {
    QueueResponse((char *)"PING_ALL_DEVICES\nPlease wait...  ");
    rfPingAllDevices();
    QueueResponse((char *)"Done!\n");
    CommandProcessed = 1;
  }
  
  if (strncmp(&Data[3], (char *) "SELF_TEST", 9) == 0) {
    QueueResponse((char *)"SELF_TEST\nPlease wait...  ");
    //TODO: Implement it!

    QueueResponse((char *)"Done!\n");
    CommandProcessed = 1;
  }
  
  if (strncmp(&Data[3], (char *) "SEND_DATA", 9) == 0) {
    QueueResponse((char *)"SEND_DATA\n");
    char Parameters[Length - ParametersPos + 1];
    strlcpy(Parameters, &Data[ParametersPos + 1], Length - ParametersPos);
    int DevAddress = 0x00;
    uint8_t ArgumentsLength = Length - ParametersPos - 6;
//    printf("ArgumentsLength: %d\n", ArgumentsLength);
    char Arguments[ArgumentsLength];
    if (sscanf(Parameters, "%4x:%s", &DevAddress, Arguments) > 0) {
/*      printf("Adr: %4x\n", DevAddress);
      for (int i=0; i<ArgumentsLength; i++) {
        printf("%c\n", Arguments[i]);
      }*/
      dLink Device = rfGetDevice(DevAddress);
      if (Device == NULL) {
        QueueResponse((char *)"Error: Device address not set!\n\n");
      } else if (Device->Type == 0x00) {
        QueueResponse((char *)"Error: Device type not valid!\n\n");
      } else {
        rfSendData(rfCMD_W_DATA, Device, Arguments);
      }
    } else {
      QueueResponse((char *)"Error: Device address not set!\n\n");
    }
    CommandProcessed = 1;
  }
  
  if (CommandProcessed == 0) {
    QueueResponse((char *)"Error: Unknown command!\n\n");
  }
  
//  free(Command);
}

void CmdHandleTxRxStatus(uint8_t Chip) {
  nRF24_HandleStatus(&hspi2, Chip);
  uint8_t Status = nRF24_GetStatus(&hspi2, Chip);
  char Buf[17];
  sprintf(Buf, "Done, status: %x\n", Status);
  QueueResponse(Buf);
}

void CmdCheckTxRxStatus(uint8_t Chip) {
  uint8_t Status = nRF24_GetStatus(&hspi2, Chip);
  char Buf[10];
  sprintf(Buf, "Status: %x\n", Status);
  QueueResponse(Buf);
}

void CmdFindNewDevice (void) {
  QueueResponse((char *)"FIND-NEW-DEVICE\nPlease wait...\n");
  
  //TODO: Generate new device address besed on table of current devices
  dLink newDevice = rfCreateDevice();
  
  uint8_t Data[17] = {
  0x00, 
  nRF24_HUB_addr[0],
  nRF24_HUB_addr[1],
  nRF24_HUB_addr[2],
  nRF24_HUB_addr[3],
  nRF24_HUB_addr[4],
  rfCMD_DISCOVER,
  nRF24_NET_TX_START_addr[0], 
  nRF24_NET_TX_START_addr[1], 
  nRF24_NET_TX_START_addr[2], 
  (uint8_t)(newDevice->Address >> 8), 
  (uint8_t) newDevice->Address, 0x01};

  *((uint32_t *)(Data+13)) = newDevice->Salt;
    
  nRF24_TXPacket(&hspi2, &nRF24_DEVICE_CFG_addr[0], &Data[0], 17);
    
//  rfSendCommad(rfCMD_DISCOVER, newDevice->Address, &Data[0], 11, newDevice->Salt);

}

void CmdPING (void) {
  QueueResponse((char *)"PONG!\n");
}

void QueueResponse (char *Response) {
  uint8_t Length = strlen(Response);
  uint8_t * Buf = (uint8_t*)pvPortMalloc(Length);
  if (Buf == NULL) {
    printf("Error: can't allocate memmory!\n");
  } else {
    memcpy(Buf, Response, Length);
    xCmdResponse qCmdResponse;
    qCmdResponse.cLength = Length;
    qCmdResponse.cData = Buf;

    if ( xQueueSend( U2TxQueueHandle, &(qCmdResponse), portMAX_DELAY) != pdPASS ) {
      //Error, the queue if full!!!
      //TODO: handle this error
      printf("Error to add to U2TxQueue!\n");
    }
//    free(Buf);
  }
}
