#include "nRF24L01P.h"

void nRF24_Delay_ms(uint32_t Delay);
void nRF24_Delay_us(uint32_t Delay);
uint8_t nRF24_HUB_addr[nRF24_RX_ADDR_WIDTH] = {0xEE, 0x12, 0x13, 0x14, 0x00};
//uint8_t nRF24_ACK_RX_addr[nRF24_RX_ADDR_WIDTH] = {0x11, 0x12, 0x13, 0x14, 0x00};

uint8_t nRF24_NET_TX_START_addr[nRF24_RX_ADDR_WIDTH] = {0x11, 0x12, 0x13, 0x14, 0x01};

uint8_t nRF24_DEVICE_CFG_addr[nRF24_RX_ADDR_WIDTH] = {0xF0, 0xFF, 0x0F, 0x00, 0xCF};

uint8_t nRF24_ReadMask[32] = {0xff};
uint8_t Status = 0x00;

void nRF24_Configure(void)
{
    /*nRF24_RX_addr[0] = nRF24_TX_addr[0] = STM32_UUID[8];
  nRF24_RX_addr[1] = nRF24_TX_addr[1] = STM32_UUID[9];
  nRF24_RX_addr[2] = nRF24_TX_addr[2] = STM32_UUID[10];
  nRF24_RX_addr[3] = nRF24_TX_addr[3] = STM32_UUID[11];
  nRF24_RX_addr[4] = nRF24_TX_addr[4] = 0x00;
  */

    nRF24_SetCE(CHIP_Tx, GPIO_PIN_LOW);
    nRF24_SetCSN(CHIP_Tx, GPIO_PIN_HIGH);

    nRF24_SetCE(CHIP_Rx, GPIO_PIN_LOW);
    nRF24_SetCSN(CHIP_Rx, GPIO_PIN_HIGH);
}

void nRF24_SetCE(uint8_t Chip, uint8_t PinState) 
{
    if (Chip == CHIP_Tx) {
        T_CE_GPIO_Port->BSRR |= (T_CE_Pin << PinState);
    } else {
        R_CE_GPIO_Port->BSRR |= (R_CE_Pin << PinState);
    }
}

void nRF24_SetCSN(uint8_t Chip, uint8_t PinState) 
{
        if (Chip == CHIP_Tx) {
            T_CSN_GPIO_Port->BSRR |= (T_CSN_Pin << PinState);
        }
        else {
            R_CSN_GPIO_Port->BSRR |= (R_CSN_Pin << PinState);
        }
}

/**
 *  \brief Brief
 *  
 *  \param [in] ShortAddress Parameter_Description
 *  \param [in] FullAddress  Parameter_Description
 *  \return Return_Description
 *  
 *  \details Details
 */
