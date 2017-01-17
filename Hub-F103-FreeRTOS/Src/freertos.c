
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "F103RE_Peripheral.h"
#include "at_processor.h"
#include "config.h"
#include "nRF24L01P.h"

/* Variables -----------------------------------------------------------------*/

TaskHandle_t      defaultTaskHandle;
TaskHandle_t      CMDQHandle;
TaskHandle_t      U1TXHandle;
TaskHandle_t      U2TXHandle;
TaskHandle_t      SPITxIrqHandle;
TaskHandle_t      SPIRxIrqHandle;
QueueHandle_t     CmdQueueHandle;
QueueHandle_t     U1TxQueueHandle;
QueueHandle_t     U2TxQueueHandle;
SemaphoreHandle_t nRFTxStatusHandle;
SemaphoreHandle_t nRFRxStatusHandle;
SemaphoreHandle_t U1TxTCHandle;
SemaphoreHandle_t U2TxTCHandle;

uint8_t Usart1DMABuf[SAE_OUTPUT_DATA_LENGTH];
uint8_t Usart2DMABuf[SAE_OUTPUT_DATA_LENGTH];

void StartDefaultTask(void const *argument);
void CmdProcessQueue(void const *argument);
void Usart1Tx(void const *argument);
void Usart2Tx(void const *argument);
void SPITxIrqStatus(void const *argument);
void SPIRxIrqStatus(void const *argument);

void AddCMDToQueue(uint8_t *Data, uint8_t Length);

/* Init FreeRTOS */

void FreeRTOS_Init(void)
{
    BaseType_t xReturned;
        nRFTxStatusHandle = xSemaphoreCreateBinary();
        nRFRxStatusHandle = xSemaphoreCreateBinary();
    U1TxTCHandle = xSemaphoreCreateBinary();
    U2TxTCHandle = xSemaphoreCreateBinary();

    xReturned = xTaskCreate(
        (TaskFunction_t)StartDefaultTask, /* Function that implements the task. */
        "StartDefaultTask",               /* Text name for the task. */
        configMINIMAL_STACK_SIZE,         /* Stack size in words, not bytes. */
        NULL,                             /* Parameter passed into the task. */
        tskIDLE_PRIORITY,                 /* Priority at which the task is created. */
        &defaultTaskHandle);              /* Used to pass out the created task's handle. */

    xReturned = xTaskCreate(
        (TaskFunction_t)CmdProcessQueue, /* Function that implements the task. */
        "CmdProcessQueue",               /* Text name for the task. */
        256,                             /* Stack size in words, not bytes. */
        NULL,                            /* Parameter passed into the task. */
        tskIDLE_PRIORITY + 2,            /* Priority at which the task is created. */
        &CMDQHandle);                    /* Used to pass out the created task's handle. */

    xReturned = xTaskCreate(
        (TaskFunction_t)Usart1Tx, /* Function that implements the task. */
        "Usart1Tx",               /* Text name for the task. */
        128,                      /* Stack size in words, not bytes. */
        NULL,                     /* Parameter passed into the task. */
        tskIDLE_PRIORITY + 1,     /* Priority at which the task is created. */
        &U1TXHandle);             /* Used to pass out the created task's handle. */

    xReturned = xTaskCreate(
        (TaskFunction_t)Usart2Tx, /* Function that implements the task. */
        "Usart2Tx",               /* Text name for the task. */
        128,                      /* Stack size in words, not bytes. */
        NULL,                     /* Parameter passed into the task. */
        tskIDLE_PRIORITY + 1,     /* Priority at which the task is created. */
        &U2TXHandle);             /* Used to pass out the created task's handle. */

    xReturned = xTaskCreate(
        (TaskFunction_t)SPITxIrqStatus, /* Function that implements the task. */
        "SPITxIrqStatus",               /* Text name for the task. */
        128,                            /* Stack size in words, not bytes. */
        NULL,                           /* Parameter passed into the task. */
        tskIDLE_PRIORITY + 3,           /* Priority at which the task is created. */
        &SPITxIrqHandle);               /* Used to pass out the created task's handle. */

    xReturned = xTaskCreate(
        (TaskFunction_t)SPIRxIrqStatus, /* Function that implements the task. */
        "SPIRxIrqStatus",               /* Text name for the task. */
        128,                            /* Stack size in words, not bytes. */
        NULL,                           /* Parameter passed into the task. */
        tskIDLE_PRIORITY + 3,           /* Priority at which the task is created. */
        &SPIRxIrqHandle);               /* Used to pass out the created task's handle. */

    UNUSED(xReturned);

    CmdQueueHandle = xQueueCreate(SAE_CMD_QUEUE_LENGTH, sizeof(xCmd));

    U1TxQueueHandle = xQueueCreate(SAE_CMD_QUEUE_LENGTH, sizeof(xCmdResponse));
    U2TxQueueHandle = xQueueCreate(SAE_CMD_QUEUE_LENGTH, sizeof(xCmdResponse));

    USART_Configure();
    EXTI_Configure();

    vTaskStartScheduler();
}

void StartDefaultTask(void const *argument)
{

    for (;;)
    {
        SET_BIT(GPIOA->BSRR, GPIO_BSRR_BS5);
        vTaskDelay(500);
        SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR5);
        vTaskDelay(500);
    }
}

