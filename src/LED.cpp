/*
 * LED.cpp
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 *  Modified on: 22.02.2017
 *      Author: Dominik Skoda <skoda@d3s.mff.cuni.cz>
 */

#include "LED.h"

LED::LED(uint32_t pin) : pin(pin) {
}

LED::~LED() {
}

void LED::init() {
	GPIO_InitTypeDef  gpioInitStruct;

	// Enable GPIO pins connected to LEDs and set them as output */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_Init;
	GPIO_Init.Pin = pin;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_NOPULL;
	GPIO_Init.Speed = GPIO_SPEED_HIGH;  
	HAL_GPIO_Init(GPIOD, &GPIO_Init);
}

void LED::on() {
	HAL_GPIO_WritePin(GPIOD, pin, GPIO_PIN_SET);
}

void LED::off() {
	HAL_GPIO_WritePin(GPIOD, pin, GPIO_PIN_RESET);
}

PulseLED* PulseLED::tickListeners[MAX_TICK_LISTENERS];
int PulseLED::tickListenersNum = 0;

void PulseLED::tickInterruptHandler() {
	for (int idx = 0; idx < tickListenersNum; idx++) {
		tickListeners[idx]->tick();
	}
}


PulseLED::PulseLED(LED& led, int minimalOnTimeTicks) : led(led), minimalOnTimeTicks(minimalOnTimeTicks), onTicks(-1) {
}

PulseLED::~PulseLED() {
}

void PulseLED::init() {
	assert_param(tickListenersNum < MAX_TICK_LISTENERS);
	tickListeners[tickListenersNum++] = this;
}

void PulseLED::pulse() {
	onTicks = 0;
	led.on();
}

void PulseLED::tick() {
	if (onTicks != -1) {
		onTicks++;

		if (onTicks > minimalOnTimeTicks) {
			onTicks = -1;
			led.off();
		}
	}
}

