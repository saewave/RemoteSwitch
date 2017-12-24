#include "CC1101.h"

uint8_t CC1101_Data[CC1101_BUFFER_LENGTH];
volatile uint8_t CC1101_DataLength = 0;
uint8_t CC1101_HUB_ADDR[CC1101_ADDR_LENGTH] = {0xEC, 0xA2};
uint8_t CC1101_DEVICE_DISCOVER_ADDR[CC1101_ADDR_LENGTH] = {0xF0, 0xCF};
volatile  uint8_t CC1101_TransmiterState = CC1101_TR_STATE_IDLE;

void CC1101_Reset(void) {
    CSN_LOW();
    Delay_us(5);
    CSN_HIGH();
    Delay_us(40);
    CC1101_WriteStrobe(0x30);
    Delay_us(2);
}

void CC1101_TxMode(void) {
    CC1101_WriteStrobe(CCxxx0_STX);
    Delay_us(10);
    CC1101_WriteStrobe(CCxxx0_SNOP);
    Delay_us(10);
    CC1101_WriteStrobe(CCxxx0_SNOP);
}

void CC1101_RxMode(void) {
    CC1101_SetSyncWord(CC1101_HUB_ADDR[0], CC1101_HUB_ADDR[1]);
    CC1101_WriteStrobe(CCxxx0_SIDLE);
    CC1101_WriteStrobe(CCxxx0_SFRX);
    CC1101_WriteStrobe(CCxxx0_SRX);
}

void CC1101_Configure(void)
{
    CC1101_WriteReg(CCxxx0_IOCFG0,0x06); //IOCFG0 - GDO0 Output Pin Configuration
    CC1101_WriteReg(CCxxx0_FIFOTHR,0x47); //FIFOTHR - RX FIFO and TX FIFO Thresholds
    CC1101_WriteReg(CCxxx0_PKTLEN,0x3E); //PKTLEN - Packet Length
    CC1101_WriteReg(CCxxx0_PKTCTRL1,0x08 + (CC1101_ADD_RSSI_LQI ? 0x04 : 0)); //PKTCTRL1 - Packet Automation Control
    CC1101_WriteReg(CCxxx0_PKTCTRL0,0x05); //PKTCTRL0 - Packet Automation Control
    CC1101_WriteReg(CCxxx0_ADDR,0x23); //ADDR - Device Address
    CC1101_WriteReg(CCxxx0_CHANNR,0x01); //CHANNR - Channel Number
    CC1101_WriteReg(CCxxx0_FSCTRL1,0x06); //FSCTRL1 - Frequency Synthesizer Control
    CC1101_WriteReg(CCxxx0_FREQ2,0x10); //FREQ2 - Frequency Control Word, High Byte
    CC1101_WriteReg(CCxxx0_FREQ1,0xA7); //FREQ1 - Frequency Control Word, Middle Byte
    CC1101_WriteReg(CCxxx0_FREQ0,0x62); //FREQ0 - Frequency Control Word, Low Byte
    CC1101_WriteReg(CCxxx0_MDMCFG4,0xF9); //MDMCFG4 - Modem Configuration
    CC1101_WriteReg(CCxxx0_MDMCFG3,0x93); //MDMCFG3 - Modem Configuration
    CC1101_WriteReg(CCxxx0_MDMCFG2,0x43); //MDMCFG2 - Modem Configuration
    CC1101_WriteReg(CCxxx0_DEVIATN,0x15); //DEVIATN - Modem Deviation Setting
    CC1101_WriteReg(CCxxx0_MCSM0,0x18); //MCSM0 - Main Radio Control State Machine Configuration
    CC1101_WriteReg(CCxxx0_MCSM1,0x0F); //MCSM1 - Main Radio Control State Machine Configuration
    CC1101_WriteReg(CCxxx0_FOCCFG,0x16); //FOCCFG - Frequency Offset Compensation Configuration
    CC1101_WriteReg(CCxxx0_WORCTRL,0xFB); //WORCTRL - Wake On Radio Control
    CC1101_WriteReg(CCxxx0_FSCAL3,0xE9); //FSCAL3 - Frequency Synthesizer Calibration
    CC1101_WriteReg(CCxxx0_FSCAL2,0x2A); //FSCAL2 - Frequency Synthesizer Calibration
    CC1101_WriteReg(CCxxx0_FSCAL1,0x00); //FSCAL1 - Frequency Synthesizer Calibration
    CC1101_WriteReg(CCxxx0_FSCAL0,0x1F); //FSCAL0 - Frequency Synthesizer Calibration
    CC1101_WriteReg(CCxxx0_TEST2,0x81); //TEST2 - Various Test Settings
    CC1101_WriteReg(CCxxx0_TEST1,0x35); //TEST1 - Various Test Settings
    CC1101_WriteReg(CCxxx0_TEST0,0x09); //TEST0 - Various Test Settings
}

