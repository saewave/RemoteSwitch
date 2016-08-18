/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "task.h"

/* USER CODE BEGIN Includes */
#include "UUID.h"
#include "at_processor.h"
#include "config.h"
#include "gpio.h"
#include "iwdg.h"
#include "nRF24L01P.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "xdebug.h"
#include <stdlib.h>
#include <string.h>
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId    defaultTaskHandle;
osThreadId    CMDQHandle;
osThreadId    U2TXHandle;
osThreadId    SPITxIrqHandle;
osThreadId    SPIRxIrqHandle;
osMessageQId  CmdQueueHandle;
osMessageQId  U2TxQueueHandle;
osSemaphoreId nRFTxStatusHandle;
osSemaphoreId nRFRxStatusHandle;

/* USER CODE BEGIN Variables */

#define OUTPUT_DATA_LENGTH 20

uint8_t Usart2Char                   = 0x00;
uint8_t Usart2Cmd[AS_CMD_LENGTH + 1] = {0x00};
uint8_t CPosition                    = 0x00;

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const *argument);
void CmdProcessQueue(void const *argument);
void Usart2Tx(void const *argument);
void SPITxIrqStatus(void const *argument);
void SPIRxIrqStatus(void const *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */
void AddCMDToQueue(void);
/* USER CODE END FunctionPrototypes */

/* Hook prototypes */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void)
{
    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* Create the semaphores(s) */
    /* definition and creation of nRFTxStatus */
    osSemaphoreDef(nRFTxStatus);
    nRFTxStatusHandle = osSemaphoreCreate(osSemaphore(nRFTxStatus), 1);

    /* definition and creation of nRFRxStatus */
    osSemaphoreDef(nRFRxStatus);
    nRFRxStatusHandle = osSemaphoreCreate(osSemaphore(nRFRxStatus), 1);

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* Create the thread(s) */
    /* definition and creation of defaultTask */
    osThreadDef(defaultTask, StartDefaultTask, osPriorityBelowNormal, 0, 128);
    defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

    /* definition and creation of CMDQ */
    osThreadDef(CMDQ, CmdProcessQueue, osPriorityAboveNormal, 0, 512);
    CMDQHandle = osThreadCreate(osThread(CMDQ), NULL);

    /* definition and creation of U2TX */
    osThreadDef(U2TX, Usart2Tx, osPriorityBelowNormal, 0, 256);
    U2TXHandle = osThreadCreate(osThread(U2TX), NULL);

    /* definition and creation of SPITxIrq */
    osThreadDef(SPITxIrq, SPITxIrqStatus, osPriorityNormal, 0, 256);
    SPITxIrqHandle = osThreadCreate(osThread(SPITxIrq), NULL);

    /* definition and creation of SPIRxIrq */
    osThreadDef(SPIRxIrq, SPIRxIrqStatus, osPriorityNormal, 0, 256);
    SPIRxIrqHandle = osThreadCreate(osThread(SPIRxIrq), NULL);

    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    /* USER CODE END RTOS_THREADS */

    /* Create the queue(s) */
    /* definition and creation of CmdQueue */
    osMessageQDef(CmdQueue, 32, xCmd);
    CmdQueueHandle = osMessageCreate(osMessageQ(CmdQueue), NULL);

    /* definition and creation of U2TxQueue */
    osMessageQDef(U2TxQueue, 32, xCmdResponse);
    U2TxQueueHandle = osMessageCreate(osMessageQ(U2TxQueue), NULL);

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const *argument)
{

    /* USER CODE BEGIN StartDefaultTask */
    /* Infinite loop */
    for (;;)
    {
        HAL_IWDG_Refresh(&hiwdg);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        osDelay(500);
    }
    /* USER CODE END StartDefaultTask */
}

/* CmdProcessQueue function */
void CmdProcessQueue(void const *argument)
{
    /* USER CODE BEGIN CmdProcessQueue */

    xCmd qCmd;

    /* Infinite loop */
    for (;;)
    {
        if (xQueueReceive(CmdQueueHandle, &qCmd, portMAX_DELAY))
        {
            ProcessATCommand((char *)qCmd.cData, qCmd.cLength);
            free(qCmd.cData);
        }
    }
    /* USER CODE END CmdProcessQueue */
}

/* Usart2Tx function */
void Usart2Tx(void const *argument)
{
    /* USER CODE BEGIN Usart2Tx */

    xCmdResponse qCmdResponse;

    /* Infinite loop */
    for (;;)
    {
        if (xQueueReceive(U2TxQueueHandle, &qCmdResponse, portMAX_DELAY))
        {
            //      dxprintf("=%s\n", qCmdResponse.cData);
            HAL_StatusTypeDef Status;
            do
            {
                Status = HAL_UART_Transmit(&huart2, qCmdResponse.cData, qCmdResponse.cLength, 100);
            } while (Status != HAL_OK);
            do
            {
                Status = HAL_UART_Transmit(&huart1, qCmdResponse.cData, qCmdResponse.cLength, 100);
            } while (Status != HAL_OK);
            vPortFree(qCmdResponse.cData);
        }
    }
    /* USER CODE END Usart2Tx */
}

