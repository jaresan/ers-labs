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
    printf("Sending...\n");
    uint8_t txBuffer[1] = { 'A' };
    printf("Sent successfuly: %d\n", HAL_UART_Transmit_IT(&handle, txBuffer, sizeof(txBuffer)) == HAL_OK);
}

void UART::init() {
    __HAL_RCC_GPIOA_CLK_ENABLE();

    HAL_NVIC_EnableIRQ(USART2_IRQn);
    HAL_NVIC_SetPriority(USART2_IRQn, 2, 1);

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

    __HAL_UART_ENABLE_IT(&handle, UART_IT_CTS | UART_IT_LBD | UART_IT_TXE | UART_IT_TC | UART_IT_RXNE | UART_IT_IDLE | UART_IT_PE | UART_IT_ERR);
}
