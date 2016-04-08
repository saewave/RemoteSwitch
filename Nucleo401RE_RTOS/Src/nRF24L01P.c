#include "nRF24L01P.h"
#include "rf_processor.h"
#include "at_processor.h"

uint8_t nRF24_HUB_addr[nRF24_RX_ADDR_WIDTH] = {0xEE, 0x12, 0x13, 0x14, 0x00};
//uint8_t nRF24_ACK_RX_addr[nRF24_RX_ADDR_WIDTH] = {0x11, 0x12, 0x13, 0x14, 0x00};

uint8_t nRF24_NET_TX_START_addr[nRF24_RX_ADDR_WIDTH] = {0x11, 0x12, 0x13, 0x14, 0x01};

uint8_t nRF24_DEVICE_CFG_addr[nRF24_RX_ADDR_WIDTH] = {0xF0, 0xFF, 0x0F, 0x00, 0xCF};

uint8_t nRF24_ReadMask[32] = {0xff};

void nRF24_Configure(void) {
  /*nRF24_RX_addr[0] = nRF24_TX_addr[0] = STM32_UUID[8];
  nRF24_RX_addr[1] = nRF24_TX_addr[1] = STM32_UUID[9];
  nRF24_RX_addr[2] = nRF24_TX_addr[2] = STM32_UUID[10];
  nRF24_RX_addr[3] = nRF24_TX_addr[3] = STM32_UUID[11];
  nRF24_RX_addr[4] = nRF24_TX_addr[4] = 0x00;
  */

  CE_LOW(CHIP_Tx);
  CSN_HIGH(CHIP_Tx);
  CE_LOW(CHIP_Rx);
  CSN_HIGH(CHIP_Rx);
}

void nRF24_GetDeviceFullAddress(uint16_t ShortAddress, uint8_t *FullAddress) {
  memcpy(FullAddress, nRF24_NET_TX_START_addr, 5);
  FullAddress[3] = (uint8_t)(ShortAddress >> 8);
  FullAddress[4] = (uint8_t)(ShortAddress);
}

// Put nRF24L01 in RX mode
void nRF24_RXMode(SPI_HandleTypeDef* hspi, uint8_t RX_PAYLOAD) {
  printf("nRF24_RXMode\n");
  CE_LOW(CHIP_Rx);
  nRF24_WriteBuf(hspi, CHIP_Rx, nRF24_CMD_WREG | nRF24_REG_RX_ADDR_P0, nRF24_HUB_addr, nRF24_RX_ADDR_WIDTH); // Set static RX address
  nRF24_RWReg(hspi, CHIP_Rx, nRF24_CMD_WREG | nRF24_REG_FEATURE, 0x04);  //Allow dynamic payload length
  nRF24_RWReg(hspi, CHIP_Rx, nRF24_CMD_WREG | nRF24_REG_DYNPD, 0x1F);  //Enable dynamic payload length for 0-5 channels
  nRF24_RWReg(hspi, CHIP_Rx, nRF24_CMD_WREG | nRF24_REG_EN_AA,0x1F); // Enable acknowledgement for data pipe 1
  nRF24_RWReg(hspi, CHIP_Rx, nRF24_CMD_WREG | nRF24_REG_EN_RXADDR,0x1F); // Enable data pipe 0-1
  nRF24_RWReg(hspi, CHIP_Rx, nRF24_CMD_WREG | nRF24_REG_RF_CH,0x7C); // Set frequency channel 110 (2.510MHz)
//  nRF24_RWReg(hspi, CHIP_Rx, nRF24_CMD_WREG | nRF24_REG_RX_PW_P0,RX_PAYLOAD); // Set RX payload length (10 bytes)
  nRF24_RWReg(hspi, CHIP_Rx, nRF24_CMD_WREG | nRF24_REG_RF_SETUP,0x23); // Setup: 250Kbps, 0dBm, LNA off
  nRF24_RWReg(hspi, CHIP_Rx, nRF24_CMD_WREG | nRF24_REG_CONFIG,0x0F); // Config: CRC on (2 bytes), Power UP, RX/TX ctl = PRX
  CE_HIGH(CHIP_Rx);
}

