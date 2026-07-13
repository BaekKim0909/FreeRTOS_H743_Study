//
// Created by 74222 on 2026/5/20.
//
#include "cmsis_os2.h"
#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "lv_demo_music.h"

void StartUITask(void *argument)
{
    lv_init();
    lv_port_disp_init();
    lv_demo_music();
    /* Infinite loop */
    for (;;)
    {
        lv_timer_handler();
        osDelay(2);
    }
}
