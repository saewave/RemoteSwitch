#include "config.h"
#include "CC1101.h"

void readConfig(void)
{
    //  uint32_t fAddress = configFLASH_CFG_ADDR;
    uint8_t Config[10]     = {0x00};
    uint8_t isConfigStored = (*(__IO uint8_t *)(configFLASH_CFG_ADDR + 1));
    if (isConfigStored == 0xFF)
    {
        //    printf("Confing not set!\n");
        return;
    }
    dxprintf("Saved addr:\n");
    for (int i = 0; i < 10; i += 2)
    {
        Config[i]     = (*(__IO uint8_t *)(configFLASH_CFG_ADDR + i + 1));
        Config[i + 1] = (*(__IO uint8_t *)(configFLASH_CFG_ADDR + i));
        dxprintf("%x ", Config[i]);
        dxprintf("%x ", Config[i + 1]);
    }
    dxprintf("\n");
    Transceiver_SetDeviceAddress(&Config[0], 1);
    Transceiver_SetHUBAddress(&Config[5]);
}
