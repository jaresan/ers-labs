//
// Created by Antonín Jareš on 2019-05-08.
//

#ifndef STM32F4_BLINK_UART_H
#define STM32F4_BLINK_UART_H

#include "stm32f4xx_hal.h"

class UART {

public:
    UART();
    ~UART();

    void test();
    void init();
    UART_HandleTypeDef handle;
};


#endif //STM32F4_BLINK_UART_H
