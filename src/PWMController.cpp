//
// Created by Antonín Jareš on 2019-06-01.
//

#include "PWMController.h"

PWMController::PWMController() {
    __HAL_RCC_TIM4_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void PWMController::init() {
    // Set up PWM direction pins
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_Init;
    GPIO_Init.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init.Pull = GPIO_NOPULL;
    GPIO_Init.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_Init);
    //

    uwPeriod = (SystemCoreClock / 17570) - 1;

    //    xPulse = uwPeriod / 2  - 1; -> 50%

    TimHandle.Instance = TIM4;

    TimHandle.Init.Period = uwPeriod;
    TimHandle.Init.Prescaler = 0;
    TimHandle.Init.ClockDivision = 0;
    TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
    TimHandle.Init.RepetitionCounter = 0;

    if (HAL_TIM_PWM_Init(&TimHandle) != HAL_OK) {
//        Error_Handler();
    }

    sConfig.OCMode = TIM_OCMODE_PWM2;
    sConfig.OCFastMode = TIM_OCFAST_DISABLE;
    sConfig.OCPolarity = TIM_OCPOLARITY_LOW;
    sConfig.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfig.OCIdleState = TIM_OCIDLESTATE_SET;
    sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    sConfig.Pulse = 0;

    if (HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_1) != HAL_OK) {
//        Error_Handler();
    }

    if (HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_2) != HAL_OK) {
//        Error_Handler();
    };
}

void PWMController::setSpeed(uint8_t dir, float speed) {
    uint32_t pulse;
    unsigned int channel;
    uint16_t pin;
    bool reset = false;

    if (dir == 0) {
        channel = TIM_CHANNEL_1;
        pin = GPIO_PIN_0;
    } else {
        channel = TIM_CHANNEL_2;
        pin = GPIO_PIN_1;
    }

    if (speed == 0) {
        pulse = 0;
    } else if (speed < 0) {
        reset = true;
        pulse = uint32_t(uwPeriod + (uwPeriod / 100.0) * speed) - 1;
    } else {
        pulse = uint32_t((uwPeriod / 100.0) * speed) - 1;
    }

    sConfig.Pulse = pulse;
    if (reset) {
        HAL_GPIO_WritePin(GPIOC, pin, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(GPIOC, pin, GPIO_PIN_SET);
    }

    HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, channel);
    HAL_TIM_PWM_Start(&TimHandle, channel);
}

