#include "rf_device.h"
#include "FreeRTOS.h"
#include "at_processor.h"
#include "config.h"
#include "nRF24L01P.h"
#include "task.h"
#include "xdebug.h"

dLink rfDevices = NULL;

uint16_t rfDeviceAddrCounter = 0x0001;

dLink rfCreateDevice(void)
{
    dxprintf("rfCreateDevice\n");
    dLink newDevice = (dLink)pvPortMalloc(sizeof(xDevice));
    if (newDevice == NULL)
    {
        dxprintf("Error: can't allocate memory!\n");
        return NULL;
    }
    dxprintf("New device on: %p!\n", newDevice);
    newDevice->Next   = NULL;
    newDevice->Prev   = NULL;
    newDevice->Type   = 0x00;
    newDevice->Config = 0x00;
    newDevice->Salt   = rfGetSalt();

    if (rfDevices == NULL)
    { // If list is empty
        rfDevices          = newDevice;
        rfDevices->Address = rfDeviceAddrCounter;
    }
    else
    { // If list is not empty, find last elem, and add to it.
        dLink Cur           = rfDevices;
        rfDeviceAddrCounter = Cur->Address; // TODO: Change the code below to get find Address more accurate
        while (Cur->Next != NULL)
        {
            Cur = Cur->Next;
            if (rfDeviceAddrCounter < Cur->Address)
            {
                rfDeviceAddrCounter = Cur->Address;
            }
        };
        newDevice->Address = ++rfDeviceAddrCounter;
        newDevice->Prev    = Cur;
        Cur->Next          = newDevice;
    }
    dxprintf("rf: %p, ND: %p\n", rfDevices, newDevice);
    return newDevice;
}

dLink rfRegisterDevice(uint16_t Address, uint8_t Type)
{
    dxprintf("rfRegisterDevice\n");
    dLink newDevice = (dLink)pvPortMalloc(sizeof(xDevice));
    if (newDevice == NULL)
    {
        dxprintf("Error: can't allocate memory!\n");
        return NULL;
    }
    dxprintf("New device on: %p!\n", newDevice);
    newDevice->Next   = NULL;
    newDevice->Prev   = NULL;
    newDevice->Type   = Type;
    newDevice->Config = 0x00;
    newDevice->Salt   = rfGetSalt();

    if (Address == 0)
    {
        Address = rfDeviceAddrCounter;
    }
    if (rfDevices == NULL)
    { // If list is empty
        rfDevices          = newDevice;
        rfDevices->Address = Address;
    }
    else
    { // If list is not empty, find last elem, and add to it.
        dLink Cur           = rfDevices;
        rfDeviceAddrCounter = Cur->Address; // TODO: Change the code below to get find Address more accurate
        while (Cur->Next != NULL)
        {
            Cur = Cur->Next;
            if (rfDeviceAddrCounter < Cur->Address)
            {
                rfDeviceAddrCounter = Cur->Address;
            }
        };
        if (Address == 0)
        {
            Address = ++rfDeviceAddrCounter;
        }
        newDevice->Address = Address;
        newDevice->Prev    = Cur;
        Cur->Next          = newDevice;
    }
    dxprintf("rf: %p, ND: %p\n", rfDevices, newDevice);
    return newDevice;
}

void rfListDevices(void)
{
    dLink   Cur = rfDevices;
    uint8_t cnt = 1;
    char    Buf[100];
    while (Cur != NULL)
    {
        sprintf(Buf, "%d, Device: Adr: %04x Type: %02x, Conf: %x, Sa: %x, mA: %p, mP: %p, mN: %p\n", cnt, Cur->Address, Cur->Type, Cur->Config, Cur->Salt, Cur, Cur->Prev, Cur->Next);
        QueueResponse(Buf, OUSART2);
        Cur = Cur->Next;
        cnt++;
    };
    sprintf(Buf, "Found %d device(s)\n", cnt - 1);
    QueueResponse(Buf, OUSART2);
}

dLink rfUpdateDevice(uint16_t Address, uint8_t Type, uint8_t Config)
{
    dLink   Cur       = rfDevices;
    uint8_t isUpdated = 0;
    while (Cur != NULL)
    {
        //    dxprintf("fd A: %x, P: %p, N: %p\n", Cur->Address, Cur->Prev, Cur->Next);
        if (Cur->Address == Address)
        {
            if (Type != NULL)
            {
                Cur->Type = Type;
            }
            Cur->Config = Config;
            isUpdated   = 1;
            break;
        }
        Cur = Cur->Next;
    }
    if (isUpdated)
        return Cur;
    else
        return NULL;
}

