#include "nRF24L01P.h"
#include "rf_cmd.h"
#include <stdlib.h>
#include <string.h>

uint8_t nRF24_Rx_Default_Addr[nRF24_TX_ADDR_WIDTH] = {0xF0, 0xFF, 0x0F, 0x00, 0xCF};
uint8_t nRF24_Rx_Temp_Addr[nRF24_TX_ADDR_WIDTH] = {0x00};

uint8_t nRF24_Rx_addr[nRF24_TX_ADDR_WIDTH] = {0xF0, 0xFF, 0x0F, 0x00, 0xCF};
uint8_t nRF24_HUB_addr[nRF24_TX_ADDR_WIDTH] = {0x00};

uint8_t nRF24_ReadMask[32] = {0xff};

uint8_t nRF24_SwitchTo = nRF24_SWITCH_TO_NONE;

SPI_HandleTypeDef *hspiTxRx;

void nRF24_SetDeviceAddress(uint8_t* Address, uint8_t Override) {
  memcpy(nRF24_Rx_addr, Address, 5);
}

void nRF24_SetDefaultDeviceAddress(void) {
  memcpy(&nRF24_Rx_Temp_Addr, nRF24_Rx_addr, 5);
  memcpy(nRF24_Rx_addr, nRF24_Rx_Default_Addr, 5);
}

uint8_t* nRF24_GetDeviceAddress(void) {
  return &nRF24_Rx_addr[0];
}

void nRF24_SetHUBAddress(uint8_t* Address) {
  memcpy(nRF24_HUB_addr, Address, 5);
}

uint8_t* nRF24_GetHUBAddress(void) {
  return &nRF24_HUB_addr[0];
}

void nRF24_SetSPIHandler(SPI_HandleTypeDef* hspi) {
  hspiTxRx = hspi;
}

// Put nRF24L01 in RX mode
void nRF24_RXMode() {
//  printf("* RXMode\n");
  HAL_SPI_FlushRxFifo(hspiTxRx);
  CE_LOW();
  nRF24_WriteBuf(nRF24_CMD_WREG | nRF24_REG_RX_ADDR_P0,nRF24_Rx_addr,nRF24_RX_ADDR_WIDTH); // Set static RX address
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_FEATURE, 0x04);  //Allow dynamic payload length
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_DYNPD, 0x1F);  //Enable dynamic payload length for 0-5 channels
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_EN_AA,0x1F); // Enable autoack for data pipe 1
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_EN_RXADDR,0x1F); // Enable data pipe 0-1
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_RF_CH,0x7C); // Set frequency channel 124 (2.524MHz)
//  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_RX_PW_P0,RX_PAYLOAD); // Set RX payload length (10 bytes)
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_RF_SETUP,0x23); // Setup: 1Mbps, 0dBm, LNA off
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_CONFIG,0x0F); // Config: CRC on (2 bytes), Power UP, RX/TX ctl = PRX
  CE_HIGH();
}

// Put nRF24L01 in TX mode
void nRF24_TXMode() {
//  printf("* TXMode\n");
  HAL_SPI_FlushRxFifo(hspiTxRx);
  CE_LOW();
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_CONFIG,0x02); // Config: Power UP
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_FEATURE, 0x04);  //Allow dynamic payload length
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_DYNPD, 0x1F);  //Enable dynamic payload length for 0-5 channels
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_EN_AA,0x1F); // Enable auto acknowledgement for data pipe 1
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_SETUP_RETR,0x1A); // Auto retransmit: wait 500us, 10 retries
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_RF_CH,0x7C); // Set frequency channel 124 (2.524MHz)
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_RF_SETUP,0x23); // Setup: 1Mbps, 0dBm, LNA off
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_CONFIG,0x0E); // Config: CRC on (2 bytes), Power UP, RX/TX ctl = PTX
  CE_HIGH();
}

uint8_t nRF24_ReadReg(uint8_t Reg) {
  HAL_StatusTypeDef status = HAL_OK;
  uint8_t pRxData = 0x00;
  CSN_LOW();
  status = HAL_SPI_Transmit(hspiTxRx, &Reg, 1, 10);
  HAL_SPI_TransmitReceive(hspiTxRx, (uint8_t *)&nRF24_ReadMask, &pRxData, 1, SPI_TIMEOUT);
  CSN_HIGH();
  
  if(status != HAL_OK)
  {
    SPIx_Error();
  }
  return pRxData;
}

uint8_t nRF24_SendCmd(uint8_t Cmd)
{
  uint8_t Status = 0x00;
  CSN_LOW();
  HAL_SPI_TransmitReceive(hspiTxRx, &Cmd, &Status, 1, SPI_TIMEOUT);
  CSN_HIGH();
  return Status;
}

void nRF24_RWReg(uint8_t Reg, uint8_t Data)
{
  printf("R: %x, D: %x\n", Reg, Data);
  uint8_t pBuf[2] = {Reg, Data};
  HAL_StatusTypeDef status = HAL_OK;
  CSN_LOW();
  status = HAL_SPI_Transmit(hspiTxRx, (uint8_t*) &pBuf[0], 2, SPI_TIMEOUT);
  CSN_HIGH();
  
//  printf("status: %x\n", status);
  if(status != HAL_OK)
  {
    SPIx_Error();
  }
}

