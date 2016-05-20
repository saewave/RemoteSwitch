#include "nRF24L01P.h"
#include "rf_cmd.h"
#include <string.h>

uint8_t nRF24_Rx_Default_Addr[nRF24_TX_ADDR_WIDTH] = {0xF0, 0xFF, 0x0F, 0x00, 0xCF};
uint8_t nRF24_Rx_Temp_Addr[nRF24_TX_ADDR_WIDTH] = {0x00, 0x00, 0x00, 0x00, 0x00};

uint8_t nRF24_Rx_addr[nRF24_TX_ADDR_WIDTH] = {0xF0, 0xFF, 0x0F, 0x00, 0xCF};
uint8_t nRF24_HUB_addr[nRF24_TX_ADDR_WIDTH] = {0x00, 0x00, 0x00, 0x00, 0x00};

uint8_t nRF24_SwitchTo = nRF24_SWITCH_TO_NONE;

//SPI_HandleTypeDef *hspiTxRx;

void nRF24_SetDeviceAddress(uint8_t* Address, uint8_t Override) {
  memcpy(nRF24_Rx_addr, Address, 5);
}

void nRF24_SetDefaultDeviceAddress(void) {
  memcpy(nRF24_Rx_Temp_Addr, nRF24_Rx_addr, 5);
  memcpy(nRF24_Rx_addr, nRF24_Rx_Default_Addr, 5);
}

uint8_t* nRF24_GetDeviceAddress(void) {
  return nRF24_Rx_addr;
}

void nRF24_SetHUBAddress(uint8_t* Address) {
  memcpy(nRF24_HUB_addr, Address, 5);
}

uint8_t* nRF24_GetHUBAddress(void) {
  return nRF24_HUB_addr;
}

void nRF24_SetSwitchTo(uint8_t SwitchTo) {
  nRF24_SwitchTo = SwitchTo;
}

/*void nRF24_SetSPIHandler(SPI_HandleTypeDef* hspi) {
  hspiTxRx = hspi;
}*/

// Put nRF24L01 in RX mode
void nRF24_RXMode() {
//  printf("* RXMode\n");
  SPI_FlushRxFifo();
  CE_LOW();
  uint8_t reg, i = 0x00;
  do {
    nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_CONFIG, 0x0D); // Config: Power Off
    reg = nRF24_ReadReg( nRF24_REG_CONFIG );
    i++;
//    dxprintf("reg: %x\n", reg);
  } while (reg != 0x0D && i < 0xFF);
/*
  dxprintf("**************** nRF24_RXMode\n");
  uint8_t arr[5] = {1,2,3,4,5};
  for(int i = 0; i< 2; i++) {
    uint16_t* test16 = (uint16_t*)nRF24_Rx_addr;
    dxprintf("test16 %x\n", test16);
    dxprintf("%d: %x\n", i, * (test16 + i));
  }
*/
  nRF24_WriteBuf(nRF24_CMD_WREG | nRF24_REG_RX_ADDR_P0, nRF24_Rx_addr, nRF24_RX_ADDR_WIDTH); // Set static RX address
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_FEATURE, 0x04);  //Allow dynamic payload length
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_DYNPD, 0x1F);  //Enable dynamic payload length for 0-5 channels
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_EN_AA,0x1F); // Enable autoack for data pipe 1
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_EN_RXADDR,0x1F); // Enable data pipe 0-1
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_RF_CH,0x7C); // Set frequency channel 124 (2.524MHz)
//  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_RX_PW_P0,RX_PAYLOAD); // Set RX payload length (10 bytes)
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_RF_SETUP,0x27); // Setup: 250Kbps, 0dBm, LNA off
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_CONFIG,0x0F); // Config: CRC on (2 bytes), Power UP, RX/TX ctl = PRX
//  HAL_Delay(5);
  uint16_t k = 1000;while(k){k--;};
  CE_HIGH();
}

// Put nRF24L01 in TX mode
void nRF24_TXMode() {
//  printf("* TXMode\n");
  SPI_FlushRxFifo();
  CE_LOW();
  uint8_t reg, i = 0x00;
  do {
    nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_CONFIG, 0x0C); // Config: Power Off
    reg = nRF24_ReadReg( nRF24_REG_CONFIG );
    i++;
  } while (reg != 0x0C && i < 0xFF);
  
//  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_CONFIG,0x02); // Config: Power UP
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_FEATURE, 0x04);  //Allow dynamic payload length
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_DYNPD, 0x1F);  //Enable dynamic payload length for 0-5 channels
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_EN_AA,0x1F); // Enable auto acknowledgement for data pipe 1
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_SETUP_RETR,0x1A); // Auto retransmit: wait 500us, 10 retries
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_RF_CH,0x7C); // Set frequency channel 124 (2.524MHz)
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_RF_SETUP,0x27); // Setup: 250Kbps, +7dBm(SI24R1)
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_CONFIG,0x0E); // Config: CRC on (2 bytes), Power UP, RX/TX ctl = PTX
  uint16_t k = 5000;while(k){k--;};
  CE_HIGH();
  
}

uint8_t nRF24_ReadReg(uint8_t Reg) {
  uint8_t pRxData = 0x00;
  
  CSN_LOW();
  SPI_SendData(&Reg, 1);
  SPI_FlushRxFifo();
  SPI_ReadData(&pRxData, 1, 0xff);
  CSN_HIGH();
  
  return pRxData;
}

uint8_t nRF24_SendCmd(uint8_t Cmd)
{
  uint8_t Status = 0x00;

  CSN_LOW();
  SPI_SendData(&Cmd, 1);
  CSN_HIGH();
  return Status;
}

