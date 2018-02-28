/*
 * LED.h
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 *  Modified on: 22.02.2017
 *      Author: Dominik Skoda <skoda@d3s.mff.cuni.cz>
 */

#ifndef LED_H_
#define LED_H_

#include "stm32f4xx_hal.h"

class LED {
public:

	LED(uint32_t pin);
	~LED();

	void on();
	void off();
	void init();

private:
	uint32_t pin;
};

class PulseLED {
public:
	PulseLED(LED& led, int minimalOnTimeTicks);
	~PulseLED();

	void pulse();
	void init();

	static void tickInterruptHandler();

private:
	LED& led;
	int minimalOnTimeTicks;
	int onTicks;

	void tick();

	static constexpr auto MAX_TICK_LISTENERS = 5; // Maximum number of pulse leds in the system
	static PulseLED* tickListeners[MAX_TICK_LISTENERS];
	static int tickListenersNum;
};

#endif /* LED_H_ */