void CmdProcessQueue(void const *argument)
{
    xCmd    qCmd;

    for (;;)
    {
        if (xQueueReceive(CmdQueueHandle, &qCmd, portMAX_DELAY) == pdTRUE)
        {
            ProcessATCommand((char *)qCmd.cData, qCmd.cLength);
            vPortFree(qCmd.cData);
        }
    }
}

void Usart1Tx(void const *argument)
{
    xCmdResponse qCmdResponse;
    for (;;)
    {
        if (xQueueReceive(U1TxQueueHandle, &qCmdResponse, portMAX_DELAY) == pdTRUE)
        {
            if (qCmdResponse.cLength > 0)
            {
                if (xSemaphoreTake(U1TxTCHandle, portMAX_DELAY) == pdTRUE)
                {
                    memcpy(Usart1DMABuf, qCmdResponse.cData, qCmdResponse.cLength);
                    USART_DMASendData(USART1, Usart1DMABuf, qCmdResponse.cLength);
                    vPortFree(qCmdResponse.cData);
                }
            }
        }
    }
}

void Usart2Tx(void const *argument)
{
    xCmdResponse qCmdResponse;
    for (;;)
    {
        if (xQueueReceive(U2TxQueueHandle, &qCmdResponse, portMAX_DELAY) == pdTRUE)
        {
            if (qCmdResponse.cLength > 0)
            {
                if (xSemaphoreTake(U2TxTCHandle, portMAX_DELAY) == pdTRUE)
                {
                    memcpy(Usart2DMABuf, qCmdResponse.cData, qCmdResponse.cLength);
                    USART_DMASendData(USART2, Usart2DMABuf, qCmdResponse.cLength);
                    vPortFree(qCmdResponse.cData);
                }
            }
        }
    }
}

void SPITxIrqStatus(void const *argument)
{
    for (;;)
    {
        xSemaphoreTake(nRFTxStatusHandle, portMAX_DELAY);

        nRF24_HandleStatus(SPI2, CHIP_Tx);
    }
}

void SPIRxIrqStatus(void const *argument)
{
    for (;;)
    {
        xSemaphoreTake(nRFRxStatusHandle, portMAX_DELAY);

        nRF24_HandleStatus(SPI2, CHIP_Rx);
    }
}

void uUSART_IRQHandler(USART_TypeDef *USART, uint32_t SR, uint8_t *Data, uint8_t Length)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (SR & USART_SR_IDLE && Length > 0)
    {
        if (Data[0] != SAE_CMD_BEGIN_CHAR1 || Data[1] != SAE_CMD_BEGIN_CHAR2 || Data[2] != SAE_CMD_BEGIN_CHAR3)
        {
            return;
        }
        else
        {
            xCmd qCmd;
            qCmd.cLength = Length - 3;
            qCmd.cData   = (uint8_t *)pvPortMalloc(Length - 3);
            if (qCmd.cData != NULL)
            {
                memcpy(qCmd.cData, &Data[3], Length - 3);
                if (xQueueSendFromISR(CmdQueueHandle, &qCmd, &xHigherPriorityTaskWoken) != pdPASS)
                {
                }
            }
        }
    }
    else if (SR & USART_SR_TC)
    {
        if (USART == USART1)
        {
            xSemaphoreGiveFromISR(U1TxTCHandle, &xHigherPriorityTaskWoken);
        }
        else if (USART == USART2)
        {
            xSemaphoreGiveFromISR(U2TxTCHandle, &xHigherPriorityTaskWoken);
        }
    }
}

void uEXTI_IRQHandler(uint32_t Pin)
{
    if (Pin & T_IRQ_Pin) {
        xSemaphoreGiveFromISR(nRFTxStatusHandle, NULL);
    }else if (Pin & R_IRQ_Pin) {
        xSemaphoreGiveFromISR(nRFRxStatusHandle, NULL);
    }
}

/**
 *  \brief Queue response
 *  
 *  \param [in] Response Text of response
 *  \param [in] USART 
 *  \return void
 */
void QueueResponse(char *Response, uint8_t USART)
{
    if (!U1TxQueueHandle) return;
    if (!U2TxQueueHandle) return;
    uint8_t  Length = strlen(Response);
    
    if (USART & OUSART1) {
        xCmdResponse qCmdResponseU1;
        qCmdResponseU1.cLength = Length;
        qCmdResponseU1.cData   = (uint8_t *)pvPortMalloc(Length);
        if (qCmdResponseU1.cData == NULL)
        {
            USART_DMASendData(USART1, (uint8_t *)"1QueueResponse: Can't allocate memory!\n", 39);
            vTaskDelay(50);
            return;
        }
        memcpy(qCmdResponseU1.cData, Response, Length);
        xQueueSend(U1TxQueueHandle, &qCmdResponseU1, portMAX_DELAY);
    }
    if (USART & OUSART2) {
        xCmdResponse qCmdResponseU2;
        qCmdResponseU2.cLength = Length;
        qCmdResponseU2.cData   = (uint8_t *)pvPortMalloc(Length);
        if (qCmdResponseU2.cData == NULL)
        {
            USART_DMASendData(USART2, (uint8_t *)"2QueueResponse: Can't allocate memory!\n", 39);
            vTaskDelay(50);
            return;
        }
        memcpy(qCmdResponseU2.cData, Response, Length);
        xQueueSend(U2TxQueueHandle, &qCmdResponseU2, portMAX_DELAY);
    }
}
