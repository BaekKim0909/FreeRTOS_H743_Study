//
// Created by 74222 on 2026/5/20.
//

#ifndef H743_LED_H
#define H743_LED_H

typedef enum
{
    LEDCOLOR_RED = 0,
    LEDCOLOR_GREEN
} LEDColor;

typedef enum
{
    LEDSTATE_OFF = 0,
    LEDSTATE_ON
} LEDState;


typedef struct
{
    LEDColor color;
    LEDState state;
} LEDMessage;
#endif //H743_LED_H
