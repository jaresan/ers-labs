//
// Created by Antonín Jareš on 2019-06-01.
//

#ifndef STM32F4_BLINK_PWMCONTROLLER_H
#define STM32F4_BLINK_PWMCONTROLLER_H

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstdint>

class PWMController {
public:
    PWMController();
    void setSpeed(uint8_t dir, float speed);
    void init();

private:
    uint32_t uwPeriod;
    TIM_OC_InitTypeDef sConfig;
    TIM_HandleTypeDef TimHandle;
};


#endif //STM32F4_BLINK_PWMCONTROLLER_H