void nRF24_RWReg(uint8_t Reg, uint8_t Data)
{

//  dxprintf("R: %x, D: %x\n", Reg, Data);
  uint8_t pBuf[2] = {Reg, Data};
  CSN_LOW();
  SPI_SendData(pBuf, 2);
  CSN_HIGH();
}

void nRF24_WriteBuf(uint8_t Reg, uint8_t *Data, uint8_t size)
{
/*
  dxprintf("nRF24_WriteBuf. Reg: %x\n", Reg);
  for (uint8_t i= 0; i<size; i++) {
    dxprintf("%d: %x\n", i, * ( ((uint16_t*)Data) + i));
  }
  dxputs("\n");
*/
  CSN_LOW();
  
  SPI_SendData(&Reg, 1);
  SPI_SendData(Data, size);
  CSN_HIGH();
}

void nRF24_ReadBuf(uint8_t Reg, uint8_t *Data, uint8_t count)
{
  SPI_FlushRxFifo();
  CSN_LOW();
  SPI_SendData(&Reg, 1);
  SPI_FlushRxFifo();
  SPI_ReadData(Data, count, 0xFF);
  CSN_HIGH();
}

uint8_t nRF24_TXPacket(uint8_t *nRF24_Tx_addr, uint8_t *pBuf, uint8_t Length) {

  nRF24_HandleStatus();
  nRF24_TXMode();
  /*
  uint8_t Tx_addr[5];
  memcpy(Tx_addr, nRF24_Tx_addr, 5);
  */
/*  dxputs("TX_To:\n");
  for(int i =0; i<5;i++) {
    dxprintf("%x ", nRF24_Tx_addr[i]);
  }
  dxputs("\npBuf:\n");
  for(int i =0; i<Length;i++) {
    dxprintf("%x ", pBuf[i]);
  }
  dxputs("\n");
*/
  CE_LOW();
  nRF24_WriteBuf(nRF24_CMD_WREG | nRF24_REG_TX_ADDR,    nRF24_Tx_addr, nRF24_TX_ADDR_WIDTH); // Set static TX address
  nRF24_WriteBuf(nRF24_CMD_WREG | nRF24_REG_RX_ADDR_P0, nRF24_Tx_addr, nRF24_TX_ADDR_WIDTH); // Set static RX address for auto ack
  nRF24_WriteBuf(nRF24_CMD_W_TX_PAYLOAD, pBuf, Length); // Write specified buffer to FIFO
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_CONFIG,0x0E); // Config: CRC on (2 bytes), Power UP, RX/TX ctl = PTX
  uint16_t i = 5000;
  while(i){i--;};
  CE_HIGH(); // CE pin high => Start transmit
  i = 255;
  while(i){i--;};
  CE_LOW();
  
  i = 5000;
  while(i){i--;};
  
  return 0x00;
}

void nRF24_RXPacket(uint8_t *pBuf, uint8_t *Length) {
  SPI_FlushRxFifo();
  uint8_t PayloadWidth = nRF24_ReadReg(nRF24_CMD_R_RX_PL_WID);
  if (PayloadWidth > 32) {
    nRF24_SendCmd(nRF24_CMD_FLUSH_RX);
    PayloadWidth = 0;
  } else {
    nRF24_ReadBuf(nRF24_CMD_R_RX_PAYLOAD, pBuf, PayloadWidth);
  }
  *Length = PayloadWidth;
}

uint8_t nRF24_GetStatus() {
  SPI_FlushRxFifo();
  uint8_t Status = 0x00;
  CSN_LOW();
  SPI_ReadData(&Status, 1, nRF24_CMD_NOP);
  CSN_HIGH();
//  printf("S: %x\n", hspi->Instance->DR);
  return Status;
}

uint8_t nRF24_HandleStatus() {
//  dxputs("*** nRF24_HandleStatus ***\n");
  SPI_FlushRxFifo();
  uint8_t Status = 0x00;
  dxprintf("Status: %x\n", Status);
  
  do {
    Status = nRF24_GetStatus();
    
    if (Status != 0x0E) {
      nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_STATUS, Status);
    }
    
    uint8_t Flags = 0x00;
    if (Status & nRF24_MASK_MAX_RT) {
      //Transmit error. Clear package

      dxputs("nRF24_MASK_MAX_RT\n");
      nRF24_SendCmd(nRF24_CMD_FLUSH_TX);
      Flags |= nRF24_MASK_MAX_RT;
      if (nRF24_SwitchTo == nRF24_SWITCH_TO_RX) {
        nRF24_RXMode();
        nRF24_SwitchTo = nRF24_SWITCH_TO_NONE;
      }
    }
    

    if (Status & nRF24_MASK_TX_DS) {
      //Package sent successfully. Just clear a flag
      dxputs("nRF24_MASK_TX_DS\n");
      Flags |= nRF24_MASK_TX_DS;
      if (nRF24_SwitchTo == nRF24_SWITCH_TO_RX) {
        nRF24_RXMode();
        nRF24_SwitchTo = nRF24_SWITCH_TO_NONE;
      }
    }

    if ((Status & nRF24_MASK_RX_DR) || ((Status & 0x0E) != 0x0E)) {
      //Got a package, read it from buffer 
      dxputs("nRF24_MASK_RX_DR\n");
      Flags |= nRF24_MASK_RX_DR;

      uint8_t pBuf[32];
      uint8_t Length = 0;

      nRF24_RXPacket(pBuf, &Length);

      ProcessData(pBuf, Length);
    }
//    Status = nRF24_GetStatus();
//    dxprintf("Post Status: %x\n", Status);
  } while (Status != 0x0E);
  return Status;
}
