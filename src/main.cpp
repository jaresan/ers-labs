#include "main.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"
#include "misc.h"

#include <cstdio>

uint32_t mainCycles = 0;

LED::Properties greenLedProps {
	GPIOD, GPIO_Pin_12, RCC_AHB1Periph_GPIOD
};
LED::Properties redLedProps {
	GPIOD, GPIO_Pin_14, RCC_AHB1Periph_GPIOD
};
LED::Properties orangeLedProps {
	GPIOD, GPIO_Pin_13, RCC_AHB1Periph_GPIOD
};
LED::Properties blueLedProps {
	GPIOD, GPIO_Pin_15, RCC_AHB1Periph_GPIOD
};
LED greenLed(greenLedProps);
LED redLed(redLedProps);
LED blueLed(blueLedProps);
LED orangeLed(orangeLedProps);

PulseLED greenPulseLed(greenLed, 10);

Button::Properties userButtonProps {
	GPIOA, GPIO_Pin_0, RCC_AHB1Periph_GPIOA, EXTI_Line0, EXTI_PortSourceGPIOA, EXTI_PinSource0, EXTI0_IRQn
};
Button infoButton(userButtonProps);

void handleInfoButtonInterrupt(void*) {
	greenPulseLed.pulse();
}


extern void sysTickHookMain() 
{
	static int counter = 0;

	if (counter >= 25) {
		orangeLed.off();
	} else {
		orangeLed.on();
	}

	counter++;

	if (counter >= 50) {
		counter = 0;
	}
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);	// Configures how priorities are interpreted
	                                                // 4 bits for pre-emption priority, 0 bits for non-preemptive subpriority

	// Set SysTick to fire every 10ms
	NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 0));
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 100);

	infoButton.setPriority(2,0);

	greenLed.init();
	redLed.init();
	blueLed.init();
	orangeLed.init();
	
	greenPulseLed.init();
	
	infoButton.setPressedListener(handleInfoButtonInterrupt, nullptr);
	infoButton.init();

	printf("Started.\n");


	NVIC_SystemLPConfig(NVIC_LP_SLEEPONEXIT, ENABLE); // This ..
	while (1) {
		// The following puts the processor to sleep and wakes it up only for handling interrupts (and then puts it to sleep again).
		// As a consequence, it never wakes up here.
		__WFI(); // ... and this has to be commented out when debugging.
	}
}


