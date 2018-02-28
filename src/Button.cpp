/*
 * Button.cpp
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 *  Modified on: 22.02.2017
 *      Author: Dominik Skoda <skoda@d3s.mff.cuni.cz>
 */

#include "Button.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_conf.h"

Button::Button(Properties& initProps) : props(initProps) {
}

Button::~Button() {
}

void Button::setPriority(uint8_t irqPreemptionPriority, uint8_t irqSubPriority) {
	this->irqPreemptionPriority = irqPreemptionPriority;
	this->irqSubPriority = irqSubPriority;
}

void Button::init() {

	/* Enable the Button Clock */
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/* Configure Button pin as input */
	GPIO_InitTypeDef gpioInitStruct;
	gpioInitStruct.Pin = props.pin;
	gpioInitStruct.Mode = GPIO_MODE_IT_FALLING;
	gpioInitStruct.Pull = GPIO_NOPULL;
	gpioInitStruct.Speed = GPIO_SPEED_HIGH;

	HAL_GPIO_Init(props.gpio, &gpioInitStruct);

	HAL_NVIC_SetPriority(props.irqn, irqPreemptionPriority, irqSubPriority);
	HAL_NVIC_EnableIRQ(props.irqn);

}


bool Button::isPressed() {
	return HAL_GPIO_ReadPin(props.gpio, props.pin);
}

void Button::setPressedListener(Listener pressedListener, void* obj) {
	this->pressedListener = pressedListener;
	pressedListenerObj = obj;
}

void Button::pressedInterruptHandler() {
	// Clear the pending bit
	__HAL_GPIO_EXTI_CLEAR_IT(props.pin);

	assert_param(pressedListener);
	pressedListener(pressedListenerObj);
}