/* SPITxIrqStatus function */
void SPITxIrqStatus(void const *argument)
{
    /* USER CODE BEGIN SPITxIrqStatus */
    /* Infinite loop */
    for (;;)
    {
        xSemaphoreTake(nRFTxStatusHandle, portMAX_DELAY);

        nRF24_HandleStatus(&hspi2, CHIP_Tx);
    }
    /* USER CODE END SPITxIrqStatus */
}

/* SPIRxIrqStatus function */
void SPIRxIrqStatus(void const *argument)
{
    /* USER CODE BEGIN SPIRxIrqStatus */
    /* Infinite loop */
    for (;;)
    {
        xSemaphoreTake(nRFRxStatusHandle, portMAX_DELAY);

        nRF24_HandleStatus(&hspi2, CHIP_Rx);
    }
    /* USER CODE END SPIRxIrqStatus */
}

/* USER CODE BEGIN Application */
void USART1_IRQHandler(void)
{
    if ((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET) && (__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_RXNE) != RESET))
    {
        Usart2Char = (uint8_t)(huart1.Instance->DR & (uint8_t)0x00FF);
        if (Usart2Char == AS_CMD_END_CHAR)
        {
            AddCMDToQueue();
        }
        else
        {
            if (CPosition < AS_CMD_LENGTH)
            {
                Usart2Cmd[CPosition++] = Usart2Char;
            }
            else
            {
                CPosition          = 0x00;
                uint8_t ErrorMsg[] = "Error Cmd too long!\n";
                HAL_UART_Transmit(&huart1, ErrorMsg, sizeof(ErrorMsg) - 1, 10);
            }
        }
    }

    if ((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) != RESET) && (__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_IDLE) != RESET))
    {
        AddCMDToQueue();
    }
    __HAL_UART_CLEAR_PEFLAG(&huart1);
}

void USART2_IRQHandler(void)
{
    if ((__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) != RESET) && (__HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_RXNE) != RESET))
    {
        Usart2Char = (uint8_t)(huart2.Instance->DR & (uint8_t)0x00FF);
        if (Usart2Char == AS_CMD_END_CHAR)
        {
            AddCMDToQueue();
        }
        else
        {
            if (CPosition < AS_CMD_LENGTH)
            {
                Usart2Cmd[CPosition++] = Usart2Char;
            }
            else
            {
                CPosition          = 0x00;
                uint8_t ErrorMsg[] = "Error Cmd too long!\n";
                HAL_UART_Transmit(&huart2, &ErrorMsg[0], sizeof(ErrorMsg) - 1, 10);
            }
        }
    }

    if ((__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE) != RESET) && (__HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_IDLE) != RESET))
    {
        AddCMDToQueue();
    }
    __HAL_UART_CLEAR_PEFLAG(&huart2);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    if (GPIO_Pin == GPIO_PIN_13)
    {
        dxprintf("Button... But for what?\n");
    }
    else if (GPIO_Pin == T_IRQ_Pin)
    {
        xSemaphoreGiveFromISR(nRFTxStatusHandle, &xHigherPriorityTaskWoken);
    }
    else if (GPIO_Pin == R_IRQ_Pin)
    {
        xSemaphoreGiveFromISR(nRFRxStatusHandle, &xHigherPriorityTaskWoken);
    }
    else
    {
        dxprintf("Uncknown IT, Pin: %x!\n", GPIO_Pin);
    }
}
/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM10) {
    AddCMDToQueue();
  }
}
*/
void AddCMDToQueue(void)
{
    if (CPosition < 5)
    {
        CPosition = 0x00;
    }
    else if (Usart2Cmd[0] != AS_CMD_BEGIN_CHAR1 || Usart2Cmd[1] != AS_CMD_BEGIN_CHAR2 || Usart2Cmd[2] != AS_CMD_BEGIN_CHAR3)
    {
        CPosition = 0x00;
    }
    else
    {
        uint8_t *Buf = (uint8_t *)malloc(CPosition);
        if (Buf == NULL)
        {
            CPosition          = 0x00;
            uint8_t ErrorMsg[] = "Error: can't allocate memmory!\n";
            HAL_UART_Transmit_DMA(&huart2, &ErrorMsg[0], sizeof(ErrorMsg));
        }
        else
        {
            Usart2Cmd[CPosition] = 0x00;
            memcpy(Buf, &Usart2Cmd, CPosition);
            xCmd qCmd;
            qCmd.cLength = CPosition;
            qCmd.cData   = Buf;
            CPosition    = 0x00;
            //          dxprintf("q: %s, L: %d, P: %p, B: %p\n", qCmd.cData, qCmd.cLength, qCmd.cData, Buf);
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            if (xQueueSendFromISR(CmdQueueHandle, &qCmd, &xHigherPriorityTaskWoken) != pdPASS)
            {
                //Error, the queue if full!!!
                //TODO: handle this error
                dxprintf("Error to add to Q!\n");
            }
        }
    }
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