void CC1101_WriteReg(uint8_t Reg, uint8_t Value) {
    Delay_us(2);
    CSN_LOW();
    while((CC1101_MISO_PORT->IDR & CC1101_MISO_PIN) == CC1101_MISO_PIN) {__NOP();};
    SPI_SendData(SPI1, &Reg, 1);
    SPI_SendData(SPI1, &Value, 1);
    CSN_HIGH();
}

uint8_t CC1101_WriteStrobe(uint8_t Strobe) {
    uint8_t Res = 0;
    Delay_us(2);
    CSN_LOW();
    while((CC1101_MISO_PORT->IDR & CC1101_MISO_PIN) == CC1101_MISO_PIN) {__NOP();};
//    SPI_SendData(&Strobe, 1);
    SPI_ReadData(SPI1, &Res, 1, Strobe);
    while((CC1101_MISO_PORT->IDR & CC1101_MISO_PIN) == CC1101_MISO_PIN) {__NOP();};
    CSN_HIGH();
    return Res;
}

void CC1101_WriteBurstReg(uint8_t Address, uint8_t *Buf, uint8_t Length) {
    Address |= WRITE_BURST;
    Delay_us(2);
    CSN_LOW();
    while((CC1101_MISO_PORT->IDR & CC1101_MISO_PIN) == CC1101_MISO_PIN) {__NOP();};
    SPI_SendData(SPI1, &Address, 1);
    SPI_SendData(SPI1, Buf, Length);
    CSN_HIGH();
}

uint8_t CC1101_ReadConfigReg(uint8_t Reg) {
    uint8_t Value = 0x00;
    Reg |= 0x80;
    Delay_us(2);
    CSN_LOW();
    while((CC1101_MISO_PORT->IDR & CC1101_MISO_PIN) == CC1101_MISO_PIN) {__NOP();};
    SPI_SendData(SPI1, &Reg, 1);
    SPI_ReadData(SPI1, &Value, 1, 0x00);
    CSN_HIGH();
    return Value;
}

uint8_t CC1101_ReadStatusReg(uint8_t Reg) {
    uint8_t _Reg = (Reg | 0xC0), Value = 0x00;
    Delay_us(2);
    CSN_LOW();
    while((CC1101_MISO_PORT->IDR & CC1101_MISO_PIN) == CC1101_MISO_PIN) {__NOP();};
    SPI_SendData(SPI1, &_Reg, 1);
    SPI_ReadData(SPI1, &Value, 1, 0x00);
    CSN_HIGH();
    return Value;
}

uint8_t CC1101_ReadBurstReg(uint8_t Reg,  uint8_t *Buff, uint8_t Length) {
    uint8_t Value = 0x00;
    Reg |= READ_BURST;
    Delay_us(2);
    CSN_LOW();
    while((CC1101_MISO_PORT->IDR & CC1101_MISO_PIN) == CC1101_MISO_PIN) {__NOP();};
    SPI_SendData(SPI1, &Reg, 1);
    SPI_ReadData(SPI1, Buff, Length, 0x00);
    CSN_HIGH();
    return Value;
}

