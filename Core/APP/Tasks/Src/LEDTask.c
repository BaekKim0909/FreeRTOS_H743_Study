#include "cmsis_os2.h"
#include "Tasks/Inc/LED.h"
#include "FreeRTOS.h"
#include "main.h"
extern osMessageQueueId_t LEDQueueHandle;
//
// Created by 74222 on 2026/5/20.
//
void StartLEDTask(void *argument)
{
    for (;;)
    {
        LEDMessage *message;
        osMessageQueueGet(LEDQueueHandle, &message, 0,osWaitForever);
        switch (message->color)
        {
            case LEDCOLOR_GREEN:
                HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, message->state ? GPIO_PIN_RESET : GPIO_PIN_SET);
                break;
            case LEDCOLOR_RED:
                HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, message->state ? GPIO_PIN_RESET : GPIO_PIN_SET);
                break;
        }
        vPortFree(message);
    }
}