void nRF24_GetDeviceFullAddress(uint16_t ShortAddress, uint8_t *FullAddress)
{
    memcpy(FullAddress, nRF24_NET_TX_START_addr, 5);
    FullAddress[3] = (uint8_t)(ShortAddress >> 8);
    FullAddress[4] = (uint8_t)(ShortAddress);
}
uint8_t reg, i = 0x00;
// Put nRF24L01 in RX mode
void nRF24_RXMode(SPI_TypeDef *hspi)
{
    nRF24_SetCE(CHIP_Rx, GPIO_PIN_LOW);
    reg = 0x00, i = 0x00;
    do
    {
        nRF24_RWReg(hspi, CHIP_Rx, nRF24_CMD_WREG | nRF24_REG_CONFIG, 0x0D); // Config: Power Off
        reg = nRF24_ReadReg(hspi, CHIP_Rx, nRF24_REG_CONFIG);
        i++;
    } while (reg != 0x0D && i < 0xFF);

    nRF24_WriteBuf(hspi, CHIP_Rx, nRF24_CMD_WREG | nRF24_REG_RX_ADDR_P0, nRF24_HUB_addr, nRF24_RX_ADDR_WIDTH); // Set static RX address
    nRF24_RWReg(hspi, CHIP_Rx, nRF24_CMD_WREG | nRF24_REG_FEATURE, 0x04);                                      // Allow dynamic payload length
    nRF24_RWReg(hspi, CHIP_Rx, nRF24_CMD_WREG | nRF24_REG_DYNPD, 0x1F);                                        // Enable dynamic payload length for 0-5 channels
    nRF24_RWReg(hspi, CHIP_Rx, nRF24_CMD_WREG | nRF24_REG_EN_AA, 0x1F);                                        // Enable acknowledgement for all data pipes
    nRF24_RWReg(hspi, CHIP_Rx, nRF24_CMD_WREG | nRF24_REG_EN_RXADDR, 0x1F);                                    // Enable data pipe 0-1
    nRF24_RWReg(hspi, CHIP_Rx, nRF24_CMD_WREG | nRF24_REG_RF_CH, 0x7C);                                        // Set frequency channel 110 (2.510MHz)
    nRF24_RWReg(hspi, CHIP_Rx, nRF24_CMD_WREG | nRF24_REG_RF_SETUP, 0x27);                                     // Setup: 250Kbps, 0dBm, LNA off
    nRF24_RWReg(hspi, CHIP_Rx, nRF24_CMD_WREG | nRF24_REG_CONFIG, 0x0F);                                       // Config: CRC on (2 bytes), Power UP, RX/TX ctl = PRX
//    reg = nRF24_ReadReg(hspi, CHIP_Rx, nRF24_REG_CONFIG);
    nRF24_Delay_ms(50);
//    CE_HIGH(CHIP_Rx);
    nRF24_SetCE(CHIP_Rx, GPIO_PIN_HIGH);
//    nRF24_Delay_ms(100);
//    reg = nRF24_ReadReg(hspi, CHIP_Rx, nRF24_REG_CONFIG);
}

// Put nRF24L01 in TX mode
void nRF24_TXMode(SPI_TypeDef *hspi)
{
    nRF24_SetCE(CHIP_Tx, GPIO_PIN_LOW);
    
    do
    {
        nRF24_RWReg(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_CONFIG, 0x0C); // Config: Power Off
        reg = nRF24_ReadReg(hspi, CHIP_Tx, nRF24_REG_CONFIG);
        i++;
    } while ((reg & nRF24_MASK_PWR_UP) && i < 0xFF);

    nRF24_RWReg(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_FEATURE, 0x04);    // Allow dynamic payload length
    nRF24_RWReg(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_DYNPD, 0x1F);      // Enable dynamic payload length for 0-5 channels
    nRF24_RWReg(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_EN_AA, 0x1F);      // Enable auto acknowledgement for data pipe 1
    nRF24_RWReg(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_SETUP_RETR, 0x1A); // Auto retransmit: wait 500us, 10 retries
    nRF24_RWReg(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_RF_CH, 0x7C);      // Set frequency channel 110 (2.510MHz)
    nRF24_RWReg(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_RF_SETUP, 0x27);   // Setup: 250Kbps, +7dBm
    nRF24_RWReg(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_CONFIG, 0x0E);     // Config: CRC on (2 bytes), Power UP, RX/TX ctl = PTX
    reg = nRF24_ReadReg(hspi, CHIP_Rx, nRF24_REG_CONFIG);
    nRF24_Delay_ms(5);
//    CE_HIGH(CHIP_Tx);
    nRF24_SetCE(CHIP_Tx, GPIO_PIN_HIGH);
}

void nRF24_DumpRegisters(SPI_TypeDef *hspi, uint8_t Chip)
{
    uint8_t RegVal   = 0x00;
    uint8_t rDump[6] = {nRF24_REG_STATUS, nRF24_REG_EN_AA, nRF24_REG_SETUP_RETR, nRF24_REG_RF_CH, nRF24_REG_RF_SETUP, nRF24_REG_CONFIG};
    for (int reg = 0; reg < 6; reg++)
    {
        RegVal = nRF24_ReadReg(hspi, Chip, nRF24_CMD_RREG | rDump[reg]);
    }
    UNUSED(RegVal);
}


uint8_t nRF24_ReadReg(SPI_TypeDef *hspi, uint8_t Chip, uint8_t Reg)
{
    uint8_t           pRxData[1] = {0x00};
    nRF24_SetCSN(Chip, GPIO_PIN_LOW);
    nRF24_Delay_us(5);
//    HAL_SPI_Transmit(hspi, &Reg, 1, 10);
    (void)hspi->DR;
    SPI_SendData(hspi, &Reg, 1);
    
//    HAL_SPI_TransmitReceive(hspi, (uint8_t *)&nRF24_ReadMask, &pRxData, 1, SPI_TIMEOUT);
    SPI_ReadData(hspi, pRxData, 1, 0xff);
    //CSN_HIGH(Chip);
    nRF24_SetCSN(Chip, GPIO_PIN_HIGH);
    nRF24_Delay_us(5);
    return pRxData[0];
}