// Put nRF24L01 in TX mode
void nRF24_TXMode(SPI_HandleTypeDef* hspi) {
  printf("nRF24_TXMode\n");
  CE_LOW(CHIP_Tx);
  nRF24_RWReg(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_CONFIG,0x02); // Config: Power UP
  nRF24_RWReg(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_FEATURE, 0x04);  //Allow dynamic payload length
  nRF24_RWReg(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_DYNPD, 0x1F);  //Enable dynamic payload length for 0-5 channels
  nRF24_RWReg(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_EN_AA,0x1F); // Enable auto acknowledgement for data pipe 1
  nRF24_RWReg(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_SETUP_RETR,0x1A); // Auto retransmit: wait 500us, 10 retries
  nRF24_RWReg(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_RF_CH,0x7C); // Set frequency channel 110 (2.510MHz)
  nRF24_RWReg(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_RF_SETUP,0x26); // Setup: 250Kbps, 0dBm, LNA off
  nRF24_RWReg(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_CONFIG,0x0E); // Config: CRC on (2 bytes), Power UP, RX/TX ctl = PTX
  CE_HIGH(CHIP_Tx);
}

void nRF24_DumpRegisters(SPI_HandleTypeDef* hspi, uint8_t Chip) {
  uint8_t RegVal = 0x00;
  printf("Dump All Registers\n\n");
  uint8_t rDump[6] = {nRF24_REG_STATUS, nRF24_REG_EN_AA, nRF24_REG_SETUP_RETR, nRF24_REG_RF_CH, nRF24_REG_RF_SETUP, nRF24_REG_CONFIG};
  for (int reg = 0; reg<6; reg++) {
    RegVal = nRF24_ReadReg(hspi, Chip, nRF24_CMD_RREG | rDump[reg]);
    printf("Reg %x: %x\n", rDump[reg], RegVal);
  }
  
  printf("Dump Done\n\n");
}

uint8_t nRF24_ReadReg(SPI_HandleTypeDef* hspi, uint8_t Chip, uint8_t Reg) {
  HAL_StatusTypeDef status = HAL_OK;
  uint8_t pRxData = 0x00;
  CSN_LOW(Chip);
  status = HAL_SPI_Transmit(hspi, &Reg, 1, 10);
  HAL_SPI_TransmitReceive(hspi, (uint8_t *)&nRF24_ReadMask, &pRxData, 1, SPI_TIMEOUT);
  CSN_HIGH(Chip);
  
  if(status != HAL_OK)
  {
    SPIx_Error();
  }
  return pRxData;
}

uint8_t nRF24_SendCmd(SPI_HandleTypeDef* hspi, uint8_t Chip, uint8_t Cmd)
{
  uint8_t Status = 0x00;
  CSN_LOW(Chip);
  HAL_SPI_TransmitReceive(hspi, &Cmd, &Status, 1, SPI_TIMEOUT);
  CSN_HIGH(Chip);
  return Status;
}

void nRF24_RWReg(SPI_HandleTypeDef* hspi, uint8_t Chip, uint8_t Reg, uint8_t Data)
{
  printf("R: %x, D: %x\n", Reg, Data);
  uint8_t pBuf[2] = {Reg, Data};
  HAL_StatusTypeDef status = HAL_OK;
  CSN_LOW(Chip);
  status = HAL_SPI_Transmit(hspi, (uint8_t*) &pBuf[0], 2, SPI_TIMEOUT);
  CSN_HIGH(Chip);
  for(int i=0;i<20;i++){};
//  printf("status: %x\n", status);
  if(status != HAL_OK)
  {
    SPIx_Error();
  }
}

void nRF24_WriteBuf(SPI_HandleTypeDef* hspi, uint8_t Chip, uint8_t Reg, uint8_t *Data, uint8_t size)
{
  printf("WriteBuf. Reg: %x\n", Reg);
  for (uint8_t i= 0; i<size; i++) {
    printf("%x ", Data[i]);
  }
  printf("\n");
  
  HAL_StatusTypeDef status = HAL_OK;
  CSN_LOW(Chip);
  status = HAL_SPI_Transmit(hspi, (uint8_t*) &Reg, 1, SPI_TIMEOUT);
  if(status != HAL_OK)
  {
    SPIx_Error();
  }
  status = HAL_SPI_Transmit(hspi, Data, size, SPI_TIMEOUT);
  CSN_HIGH(Chip);
  
  if(status != HAL_OK)
  {
    SPIx_Error();
  }
}

void nRF24_ReadBuf(SPI_HandleTypeDef* hspi, uint8_t Chip, uint8_t Reg, uint8_t *Data, uint8_t count)
{
  CSN_LOW(Chip);
  HAL_SPI_Transmit(hspi, (uint8_t*) &Reg, 1, SPI_TIMEOUT);
  HAL_SPI_Receive(hspi, Data, count, SPI_TIMEOUT);
  CSN_HIGH(Chip);
}