dLink rfGetDevice(uint16_t Address)
{
    dLink Cur = rfDevices;
    while (Cur != NULL)
    {
        //    dxprintf("fd A: %x, P: %p, N: %p\n", Cur->Address, Cur->Prev, Cur->Next);
        if (Cur->Address == Address)
        {
            return Cur;
        }
        Cur = Cur->Next;
    }
    return NULL;
}

uint8_t rfRemoveDevices(uint16_t Address)
{
    dLink   Cur       = rfDevices;
    uint8_t isRemoved = 0;
    while (Cur != NULL)
    {
        //    dxprintf("fd A: %x, P: %p, N: %p\n", Cur->Address, Cur->Prev, Cur->Next);
        if (Cur->Address == Address)
        {
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

void rfSaveDevices(void)
{
    //0x4002 3C00 - 0x4002 3FFF
    // Unlock flash to erase and write

    if (rfDevices == NULL)
    {
        QueueResponse((char *)"Nothing to store\n", OUSART1);
        return;
    }
    /**  Flash pages */
    taskENTER_CRITICAL();
    FLASH->KEYR = FLASH_FKEY1;
    FLASH->KEYR = FLASH_FKEY2;

    while (FLASH->SR & FLASH_SR_BSY)
    {
        __NOP();
    }; // Wait untill memory ready

    FLASH->CR |= FLASH_CR_PER;              // Erase one page
    FLASH->AR |= SAE_DEVICES_MEMORY_ADDRESS; // Erase address
    FLASH->CR |= FLASH_CR_STRT;

    while (FLASH->SR & FLASH_SR_BSY)
    {
        __NOP();
    }; // Wait untill memory ready
    FLASH->CR &= ~FLASH_CR_PER;
    while (FLASH->SR & FLASH_SR_BSY)
    {
        __NOP();
    };
    FLASH->CR |= FLASH_CR_PG; // Allow flash programming
    while (FLASH->SR & FLASH_SR_BSY)
    {
        __NOP();
    };

    dLink Cur = rfDevices;

    uint32_t Address = SAE_DEVICES_MEMORY_ADDRESS;
    while (Cur != NULL)
    {
        if (Cur->Type == NULL)
        {
            Cur = Cur->Next;
            continue;
        }
        *(__IO uint16_t *)Address = (uint16_t)Cur->Address;
        while (FLASH->SR & FLASH_SR_BSY)
        {
            __NOP();
        };
        Address += 2;
        *(__IO uint16_t *)Address = (uint16_t)((uint16_t)Cur->Type | ((uint16_t)Cur->Config << 8));
        while (FLASH->SR & FLASH_SR_BSY)
        {
            __NOP();
        };
        Address += 2;
        *(__IO uint16_t *)Address = (uint16_t)Cur->Salt;
        while (FLASH->SR & FLASH_SR_BSY)
        {
            __NOP();
        };
        Address += 2;
        *(__IO uint16_t *)Address = (uint16_t)(Cur->Salt >> 16);
        while (FLASH->SR & FLASH_SR_BSY)
        {
            __NOP();
        };
        Address += 2;
        Cur = Cur->Next;
    };

    while (FLASH->SR & FLASH_SR_BSY)
    {
        __NOP();
    };
    FLASH->CR &= ~(FLASH_CR_PG);
    FLASH->CR |= FLASH_CR_LOCK;
    taskEXIT_CRITICAL();
}

void rfLoadDevices(void)
{
    uint32_t Address       = SAE_DEVICES_MEMORY_ADDRESS;
    uint16_t DeviceAddress = *(__IO uint16_t *)Address;
    //  uint16_t DeviceConfAndType = 0x00;
    dLink Prev = NULL, Next = NULL;
    dLink newDevice = NULL;
    while (DeviceAddress != 0xFFFF)
    {
        Prev      = newDevice;
        newDevice = (dLink)pvPortMalloc(sizeof(xDevice));
        if (Prev != NULL)
        {
            Prev->Next = newDevice;
        }
        else
        {
            rfDevices = newDevice;
        }
        newDevice->Prev    = Prev;
        newDevice->Next    = Next;
        newDevice->Address = DeviceAddress;
        //    DeviceConfAndType = *(__IO uint16_t*)(Address+2);
        newDevice->Type   = *(__IO uint8_t *)(Address + 2);
        newDevice->Config = *(__IO uint8_t *)(Address + 3);
        newDevice->Salt   = *(__IO uint32_t *)(Address + 4);
        Address += 8;
        DeviceAddress = *(__IO uint16_t *)Address;
    }
}

void rfPingDevice(uint16_t Address)
{
    dLink Device = rfGetDevice(Address);
    if (Device == NULL)
    {
        QueueResponse((char *)"Device not registered!", OUSART1);
        return;
    }

    uint8_t Data[2] = {0x00, 0x01};
    rfSendCommad(rfCMD_PING, Address, Data, 2, Device->Salt);
}

void rfPingAllDevices(void)
{

    dLink Cur = rfDevices;
    if (Cur == NULL)
    {
        QueueResponse((char *)"Devices not found!\n", OUSART1);
        return;
    }
    uint8_t Data[2] = {0x00, 0x01};
    while (Cur != NULL)
    {
        //    dxprintf("fd A: %x, P: %p, N: %p\n", Cur->Address, Cur->Prev, Cur->Next);
        rfSendCommad(rfCMD_PING, Cur->Address, Data, 2, Cur->Salt);
//        osDelay(20);
        Cur = Cur->Next;
    }
}

void rfSendData(uint8_t Cmd, dLink Device, char *Parameters)
{
    switch (Device->Type)
    {
    case rfDEVICE_TYPE_1:
    case rfDEVICE_TYPE_2:
    {
        int NewValue[21] = {0x00};
        int readVals     = sscanf(Parameters, rfCMD_DATA_MASK,
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
        if (readVals > 0)
        {
            uint8_t Data[21] = {0x00};
            for (int i = 0; i < readVals; i++)
            {
                Data[i] = (uint8_t)NewValue[i];
                //          dxprintf("NewValue: %03x\n", Data[i]);
            }
            //        dxprintf("** Cmd: %x\n", Cmd);
            rfSendCommad(Cmd, Device->Address, Data, readVals, Device->Salt);
        }
        else
        {
            QueueResponse((char *)"Error: Device address not set!\n\n", OUSART1);
        }
    }
    break;
    default:
    {
        char Buf[50];
        sprintf(Buf, "Error: Device type %d not supported yet!\n\n", Device->Type);
        QueueResponse(&Buf[0], OUSART1);
    }
    break;
    }
}

void rfSendCommad(uint8_t Command, uint16_t Address, uint8_t *Data, uint8_t Length, uint32_t Salt)
{
    if (Length > 21)
    {
        dxprintf("Data can't be length than 20 bytes");
    }
    uint8_t DeviceAddress[5] = {0x00};
    nRF24_GetDeviceFullAddress(Address, DeviceAddress);

    uint8_t *pBuf = (uint8_t *)pvPortMalloc(Length + 7 + 4);
    pBuf[0]       = 0x00; //Header, default 0x00
    memcpy(&pBuf[1], nRF24_HUB_addr, 5);
    pBuf[6] = Command; //Cmd

    memcpy(&pBuf[7], Data, Length);

    uint32_t crcRes                    = rfCalcCRC32(&pBuf[0], Length + 7);
    *((uint32_t *)(pBuf + 7 + Length)) = crcRes;

    dxprintf("TX:\n\n");
    for (int i = 0; i < Length + 7 + 4; i++)
    {
        dxprintf("%x ", pBuf[i]);
    }
    dxprintf("\n\n");
    nRF24_TXPacket(SPI2, DeviceAddress, pBuf, Length + 7);
    vPortFree(pBuf);
}

void rfPrepareTestDevices(void)
{
    const int Length            = 2;
    uint16_t  Addresses[Length] = {0x0001, 0x0002};
    uint8_t   Types[Length]     = {0x01, 0x02};

    for (int i = 0; i < Length; i++)
    {
        dLink newDevice = rfCreateDevice();
        rfUpdateDevice(Addresses[i], Types[i], 0x00);
    }

    rfListDevices();
}

uint32_t rfCalcCRC32(uint8_t *Data, uint8_t Length)
{
    uint32_t cnt;
    CRC->CR = CRC_CR_RESET;
    /* Calculate number of 32-bit blocks */
    cnt = Length >> 2;
    /* Calculate */
    while (cnt--)
    {
        /* Set new value */
        CRC->DR = *(uint32_t *)Data;

        /* Increase by 4 */
        Data += 4;
    }
    /* Calculate remaining data as 8-bit */
    cnt = Length % 4;
    while (cnt--)
    {
        *((uint8_t *)&CRC->DR) = *Data++;
    }

    return CRC->DR;
}

uint32_t rfGetSalt(void)
{
    return 0xAABBCCDD; //TODO: Should be replaced to random 32bit number!
}
