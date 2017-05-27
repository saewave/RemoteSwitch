#include "CC1101.h"
#include "F030f4_Peripheral.h"
#include "rf_cmd.h"
#include <string.h>

uint8_t CC1101_Data[64];
uint8_t CC1101_HUB_ADDR[CC1101_ADDR_LENGTH] = {0xAB, 0xCD};
uint8_t CC1101_RX_ADDR[CC1101_ADDR_LENGTH]  = {0xF0, 0xFF};

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
    CC1101_WriteStrobe(CCxxx0_SIDLE);
    CC1101_WriteStrobe(CCxxx0_SFRX);
    CC1101_WriteStrobe(CCxxx0_SRX);
}

void CC1101_Configure(void)
{
    CC1101_WriteReg(CCxxx0_IOCFG0,0x06); //IOCFG0 - GDO0 Output Pin Configuration
    CC1101_WriteReg(CCxxx0_SYNC0, CCxxx0_SYNC_WORD0); //CCxxx0_SYNC0
    CC1101_WriteReg(CCxxx0_SYNC1, CCxxx0_SYNC_WORD1); //CCxxx0_SYNC1
    CC1101_WriteReg(CCxxx0_FIFOTHR,0x47); //FIFOTHR - RX FIFO and TX FIFO Thresholds
    CC1101_WriteReg(CCxxx0_PKTLEN,0x3E); //PKTLEN - Packet Length
    CC1101_WriteReg(CCxxx0_PKTCTRL1, 0x08 + (CC1101_ADD_RSSI_LQI ? 0x04 : 0)); //PKTCTRL1 - Packet Automation Control
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
    SPI_SendData(&Reg, 1);
    SPI_SendData(&Value, 1);
    CSN_HIGH();
}

uint8_t CC1101_WriteStrobe(uint8_t Strobe) {
    uint8_t Res = 0;
    Delay_us(2);
    CSN_LOW();
    while((CC1101_MISO_PORT->IDR & CC1101_MISO_PIN) == CC1101_MISO_PIN) {__NOP();};
//    SPI_SendData(&Strobe, 1);
    SPI_ReadData(&Res, 1, Strobe);
    while((CC1101_MISO_PORT->IDR & CC1101_MISO_PIN) == CC1101_MISO_PIN) {__NOP();};
    CSN_HIGH();
    return Res;
}

void CC1101_WriteBurstReg(uint8_t Address, uint8_t *Buf, uint8_t Length) {
    Address |= WRITE_BURST;
    Delay_us(2);
    CSN_LOW();
    while((CC1101_MISO_PORT->IDR & CC1101_MISO_PIN) == CC1101_MISO_PIN) {__NOP();};
    SPI_SendData(&Address, 1);
    SPI_SendData(Buf, Length);
    CSN_HIGH();
}

uint8_t CC1101_ReadConfigReg(uint8_t Reg) {
    uint8_t Value = 0x00;
    Reg |= READ_SINGLE;
    Delay_us(2);
    CSN_LOW();
    while((CC1101_MISO_PORT->IDR & CC1101_MISO_PIN) == CC1101_MISO_PIN) {__NOP();};
    SPI_SendData(&Reg, 1);
    SPI_ReadData(&Value, 1, 0x00);
    CSN_HIGH();
    return Value;
}

uint8_t CC1101_ReadStatusReg(uint8_t Reg) {
    uint8_t _Reg = (Reg | READ_BURST), Value = 0x00;
    Delay_us(2);
    CSN_LOW();
    while((CC1101_MISO_PORT->IDR & CC1101_MISO_PIN) == CC1101_MISO_PIN) {__NOP();};
    SPI_SendData(&_Reg, 1);
    SPI_ReadData(&Value, 1, 0x00);
    CSN_HIGH();
    return Value;
}

uint8_t CC1101_ReadBurstReg(uint8_t Reg,  uint8_t *Buff, uint8_t Length) {
    uint8_t Value = 0x00;
    Reg |= READ_BURST;
    Delay_us(2);
    CSN_LOW();
    while((CC1101_MISO_PORT->IDR & CC1101_MISO_PIN) == CC1101_MISO_PIN) {__NOP();};
    SPI_SendData(&Reg, 1);
    SPI_ReadData(Buff, Length, 0x00);
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

void CC1101_TxData(uint8_t * Address, uint8_t * Buf, uint8_t Length) {
    uint8_t tBuf[Length + 1];
    tBuf[0] = Length;
    memcpy(&tBuf[1], Buf, Length);
    
#ifdef _DEBUG
        dxputs("DATA TO SEND: \n");
        for (uint8_t i = 0; i < Length+1; i++) {
            dxprintf("%x ", tBuf[i]);
        }
        dxputs("\n");
#endif
            
    CC1101_WriteStrobe(CCxxx0_SIDLE);
    CC1101_WriteStrobe(CCxxx0_SFTX);
    CC1101_WriteBurstReg(CCxxx0_TXFIFO, tBuf, Length + 1);
    CC1101_WriteStrobe(CCxxx0_STX);
}

void CC1101_HandleStatus(void) {
    dxputs("-------------------\n");
    SPI_FlushRxFifo();
    uint8_t Status = CC1101_WriteStrobe(CCxxx0_SNOP | 0x80);
    dxprintf("Status: %x\n", Status);
    if (Status & FIFO_BYTES_MASK) {
        uint8_t Length = CC1101_ReadStatusReg(CCxxx0_RXFIFO);
        dxprintf("Length: %d\n", Length);
//        Length = Length == 0 ? 10 : Length;
        if (Length & BYTES_IN_RXFIFO) {
            CC1101_ReadBurstReg(CCxxx0_RXFIFO, CC1101_Data, Length + (CC1101_ADD_RSSI_LQI ? 2 : 0));
#if configALL_TIME_RX_MODE == 0x01
            if ((Status & CHIP_STATE_MASK) != 0x01) {
                CC1101_RxMode();
                dxprintf("CC1101_RxMode 1\n");
            }
#endif
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
#endif
            rfProcessData(CC1101_Data, Length);
        }
    }
#if configALL_TIME_RX_MODE == 0x01
    if (((Status & CHIP_STATE_MASK) != 0x01) && ((Status & BYTES_IN_RXFIFO) == 0)) {
        CC1101_RxMode();
        dxprintf("CC1101_RxMode 2\n");
    }
#endif
}

uint8_t *CC1101_GetDeviceAddress(void)
{
    return CC1101_RX_ADDR;
}

void CC1101_SetHUBAddress(uint8_t *Address)
{
    memcpy(CC1101_HUB_ADDR, Address, CC1101_ADDR_LENGTH);
}

uint8_t *CC1101_GetHUBAddress(void)
{
    return CC1101_HUB_ADDR;
}

void CC1101_SetDeviceAddress(uint8_t *Address, uint8_t Override)
{
    memcpy(CC1101_RX_ADDR, Address, CC1101_ADDR_LENGTH);
}

void CC1101_SetDefaultDeviceAddress(void)
{
/*    memcpy(nRF24_Rx_Temp_Addr, nRF24_Rx_addr, 5);
    memcpy(CC1101_RX_ADDR, nRF24_Rx_Default_Addr, 5);*/
}
