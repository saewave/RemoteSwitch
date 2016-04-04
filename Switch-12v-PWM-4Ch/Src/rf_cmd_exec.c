#include "rf_cmd_exec.h"
#include "tim.h"
#include <stdlib.h>

#define rbVALUE_MS_MULTIPLIER 100
#define rbSTEP_DURATION_MS 10
#define rb100_PERCENTS 100

int8_t Direction = 0;
uint16_t DeltaStep = 0;
uint16_t DeltaCount = 0;

void rfCmdExec(uint8_t *Data, uint8_t Length) {
  uint8_t Brightness = ((uint8_t)Data[0]) > 100 ? 100 : Data[0];  //Max Brightness = 100%
  uint16_t tDelay = (uint16_t)Data[1] << 8 | Data[2];
//  printf("B: %d, D: %d\n", Brightness, tDelay);
  if (tDelay > 0) {
    uint16_t n_b = ((uint16_t)htim14.Instance->ARR) * Brightness / rb100_PERCENTS;
    if (htim14.Instance->CCR1 == n_b) {
      return;
    }
    uint32_t t_st_c = MCUFreq * rbSTEP_DURATION_MS / 1000 / htim17.Instance->PSC;   // Step time to change Brightness in clocks
    DeltaCount = tDelay * rbVALUE_MS_MULTIPLIER / rbSTEP_DURATION_MS;      // Count of steps
    int16_t d_b_c_i = abs(((uint16_t)htim14.Instance->CCR1) - n_b);

    Direction = (uint16_t)htim14.Instance->CCR1 > n_b ? -1 : 1;
    DeltaStep = d_b_c_i / DeltaCount;
    
    htim17.Instance->ARR = t_st_c;    //    htim17.Init.Period = t_st_c;

    HAL_TIM_Base_Start_IT(&htim17);
//    printf("Cur: %d, DC: %d, DS: %d, Dir: %d, New: %d\n", htim14.Instance->CCR1, DeltaCount, DeltaStep, Direction, n_b);
  } else {

    if (Brightness == 0x00) {  // Turn off the light
      htim14.Instance->CCR1 = 0x00;
    }
    if (Brightness > 0x00) {  // Update PWM according to value
      float fBrightness = Brightness;
      htim14.Instance->CCR1 = (uint32_t)((uint16_t)htim14.Instance->ARR * (fBrightness / 100));
    }
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

  if (htim->Instance == htim17.Instance) {
    if (DeltaCount > 0) {
      htim14.Instance->CCR1 += (DeltaStep * Direction);
      DeltaCount--;
    } else {
      HAL_TIM_Base_Stop(&htim17);
    }
  }
}
