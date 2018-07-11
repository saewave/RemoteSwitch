/* 
 * This file is part of the RemoteSwitch distribution 
 * (https://github.com/saewave/RemoteSwitch).
 * Copyright (c) 2018 Samoilov Alexey.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "core.h"
#include "F030C8_Peripheral.h"
#include "SI446x.h"
#include "rf_cmd.h"

uint16_t State              = STATE_IDLE;
uint8_t  WateringInProgress = 0;

uint16_t ADCZBuf[ADC_CH_CNT][ADC_CH_LEN];
uint16_t ADCVBuf[ADC_CH_CNT][ADC_CH_LEN];

uint16_t ADCZVal[ADC_CH_LEN];
uint16_t ADCVVal[ADC_CH_LEN];
uint16_t ADCRVal[ADC_CH_LEN];
uint8_t  rBuf[ADC_CH_LEN * 3] = {0};

tChannelSettings Channels[ADC_CH_LEN] = {
    {550, 600, 0, 5, 5, 0, CH_STATUS_DISABLED, GPIOA, GPIO_BSRR_BS_12},
    {1100, 1300, 0, 5, 10, 0, CH_STATUS_DISABLED, GPIOA, GPIO_BSRR_BS_11},
    {1100, 1300, 0, 5, 7, 0, CH_STATUS_DISABLED, GPIOA, GPIO_BSRR_BS_10}
};

void Process(void)
{
    if (State == STATE_IDLE)
        return;

    if (State == STATE_GND) {
        CH_GND();
    }
    if (State == STATE_GND_ADC) { // 2  Get true GND
        ADC_Start((uint32_t *)ADCZBuf, ADC_CH_LEN * ADC_CH_CNT);
    }
    if (State == STATE_ON) { // 3 Calc true GND
        ADC_Stop();
        for (uint8_t ch = 0; ch < ADC_CH_LEN; ch++) {
            ADCZVal[ch] = 0;
            for (uint8_t cnt = 0; cnt < ADC_CH_CNT; cnt++) {
                ADCZVal[ch] += ADCZBuf[cnt][ch];
            }
            ADCZVal[ch] /= ADC_CH_CNT;
        }
        CH_VDD();
    }
    if (State == STATE_ON_ADC) {
        ADC_Start((uint32_t *)ADCVBuf, ADC_CH_LEN * ADC_CH_CNT);
    }
    if (State == STATE_CALC) {
        ADC_Stop();
        CH_GND();
        for (uint8_t ch = 0; ch < ADC_CH_LEN; ch++) {
            ADCVVal[ch] = 0;
            for (uint8_t cnt = 0; cnt < ADC_CH_CNT; cnt++) {
                ADCVVal[ch] += ADCVBuf[cnt][ch];
            }
            ADCVVal[ch] /= ADC_CH_CNT;
        }
        for (uint8_t ch = 0; ch < ADC_CH_LEN; ch++) {
            ADCRVal[ch] = ADCVVal[ch] - ADCZVal[ch];
            if (Channels[ch].Status == CH_STATUS_DISABLED) {
                //Stop pump in any case
                Channels[ch].GPIO_Port->BSRR = Channels[ch].GPIO_Pin << 16;
                continue;
            }
            if (Channels[ch].MinDryness > ADCRVal[ch] && Channels[ch].Status == CH_STATUS_WATERING) { //Stop watering
                Channels[ch].Status = CH_STATUS_NORMAL;
                //                    Channels[ch].Cycles = 0;
            }
            if (Channels[ch].MaxDryness < ADCRVal[ch] && Channels[ch].Cycles == 0 && Channels[ch].Status == CH_STATUS_NORMAL) { //Start watering
                Channels[ch].Cycles = Channels[ch].MaxCycles;
                Channels[ch].Status = CH_STATUS_WATERING;
            }
            if (Channels[ch].Cycles == 0 && Channels[ch].Status == CH_STATUS_WATERING) { //Watering in progress, but cycles is over
                Channels[ch].Status = CH_STATUS_ECOVERFLOW;
            }
            if (Channels[ch].Status == CH_STATUS_WATERING) {
                Channels[ch].Cycles--;
                Channels[ch].CycleTimeCounter = Channels[ch].CycleTime;
                Channels[ch].GPIO_Port->BSRR  = Channels[ch].GPIO_Pin;
                WateringInProgress            = 1;
            }
        }

        uint8_t Pos = 0;
        for (uint8_t ch = 0; ch < ADC_CH_LEN; ch++) {
            Pos           = ch * 3;
            rBuf[Pos]     = (uint8_t)(ADCRVal[ch] >> 8);
            rBuf[Pos + 1] = (uint8_t)ADCRVal[ch];
            rBuf[Pos + 2] = Channels[ch].Status;
        }
        SendCommandToHub(rfCMD_R_DATA, rBuf, ADC_CH_LEN * 3);

        if (WateringInProgress == 0) {
            SwitchToState(STATE_IDLE);
            return;
        }
    }
    if (State >= STATE_WATERING && State < STATE_RESET) {
        WateringInProgress = 0;
        for (uint8_t ch = 0; ch < ADC_CH_LEN; ch++) { //Monitor cycle watering time
            if (Channels[ch].CycleTimeCounter == 0) {
                Channels[ch].GPIO_Port->BSRR = Channels[ch].GPIO_Pin << 16;
            } else {
                Channels[ch].CycleTimeCounter--;
                WateringInProgress++;
            }
        }
        if (WateringInProgress == 0)
            SwitchToState(STATE_RESET);
    }
    if (State == STATE_RESET) {
        ADC_Stop();
        for (uint8_t ch = 0; ch < ADC_CH_LEN; ch++) { //Stop all pumps in any case
            Channels[ch].GPIO_Port->BSRR = Channels[ch].GPIO_Pin << 16;
        }
    }

    State++;
    if (State > STATE_RESET)
        State = STATE_IDLE;
}

void SwitchToState(enum mState newState)
{
    if (newState == State)
        return;

    State = newState;
}
uint8_t RegistersT[4] = {0};
uint8_t FIFOStatus[2] = {0};
uint8_t TData[20]     = {0};
void uTIM_IRQHandler(TIM_TypeDef *Tim)
{
    //    uint8_t cStatus[2] = {0};
    if (Tim == TIM14) {
        GPIOB->ODR ^= GPIO_ODR_12;
        Process();
    }
    if (Tim == TIM16) { //For debug purposes
        TIM16->CR1 &= ~TIM_CR1_CEN;
        GPIOB->ODR ^= GPIO_ODR_12;
        //        SendCommandToHub(rfCMD_R_DATA, rBuf, ADC_CH_LEN*3);
        Transceiver_ReadFRR(Transceiver_FRR_A_READ, 4, RegistersT);
        uint8_t Params = 0;
        Transceiver_ReadRegs(Transceiver_FIFO_INFO, &Params, 1, FIFOStatus, 2, 1);
        if (FIFOStatus[0] > 0) {
            Transceiver_ReadRXBuf(TData, FIFOStatus[0]);
        }
    }
}

void uRTC_IRQHandler(uint32_t RTC_ISR)
{
    if (RTC_ISR & RTC_ISR_ALRAF) {
        RTC_Time_Configure(0, 0, 0);
        SwitchToState(STATE_GND);
    }
}

uint8_t SetParamsToChannel(tUpdateChannelSettings *Data)
{
    if (Data->Channel > ADC_CH_LEN) {
        //Channel number is incorrect
        return 1;
    }
    if (Channels[Data->Channel].Status != CH_STATUS_NORMAL && Channels[Data->Channel].Status != CH_STATUS_DISABLED) {
        //Can't change status while channel in use
        return 2;
    }
    Channels[Data->Channel].MinDryness       = ((uint16_t)Data->MinDryness.H << 8) | Data->MinDryness.L;
    Channels[Data->Channel].MaxDryness       = ((uint16_t)Data->MaxDryness.H << 8) | Data->MaxDryness.L;
    Channels[Data->Channel].Cycles           = 0;
    Channels[Data->Channel].MaxCycles        = Data->MaxCycles;
    Channels[Data->Channel].CycleTime        = Data->CycleTime;
    Channels[Data->Channel].CycleTimeCounter = 0; //Data->CycleTimeCounter;
    Channels[Data->Channel].Status           = Data->Status;

    return 0;
}

void LoadSettings(uint8_t Length)
{
    /*
    uint8_t isConfigStored                      = (*(__IO uint8_t *)(CHANNELS_CONFIG_FLASH_ADDRESS));
    if (isConfigStored == 0xFF) {
        //    printf("Confing not set!\n");
        return;
    }
    uint8_t StructLength = sizeof(tChannelSettings);
    for (int i = 0; i < Length * 2; i += StructLength) {
        Channels[i].     = (*(__IO uint8_t *)(SAE_FLASH_CFG_ADDR + i + 1));
        Config[i + 1] = (*(__IO uint8_t *)(SAE_FLASH_CFG_ADDR + i));
        dxprintf("%x ", Config[i]);
        dxprintf("%x ", Config[i + 1]);
    }
    dxprintf("\n");
    */
}