void CC1101_TxTestData(void) {
    uint8_t Buf[] = {0x05, 0x48, 0x65, 0x6C, 0x6C, 0x6F};
    CC1101_WriteStrobe(CCxxx0_SIDLE);
    CC1101_WriteStrobe(CCxxx0_SFTX);
    CC1101_WriteBurstReg(CCxxx0_TXFIFO, Buf, 6);
    CC1101_WriteStrobe(CCxxx0_STX);
}

void CC1101_TxData(uint16_t Address, uint8_t * Buf, uint8_t Length) {
    uint8_t tBuf[Length + 1];
    tBuf[0] = Length;
    if (Length > 0)
        memcpy(&tBuf[1], Buf, Length);

//    while(CC1101_TransmiterState != CC1101_TR_STATE_IDLE) { __NOP();};
    CC1101_TransmiterState = CC1101_TR_STATE_TX;
    CC1101_SetSyncWord((uint8_t)(Address>>8), (uint8_t)Address);
    CC1101_WriteStrobe(CCxxx0_SIDLE);
    CC1101_WriteStrobe(CCxxx0_SFTX);
    CC1101_WriteBurstReg(CCxxx0_TXFIFO, tBuf, Length + 1);
    CC1101_WriteStrobe(CCxxx0_STX);
    dxprintf("TX:\n");
    for (int i = 0; i < Length + 1; i++)
    {
        dxprintf("%x ", tBuf[i]);
    }
    dxprintf("\nTXS: %x\n", CC1101_TransmiterState);
}

void CC1101_SetSyncWord(uint8_t Word0, uint8_t Word1) {
    dxprintf("SetSyncWord: %02x %02x\n", Word0, Word1);
    CC1101_WriteReg(CCxxx0_SYNC0, Word0); //CCxxx0_SYNC0
    CC1101_WriteReg(CCxxx0_SYNC1, Word1); //CCxxx0_SYNC1
}

void CC1101_HandleStatus(void) {
    dxprintf("-------------------\n");
    uint8_t Status = CC1101_WriteStrobe(CCxxx0_SNOP | 0x80);
    dxprintf("Status: %x\n", Status);

    if (Status & FIFO_BYTES_MASK) {
        uint8_t Length = CC1101_ReadStatusReg(CCxxx0_RXFIFO);
        dxprintf("Length: %d\n", Length);
        if (Length & BYTES_IN_RXFIFO) {
            CC1101_DataLength = Length & BYTES_IN_RXFIFO;
            CC1101_ReadBurstReg(CCxxx0_RXFIFO, CC1101_Data, Length + (CC1101_ADD_RSSI_LQI ? 2 : 0));
//            CC1101_WriteStrobe(CCxxx0_SNOP | 0x80);
#ifdef _DEBUG
            dxputs("DATA: \n");
            for (uint8_t i = 0; i < Length; i++) {
                dxprintf("%x ", CC1101_Data[i]);
            }
            dxputs("\n");
#endif
#if CC1101_ADD_RSSI_LQI == 1
            uint8_t rssi, lqi; 
            unsigned char crssi;
            unsigned char clqi;
            rssi = CC1101_Data[Length + 2 - 2];
            lqi = CC1101_Data[Length + 2 - 1];
            if (rssi >= 128) {
                crssi = 255 - rssi;
                crssi /= 2;
                crssi += 74;
            } else {
                crssi = rssi/2;
                crssi += 74;
            }
            clqi = 0x3F - (lqi & 0x3F);
            dxprintf("crssi: %d, clqi: %d\n", crssi, clqi);
            UNUSED(clqi);
#endif
        }
    }
    if (((Status & CHIP_STATE_MASK) != 0x01)) {
        CC1101_RxMode();
    }
}
