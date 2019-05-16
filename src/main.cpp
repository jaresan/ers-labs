#include "main.h"
#include "LED.h"
#include "Button.h"
#include "UART.h"

#include <cstdio>
#include <cstdint>


static void SystemClock_Config(void);
static void Error_Handler(void);


void nextLEDFlash(LED* leds);

Button::Properties userButtonProps {
	GPIOA, GPIO_PIN_0, EXTI0_IRQn
};
Button infoButton(userButtonProps);

LED greenLed(GPIO_PIN_12);
LED orangeLed(GPIO_PIN_13);
LED redLed(GPIO_PIN_14);
LED blueLed(GPIO_PIN_15);

PulseLED bluePulseLed(blueLed, 1000);
UART uart;

enum LEDMode {
	All = 0,
	Clockwise = 1,
	AntiClockwise = 2
};

LEDMode ledMode = Clockwise;
bool allLedsOn = false;
int ledIndex = 0;
int lastLedIndex = 0;

void handleInfoButtonInterrupt(void*) {
	printf("BUTTON PRESSED!\n");
	switch (ledMode) {
		case All:
			ledMode = Clockwise;
			break;
		case Clockwise:
			ledMode = AntiClockwise;
			break;
		case AntiClockwise:
			ledMode = All;
			break;
	}
}

extern void sysTickHookMain() 
{
}



int main(void)
{
    /* STM32F4xx HAL library initialization:
		- Configure the Flash prefetch, Flash preread and Buffer caches
		- Systick timer is configured by default as source of time base, but user
				can eventually implement his proper time base source (a general purpose
				timer for example or other time source), keeping in mind that Time base
				duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
				handled in milliseconds basis.
		- Low Level Initialization
	*/
	HAL_Init();

	// Configure the system clock to 168 MHz
	SystemClock_Config();

	uart.init();
	infoButton.init();
	infoButton.setPriority(2,0);
	infoButton.setPressedListener(handleInfoButtonInterrupt, nullptr);
	uart.startEcho();

	// Infinite loop
	while (1) {}
}


void nextLEDFlash(LED* leds) {
	int ledCount = sizeof(leds);

	if (allLedsOn && ledMode != All) {
		for (int i = 0; i < ledCount; ++i) {
			leds[i].off();
		}
	}

	leds[lastLedIndex].off();
	leds[ledIndex].on();

	lastLedIndex = ledIndex;
	if (ledMode == Clockwise) {
		ledIndex = (ledIndex + 1) % ledCount;
	} else if (ledMode == AntiClockwise) {
		ledIndex--;
		if (ledIndex == -1) ledIndex = 3;
	} else {
		if (allLedsOn) {
			for (int i = 0; i < ledCount; ++i) {
				leds[i].off();
			}
		} else {
			for (int i = 0; i < ledCount; ++i) {
				leds[i].on();
			}
		}
		allLedsOn = !allLedsOn;
	}
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void) {
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	// Enable Power Control clock
	__HAL_RCC_PWR_CLK_ENABLE();

	/* The voltage scaling allows optimizing the power consumption when the device is 
		clocked below the maximum system frequency, to update the voltage scaling value 
		regarding system frequency refer to product datasheet.  */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		// Initialization Error
		Error_Handler();
	}

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
		clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
	if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		// Initialization Error
		Error_Handler();
	}

	// STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported
	if (HAL_GetREVID() == 0x1001) {
		// Enable the Flash prefetch
		__HAL_FLASH_PREFETCH_BUFFER_ENABLE();
	}
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void) {
	/* User may add here some code to deal with this error */
	while(1) {}
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
	/* User can add his own implementation to report the file name and line number,
		ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1) {}
}

#endif