// Send data packet
// input:
//   pBuf - buffer with data to send
// return:
//   nRF24_MASK_MAX_RT - if transmit failed with maximum auto retransmit count
//   nRF24_MAX_TX_DS - if transmit succeed
//   contents of STATUS register otherwise
uint8_t nRF24_TXPacket(SPI_HandleTypeDef* hspi, uint8_t *nRF24_TX_addr, uint8_t *pBuf, uint8_t Length) {

  nRF24_HandleStatus(hspi, CHIP_Tx);
  
  CE_LOW(CHIP_Tx);
  nRF24_WriteBuf(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_TX_ADDR,    nRF24_TX_addr, nRF24_TX_ADDR_WIDTH); // Set static TX address
  nRF24_WriteBuf(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_RX_ADDR_P0, nRF24_TX_addr, nRF24_TX_ADDR_WIDTH); // Set static RX address for auto ack
  nRF24_WriteBuf(hspi, CHIP_Tx, nRF24_CMD_W_TX_PAYLOAD, pBuf,Length); // Write specified buffer to FIFO
  nRF24_RWReg(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_CONFIG,0x0E); // Config: CRC on (2 bytes), Power UP, RX/TX ctl = PTX
  CE_HIGH(CHIP_Tx); // CE pin high => Start transmit
  uint8_t i = 255;
  while(i){i--;};
  CE_LOW(CHIP_Tx);

  return 0x00;
}

void nRF24_RXPacket(SPI_HandleTypeDef* hspi,uint8_t* pBuf, uint8_t* Length) {
  uint8_t PayloadWidth = nRF24_ReadReg(hspi, CHIP_Rx, nRF24_CMD_R_RX_PL_WID);
  if (PayloadWidth > 32) {
    nRF24_SendCmd(hspi, CHIP_Rx, nRF24_CMD_FLUSH_RX);
    PayloadWidth = 0;
  } else {
    nRF24_ReadBuf(hspi, CHIP_Rx, nRF24_CMD_R_RX_PAYLOAD, pBuf, PayloadWidth);
  }
  *Length = PayloadWidth;
}

uint8_t nRF24_GetStatus(SPI_HandleTypeDef* hspi, uint8_t Chip) {
  
  uint8_t Status = 0x00;
  uint8_t Cmd = nRF24_CMD_NOP;
  CSN_LOW(Chip);
  HAL_SPI_TransmitReceive(hspi, &Cmd, &Status, 1, SPI_TIMEOUT);
  CSN_HIGH(Chip);
  return Status;
}

void nRF24_HandleStatus(SPI_HandleTypeDef* hspi, uint8_t Chip) {
  uint8_t Status = nRF24_GetStatus(hspi, Chip);
  uint8_t Flags = 0x00;

  if (Status & nRF24_MASK_MAX_RT) {
    //Transmit error. Clear package
//    printf("nRF24_MASK_MAX_RT\n");
    nRF24_SendCmd(hspi, Chip, nRF24_CMD_FLUSH_TX);
    Flags |= nRF24_MASK_MAX_RT;
    QueueResponse((char *)"DEVICE NOT RESPONDING\n");
  }
  
  if (Status & nRF24_MASK_RX_DR) {
    //Got a package, read it from buffer 
//    printf("nRF24_MASK_RX_DR\n");
    
    // TODO: !!!! DO IT IN WHILE LOOP UNTIL FIFO IS NOT EMPTY !!!!
    
    uint8_t pBuf[32];
    uint8_t Length;
    nRF24_RXPacket(hspi, (uint8_t *)&pBuf[0], &Length);

    printf("Length: %d, Data:", Length);
    if (Length > 0) {
      for (int i = 0; i < Length;i++) {
        printf("%x ", pBuf[i]);
      }
    }
    printf("\n");

    rfProcessCommand((uint8_t *)&pBuf[0], Length);
    
    Flags |= nRF24_MASK_RX_DR;
  }
  
  if (Status & nRF24_MASK_TX_DS) {
    //Package sent successfully. Just clear a flag
//    printf("nRF24_MASK_TX_DS\n");
    Flags |= nRF24_MASK_TX_DS;
  }
  
  if (Flags) {
    nRF24_RWReg(hspi, Chip, nRF24_CMD_WREG | nRF24_REG_STATUS, Status | Flags);
  } else {
//    printf("Status 221: %x\n", Status);
  }

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