uint8_t nRF24_SendCmd(SPI_TypeDef *hspi, uint8_t Chip, uint8_t Cmd)
{
    uint8_t Status = 0x00;
//    CSN_LOW(Chip);
    nRF24_SetCSN(Chip, GPIO_PIN_LOW);
    nRF24_Delay_us(5);
//    HAL_SPI_TransmitReceive(hspi, &Cmd, &Status, 1, SPI_TIMEOUT);
    SPI_ReadData(hspi, &Status, 1, Cmd);
//    CSN_HIGH(Chip);
    nRF24_SetCSN(Chip, GPIO_PIN_HIGH);
    nRF24_Delay_ms(1);
    return Status;
}

void nRF24_RWReg(SPI_TypeDef *hspi, uint8_t Chip, uint8_t Reg, uint8_t Data)
{

    uint8_t           pBuf[2] = {Reg, Data};
//    CSN_LOW(Chip);
    nRF24_SetCSN(Chip, GPIO_PIN_LOW);
    nRF24_Delay_us(5);
//    status = HAL_SPI_Transmit(hspi, (uint8_t *)pBuf, 2, SPI_TIMEOUT);
    SPI_SendData(hspi, pBuf, 2);
//    CSN_HIGH(Chip);
    nRF24_SetCSN(Chip, GPIO_PIN_HIGH);
    nRF24_Delay_us(5);
}

void nRF24_WriteBuf(SPI_TypeDef *hspi, uint8_t Chip, uint8_t Reg, uint8_t *Data, uint8_t size)
{

//    CSN_LOW(Chip);
    nRF24_SetCSN(Chip, GPIO_PIN_LOW);
    nRF24_Delay_us(5);
//    status = HAL_SPI_Transmit(hspi, (uint8_t *)&Reg, 1, SPI_TIMEOUT);
    SPI_SendData(hspi, &Reg, 1);
//    status = HAL_SPI_Transmit(hspi, Data, size, SPI_TIMEOUT);
    SPI_SendData(hspi, Data, size);
//    CSN_HIGH(Chip);
    nRF24_SetCSN(Chip, GPIO_PIN_HIGH);
    nRF24_Delay_us(5);
}

void nRF24_ReadBuf(SPI_TypeDef *hspi, uint8_t Chip, uint8_t Reg, uint8_t *Data, uint8_t count)
{
//    CSN_LOW(Chip);
    nRF24_SetCSN(Chip, GPIO_PIN_LOW);
    nRF24_Delay_us(5);
//    HAL_SPI_Transmit(hspi, (uint8_t *)&Reg, 1, SPI_TIMEOUT);
    SPI_SendData(hspi, &Reg, 1);
//    HAL_SPI_Receive(hspi, Data, count, SPI_TIMEOUT);
    SPI_ReadData(hspi, Data, count, 0xff);
//    CSN_HIGH(Chip);
    nRF24_SetCSN(Chip, GPIO_PIN_HIGH);
    nRF24_Delay_us(5);
}

uint8_t nRF24_TXPacket(SPI_TypeDef *hspi, uint8_t *nRF24_TX_addr, uint8_t *pBuf, uint8_t Length)
{

    nRF24_HandleStatus(hspi, CHIP_Tx);

//    CE_LOW(CHIP_Tx);
    nRF24_SetCE(CHIP_Tx, GPIO_PIN_LOW);
    nRF24_WriteBuf(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_TX_ADDR, nRF24_TX_addr, nRF24_TX_ADDR_WIDTH);    // Set static TX address
    nRF24_WriteBuf(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_RX_ADDR_P0, nRF24_TX_addr, nRF24_TX_ADDR_WIDTH); // Set static RX address for auto ack
    nRF24_WriteBuf(hspi, CHIP_Tx, nRF24_CMD_W_TX_PAYLOAD, pBuf, Length);                                      // Write specified buffer to FIFO
    nRF24_RWReg(hspi, CHIP_Tx, nRF24_CMD_WREG | nRF24_REG_CONFIG, 0x0E);                                      // Config: CRC on (2 bytes), Power UP, RX/TX ctl = PTX
    nRF24_Delay_ms(5);
//    CE_HIGH(CHIP_Tx); // CE pin high => Start transmit
    nRF24_SetCE(CHIP_Tx, GPIO_PIN_HIGH);
    nRF24_Delay_ms(10);
//    CE_LOW(CHIP_Tx);
    nRF24_SetCE(CHIP_Tx, GPIO_PIN_LOW);

    return 0x00;
}

