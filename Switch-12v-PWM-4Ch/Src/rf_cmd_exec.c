#include "rf_cmd_exec.h"
#include "tim.h"
#include <stdlib.h>

#define rbVALUE_MS_MULTIPLIER 100
#define rbSTEP_DURATION_MS 10
#define rb100_PERCENTS 100

int8_t   PWM1_Direction  = 0;
uint16_t PWM1_DeltaStep  = 0;
uint16_t PWM1_DeltaCount = 0;

int8_t   PWM2_Direction  = 0;
uint16_t PWM2_DeltaStep  = 0;
uint16_t PWM2_DeltaCount = 0;

void rfCmdExec(uint8_t *Data, uint8_t Length)
{
    uint8_t Brightness_c1 = ((uint8_t)Data[0]) > 100 ? 100 : Data[0]; //Max Brightness = 100%
    uint8_t Brightness_c2 = ((uint8_t)Data[1]) > 100 ? 100 : Data[1]; //Max Brightness = 100%
    uint8_t Brightness_c3 = 0;                                        //((uint8_t)Data[2]) > 100 ? 100 : Data[2];  //Max Brightness = 100%
    uint8_t Brightness_c4 = 0;                                        //((uint8_t)Data[3]) > 100 ? 100 : Data[3];  //Max Brightness = 100%
    uint8_t tDelay        = (uint8_t)Data[2];
    uint8_t AllowStart    = 0x00;
    int16_t d_b_c_i;

    printf("*** B1: %d, B2: %d, B3: %d, B4: %d, D: %d, L: %d\n", Brightness_c1, Brightness_c2, Brightness_c3, Brightness_c4, tDelay, Length);
    if (tDelay > 0)
    {

        // Make calculation for PWM CH1
        uint16_t n_b = ((uint16_t)htim3.Instance->ARR) * Brightness_c1 / rb100_PERCENTS;
        if (htim3.Instance->CCR4 != n_b)
        {
            PWM1_DeltaCount = tDelay * rbVALUE_MS_MULTIPLIER / rbSTEP_DURATION_MS; // Count of steps
            d_b_c_i         = abs(((uint16_t)htim3.Instance->CCR4) - n_b);

            PWM1_Direction = (uint16_t)htim3.Instance->CCR4 > n_b ? -1 : 1;
            PWM1_DeltaStep = d_b_c_i / PWM1_DeltaCount;
            AllowStart     = 0x01;

            printf("PWM1 Cur: %d, DC: %d, DS: %d, Dir: %d, New: %d\n", htim14.Instance->CCR1, PWM2_DeltaCount, PWM2_DeltaStep, PWM2_Direction, n_b);
        }

        // Make calculation for PWM CH2
        n_b = ((uint16_t)htim14.Instance->ARR) * Brightness_c2 / rb100_PERCENTS;
        if (htim14.Instance->CCR1 != n_b)
        {
            PWM2_DeltaCount = tDelay * rbVALUE_MS_MULTIPLIER / rbSTEP_DURATION_MS; // Count of steps
            d_b_c_i         = abs(((uint16_t)htim14.Instance->CCR1) - n_b);

            PWM2_Direction = (uint16_t)htim14.Instance->CCR1 > n_b ? -1 : 1;
            PWM2_DeltaStep = d_b_c_i / PWM2_DeltaCount;
            AllowStart     = 0x01;

            printf("PWM2 Cur: %d, DC: %d, DS: %d, Dir: %d, New: %d\n", htim14.Instance->CCR1, PWM2_DeltaCount, PWM2_DeltaStep, PWM2_Direction, n_b);
        }

        if (AllowStart)
        {
            htim17.Instance->ARR = MCUFreq * rbSTEP_DURATION_MS / 1000 / htim17.Instance->PSC; // Step time to change Brightness in clocks
            HAL_TIM_Base_Start_IT(&htim17);
        }
    }
    else
    {

        // Set Brightness for CH1
        if (Brightness_c1 == 0x00)
        { // Turn off the light
            htim3.Instance->CCR4 = 0x00;
        }
        if (Brightness_c1 > 0x00)
        { // Update PWM according to value
            float fBrightness    = Brightness_c1;
            htim3.Instance->CCR4 = (uint32_t)((uint16_t)htim3.Instance->ARR * (fBrightness / 100));
        }

        // Set Brightness for CH2
        if (Brightness_c2 == 0x00)
        { // Turn off the light
            htim14.Instance->CCR1 = 0x00;
        }
        if (Brightness_c2 > 0x00)
        { // Update PWM according to value
            float fBrightness     = Brightness_c2;
            htim14.Instance->CCR1 = (uint32_t)((uint16_t)htim14.Instance->ARR * (fBrightness / 100));
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

    if (htim->Instance == htim17.Instance)
    {
        if (PWM1_DeltaCount > 0)
        {
            htim3.Instance->CCR4 += (PWM1_DeltaStep * PWM1_Direction);
            PWM1_DeltaCount--;
        }

        if (PWM2_DeltaCount > 0)
        {
            htim14.Instance->CCR1 += (PWM2_DeltaStep * PWM2_Direction);
            PWM2_DeltaCount--;
        }

        if (PWM1_DeltaCount == 0 && PWM2_DeltaCount == 0)
        {
            HAL_TIM_Base_Stop(&htim17);
        }
    }
}
