/*
 * main.h
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 *  Modified on: 22.02.2017
 *      Author: Dominik Skoda <skoda@d3s.mff.cuni.cz>
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "stm32f4xx_hal.h"
#include "Button.h"
#include "UART.h"
#include "LED.h"
#include "MiniPID.h"

extern Button infoButton;
extern void sysTickHookMain();

#endif /* MAIN_H_ */