void nRF24_RXPacket(SPI_TypeDef *hspi, uint8_t *pBuf, uint8_t *Length)
{
    uint8_t PayloadWidth = nRF24_ReadReg(hspi, CHIP_Rx, nRF24_CMD_R_RX_PL_WID);
    if (PayloadWidth > 32)
    {
        nRF24_SendCmd(hspi, CHIP_Rx, nRF24_CMD_FLUSH_RX);
        PayloadWidth = 0;
    }
    else
    {
        nRF24_ReadBuf(hspi, CHIP_Rx, nRF24_CMD_R_RX_PAYLOAD, pBuf, PayloadWidth);
    }
    *Length = PayloadWidth;
}

uint8_t nRF24_GetStatus(SPI_TypeDef *hspi, uint8_t Chip)
{

    uint8_t Status = 0x00;
//    CSN_LOW(Chip);
    nRF24_SetCSN(Chip, GPIO_PIN_LOW);
    nRF24_Delay_us(5);
//    HAL_SPI_TransmitReceive(hspi, &Cmd, &Status, 1, SPI_TIMEOUT);
    SPI_ReadData(hspi, &Status, 1, nRF24_CMD_NOP);
//    CSN_HIGH(Chip);
    nRF24_SetCSN(Chip, GPIO_PIN_HIGH);
    return Status;
}

void nRF24_HandleStatus(SPI_TypeDef *hspi, uint8_t Chip)
{
    uint8_t Timeout = 0xff;
    uint8_t Flags  = 0x00;
    Status = 0x00;

    do
    {
        nRF24_Delay_us(5);
        Status = nRF24_GetStatus(hspi, Chip);
        if (Status & nRF24_MASK_MAX_RT)
        {
            //Transmit error. Clear package
            dxprintf("nRF24_MASK_MAX_RT\n");
            nRF24_SendCmd(hspi, Chip, nRF24_CMD_FLUSH_TX);
            Flags |= nRF24_MASK_MAX_RT;
            QueueResponse((char *)"DEVICE NOT RESPONDING\n", OUSART2);
        }

        if ((Status & nRF24_MASK_RX_DR) || ((Status & 0x0E) != 0x0E))
        {
            //Got a package, read it from buffer
            dxprintf("nRF24_MASK_RX_DR\n");

            uint8_t Length = 0;
            uint8_t pBuf[32];
//            do
//            {
                nRF24_RXPacket(hspi, pBuf, &Length);
                rfProcessCommand(pBuf, Length);
//            } while (Length > 0);
            Flags |= nRF24_MASK_RX_DR;
        }

        if (Status & nRF24_MASK_TX_DS)
        {
            //Package sent successfully. Just clear a flag
            dxprintf("nRF24_MASK_TX_DS\n");
            Flags |= nRF24_MASK_TX_DS;
        }

        if (!(Status & 0x0E))
        {
            nRF24_RWReg(hspi, Chip, nRF24_CMD_WREG | nRF24_REG_STATUS, Status);
        }
        Timeout--;
    } while (!(Status & 0x0E));
}

void nRF24_Delay_ms(uint32_t Delay) {
    uint32_t Wait = SAE_CPU_FREQUENCY/1000*Delay;
    uint32_t i = 0;
    for(i=0;i<Wait;i++) {
        __NOP();
    }
}

void nRF24_Delay_us(uint32_t Delay) {
    uint32_t Wait = SAE_CPU_FREQUENCY/1000000*Delay;
    uint32_t i = 0;
    for(i=0;i<Wait;i++) {
        __NOP();
    }
}
