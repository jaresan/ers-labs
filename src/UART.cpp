//
// Created by Antonín Jareš on 2019-05-08.
//

#include <cstdint>
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_conf.h"
#include <cstdio>
#include "UART.h"


UART::UART(){
}

UART::~UART() {
}

void UART::test() {
    uint8_t buffer[4];
    HAL_UART_Receive(&handle, buffer, sizeof(buffer), HAL_MAX_DELAY);
    printf("Received %s\n", buffer);
//    uint8_t txBuffer[5] = { buffer[0], buffer[1], buffer[2], buffer[3], 10 };
    HAL_UART_Transmit(&handle, buffer, sizeof(buffer), HAL_MAX_DELAY);
}

void UART::init() {
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_Init;
    GPIO_Init.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_Init.Mode = GPIO_MODE_AF_PP;
    GPIO_Init.Alternate = GPIO_AF7_USART2;
    GPIO_Init.Pull = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(GPIOA, &GPIO_Init);

    __HAL_RCC_USART2_CLK_ENABLE();

    UART_HandleTypeDef Handle;
    UART_InitTypeDef HandleInit;

    Handle.Instance = USART2;
    HandleInit.BaudRate = 115200;
    HandleInit.WordLength = UART_WORDLENGTH_8B;
    HandleInit.StopBits = UART_STOPBITS_1;
    HandleInit.Parity = UART_PARITY_NONE;
    HandleInit.Mode = UART_MODE_TX_RX;
    HandleInit.HwFlowCtl = UART_HWCONTROL_NONE;
    HandleInit.OverSampling = UART_OVERSAMPLING_16;
    Handle.Init = HandleInit;

    handle = Handle;
    HAL_UART_Init(&handle);
}