void nRF24_WriteBuf(uint8_t Reg, uint8_t *Data, uint8_t size)
{
  printf("WriteBuf. Reg: %x\n", Reg);
  for (uint8_t i= 0; i<size; i++) {
    printf("%x ", Data[i]);
  }
  printf("\n");
  HAL_StatusTypeDef status = HAL_OK;
  CSN_LOW();
  status = HAL_SPI_Transmit(hspiTxRx, (uint8_t*) &Reg, 1, SPI_TIMEOUT);
  status = HAL_SPI_Transmit(hspiTxRx, Data, size, SPI_TIMEOUT);
  CSN_HIGH();
  
  if(status != HAL_OK)
  {
    SPIx_Error();
  }
}

void nRF24_ReadBuf(uint8_t Reg, uint8_t *Data, uint8_t count)
{
  HAL_SPI_FlushRxFifo(hspiTxRx);
  CSN_LOW();
  HAL_SPI_Transmit(hspiTxRx, (uint8_t*) &Reg, 1, SPI_TIMEOUT);
  HAL_SPI_Receive(hspiTxRx, Data, count, SPI_TIMEOUT);
  CSN_HIGH();
}

uint8_t nRF24_TXPacket(uint8_t *nRF24_Tx_addr, uint8_t *pBuf, uint8_t Length) {

  nRF24_HandleStatus();
  
  uint8_t Tx_addr[5] = {0x00};
  memcpy(Tx_addr, nRF24_Tx_addr, 5);
/*  printf("TX_To:\n");
  for(int i =0; i<5;i++) {
    printf("%x ", Tx_addr[i]);
  }
  printf("\npBuf:\n");
  for(int i =0; i<Length;i++) {
    printf("%x ", pBuf[i]);
  }
  printf("\n");
*/
  CE_LOW();
  nRF24_WriteBuf(nRF24_CMD_WREG | nRF24_REG_TX_ADDR,    &Tx_addr[0], nRF24_TX_ADDR_WIDTH); // Set static TX address
  nRF24_WriteBuf(nRF24_CMD_WREG | nRF24_REG_RX_ADDR_P0, &Tx_addr[0], nRF24_TX_ADDR_WIDTH); // Set static RX address for auto ack
  nRF24_WriteBuf(nRF24_CMD_W_TX_PAYLOAD, pBuf, Length); // Write specified buffer to FIFO
  nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_CONFIG,0x0E); // Config: CRC on (2 bytes), Power UP, RX/TX ctl = PTX
  CE_HIGH(); // CE pin high => Start transmit
  uint8_t i = 255;
  while(i){i--;};
  CE_LOW();

  return 0x00;
}

void nRF24_RXPacket(uint8_t* pBuf, uint8_t* Length) {
  HAL_SPI_FlushRxFifo(hspiTxRx);
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
  HAL_SPI_FlushRxFifo(hspiTxRx);
  uint8_t Status = 0x00;
  uint8_t Cmd = nRF24_CMD_NOP;
  CSN_LOW();
  HAL_SPI_TransmitReceive(hspiTxRx, &Cmd, &Status, 1, SPI_TIMEOUT);
  CSN_HIGH();
//  printf("S: %x\n", hspi->Instance->DR);
  return Status;
}

uint8_t nRF24_HandleStatus() {
  uint8_t Status = nRF24_GetStatus();
  printf("Status: %x\n", Status);
  do {
    if (Status != 0x0E) {
      nRF24_RWReg(nRF24_CMD_WREG | nRF24_REG_STATUS, Status);
    }
    uint8_t Flags = 0x00;
    if (Status & nRF24_MASK_MAX_RT) {
      //Transmit error. Clear package

      printf("nRF24_MASK_MAX_RT\n");
      nRF24_SendCmd(nRF24_CMD_FLUSH_TX);
      Flags |= nRF24_MASK_MAX_RT;
      if (nRF24_SwitchTo == nRF24_SWITCH_TO_RX) {
        nRF24_RXMode();
        nRF24_SwitchTo = nRF24_SWITCH_TO_NONE;
      }
    }
    
    if (Status & nRF24_MASK_TX_DS) {
      //Package sent successfully. Just clear a flag
      printf("nRF24_MASK_TX_DS\n");
      Flags |= nRF24_MASK_TX_DS;
      if (nRF24_SwitchTo == nRF24_SWITCH_TO_RX) {
        nRF24_RXMode();
        nRF24_SwitchTo = nRF24_SWITCH_TO_NONE;
      }
    }
    
    if ((Status & nRF24_MASK_RX_DR) || ((Status & 0x0E) != 0x0E)) {
      //Got a package, read it from buffer 
      printf("nRF24_MASK_RX_DR\n");
      Flags |= nRF24_MASK_RX_DR;

      uint8_t pBuf[32] = {0x00};
      uint8_t Length = 0;

      nRF24_RXPacket((uint8_t *)&pBuf[0], &Length);
/*      printf("Length: %d, Data:", Length);
      for (int i = 0; i < Length;i++) {
        printf("%x ", pBuf[i]);
      }
      printf("\n");
*/
      ProcessData(&pBuf[0], Length);
    }
    Status = nRF24_GetStatus();
  } while (Status != 0x0E);
  return Status;
}

/**
  * @brief  SPI error treatment function.
  * @param  None
  * @retval None
  */
static void SPIx_Error (void)
{
  printf("SPIx_Error\n");
}
