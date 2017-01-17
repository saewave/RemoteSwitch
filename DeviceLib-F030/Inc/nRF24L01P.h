#include "stm32f0xx.h"
#include "F030f4_Peripheral.h"

#ifndef __nRF24L01_H
#define __nRF24L01_H

// nRF24L01 CE (Transmit) pin
#define nRF24_CE_PORT     CE_GPIO_Port
#define nRF24_CE_PIN      CE_Pin

#define CE_LOW()      P_CE_LOW()
#define CE_HIGH()     P_CE_HIGH()
   
// nRF24L01 CSN (Chip Enable) pin
#define nRF24_CSN_PORT     CSN_GPIO_Port
#define nRF24_CSN_PIN      CSN_Pin

#define CSN_LOW()      P_CSN_LOW()
#define CSN_HIGH()     P_CSN_HIGH()

/* nRF24L0 commands */
#define nRF24_CMD_RREG             0x00  // R_REGISTER -> Read command and status registers
#define nRF24_CMD_WREG             0x20  // W_REGISTER -> Write command and status registers
#define nRF24_CMD_R_RX_PAYLOAD     0x61  // R_RX_PAYLOAD -> Read RX payload
#define nRF24_CMD_W_TX_PAYLOAD     0xA0  // W_TX_PAYLOAD -> Write TX payload
#define nRF24_CMD_FLUSH_TX         0xE1  // FLUSH_TX -> Flush TX FIFO
#define nRF24_CMD_FLUSH_RX         0xE2  // FLUSH_RX -> Flush RX FIFO
#define nRF24_CMD_REUSE_TX_PL      0xE3  // REUSE_TX_PL -> Reuse last transmitted payload
#define nRF24_CMD_R_RX_PL_WID      0x60  // Read RX payload width for the top R_RX_PAYLOAD in the RX FIFO.
#define nRF24_CMD_NOP              0xFF  // No operation (to read status register)

/* nRF24L0 registers */
#define nRF24_REG_CONFIG           0x00  // Configuration register
#define nRF24_REG_EN_AA            0x01  // Enable "Auto acknowledgment"
#define nRF24_REG_EN_RXADDR        0x02  // Enable RX addresses
#define nRF24_REG_SETUP_AW         0x03  // Setup of address widths
#define nRF24_REG_SETUP_RETR       0x04  // Setup of automatic retranslation
#define nRF24_REG_RF_CH            0x05  // RF channel
#define nRF24_REG_RF_SETUP         0x06  // RF setup register
#define nRF24_REG_STATUS           0x07  // Status register
#define nRF24_REG_OBSERVE_TX       0x08  // Transmit observe register
#define nRF24_REG_CD               0x09  // Carrier detect
#define nRF24_REG_RX_ADDR_P0       0x0A  // Receive address data pipe 0
#define nRF24_REG_RX_ADDR_P1       0x0B  // Receive address data pipe 1
#define nRF24_REG_RX_ADDR_P2       0x0C  // Receive address data pipe 2
#define nRF24_REG_RX_ADDR_P3       0x0D  // Receive address data pipe 3
#define nRF24_REG_RX_ADDR_P4       0x0E  // Receive address data pipe 4
#define nRF24_REG_RX_ADDR_P5       0x0F  // Receive address data pipe 5
#define nRF24_REG_TX_ADDR          0x10  // Transmit address
#define nRF24_REG_RX_PW_P0         0x11  // Number of bytes in RX payload id data pipe 0
#define nRF24_REG_RX_PW_P1         0x12  // Number of bytes in RX payload id data pipe 1
#define nRF24_REG_RX_PW_P2         0x13  // Number of bytes in RX payload id data pipe 2
#define nRF24_REG_RX_PW_P3         0x14  // Number of bytes in RX payload id data pipe 3
#define nRF24_REG_RX_PW_P4         0x15  // Number of bytes in RX payload id data pipe 4
#define nRF24_REG_RX_PW_P5         0x16  // Number of bytes in RX payload id data pipe 5
#define nRF24_REG_FIFO_STATUS      0x17  // FIFO status register
#define nRF24_REG_DYNPD            0x1C  // Enable dynamic payload length
#define nRF24_REG_FEATURE          0x1D  // Feature register

/* nRF24L0 bits */
#define nRF24_MASK_RX_DR           0x40  // Mask interrupt caused by RX_DR
#define nRF24_MASK_TX_DS           0x20  // Mask interrupt caused by TX_DS
#define nRF24_MASK_MAX_RT          0x10  // Mask interrupt caused by MAX_RT
#define nRF24_FIFO_RX_EMPTY        0x01  // RX FIFO empty flag
#define nRF24_FIFO_RX_FULL         0x02  // RX FIFO full flag

/* Some constants */
#define nRF24_RX_ADDR_WIDTH        5    // nRF24 RX address width
#define nRF24_TX_ADDR_WIDTH        5    // nRF24 TX address width
#define SPI_TIMEOUT                1000

#define nRF24_SWITCH_TO_NONE 0x00
#define nRF24_SWITCH_TO_TX 0x01
#define nRF24_SWITCH_TO_RX 0x02

/* Variables */
/*uint8_t nRF24_RX_addr[nRF24_RX_ADDR_WIDTH];
uint8_t nRF24_TX_addr[nRF24_TX_ADDR_WIDTH];*/
//uint8_t nRF24_SwitchTo;

/* Function prototypes */
//void nRF24_init();

void nRF24_SetDeviceAddress(uint8_t *Address, uint8_t Override);
void nRF24_SetDefaultDeviceAddress(void);
uint8_t* nRF24_GetDeviceAddress(void);
void nRF24_SetHUBAddress(uint8_t *Address);
uint8_t* nRF24_GetHUBAddress(void);
void nRF24_SetSwitchTo(uint8_t SwitchTo);
//void nRF24_SetSPIHandler(SPI_HandleTypeDef* hspi);
void nRF24_RXMode(void);
void nRF24_TXMode(void);
uint8_t nRF24_TXPacket(uint8_t *nRF24_Tx_addr, uint8_t *pBuf, uint8_t Length);
void nRF24_RXPacket(uint8_t *pBuf, uint8_t* Length);
void nRF24_RWReg(uint8_t Reg, uint8_t Data);
uint8_t nRF24_ReadReg(uint8_t Reg);
void nRF24_WriteBuf(uint8_t Reg, uint8_t *Data, uint8_t size);
void nRF24_ReadBuf(uint8_t Reg, uint8_t *Data, uint8_t count);
uint8_t nRF24_DataReady(void);
void nRF24_DumpRegisters(void);
uint8_t nRF24_SendCmd(uint8_t Cmd);
uint8_t nRF24_GetStatus(void);
uint8_t nRF24_HandleStatus(void);
void nRF24_PowerOff(void);
void nRF24_PowerOnTX(void);
void nRF24_PowerOnRX(void);
    
static void SPIx_Error (void);

static void SPIx_Write(uint8_t Value);
static uint32_t SPIx_Read(void);

#endif /* __nRF24L01_H */
