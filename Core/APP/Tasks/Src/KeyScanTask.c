//
// Created by 74222 on 2026/5/20.
//
#include "cmsis_os2.h"
#include "main.h"
#include "FreeRTOS.h"
#include "Tasks/Inc/LED.h"
extern osMessageQueueId_t LEDQueueHandle;

void StartKeyScanTask(void *argument)
{
    /* Infinite loop */
    LEDState state = LEDSTATE_OFF;
    LEDColor color = LEDCOLOR_RED;
    for (;;)
    {
        if (HAL_GPIO_ReadPin(KEY_1_GPIO_Port, KEY_1_Pin) == GPIO_PIN_RESET)
        {
            osDelay(10); // 消抖延时
            if (HAL_GPIO_ReadPin(KEY_1_GPIO_Port, KEY_1_Pin) == GPIO_PIN_RESET)
            {
                state = !state;
                LEDMessage *message = pvPortMalloc(sizeof(LEDMessage));
                message->color = color;
                message->state = state;
                osMessageQueuePut(LEDQueueHandle, &message, 0, osWaitForever);
            }
            while (HAL_GPIO_ReadPin(KEY_1_GPIO_Port, KEY_1_Pin) == GPIO_PIN_RESET)
            {
                osDelay(10); // 等待按键释放
            }
        }
        else
        {
            osDelay(10);
        }
    }
}
