#include "main.h"
#include <cstdio>
#include <cstdint>
#include "stm32f4xx_hal_iwdg.h"


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

UART uart;

bool pressRunning = false;

#define TIMx_IRQHandler                TIM3_IRQHandler

uint32_t uwPeriod = 0;
/* Pulses value */
uint32_t uwPulse1, uwPulse2 = 0;

/* Timer handler declaration */
TIM_HandleTypeDef    TimHandle;

/* Timer Output Compare Configuration Structure declaration */
TIM_OC_InitTypeDef sConfig;

void handleInfoButtonInterrupt(void*) {
	printf("BUTTON PRESSED!\n");

	if (pressRunning) {
        if (HAL_TIM_PWM_Stop(&TimHandle, TIM_CHANNEL_1) != HAL_OK) {
            /* Starting Error */
            Error_Handler();
        }

        /* Start channel 2 */
        if (HAL_TIM_PWM_Stop(&TimHandle, TIM_CHANNEL_2) != HAL_OK) {
            /* Starting Error */
            Error_Handler();
        }
    } else {
        /*##-3- Start PWM signals generation #######################################*/
        /* Start channel 1 */
        if(HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_1) != HAL_OK)
        {
            /* Starting Error */
            Error_Handler();
        }

        /* Start channel 2 */
        if(HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_2) != HAL_OK)
        {
            /* Starting Error */
            Error_Handler();
        }
	}

    pressRunning = !pressRunning;
}

extern void sysTickHookMain()
{
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim) {
    printf("OYE SENOR\n");
    GPIO_InitTypeDef GPIO_InitStruct;

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* TIM1 Peripheral clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();

    /* Enable GPIO Port Clocks */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*##-2- Configure I/Os #####################################################*/
    /* Configure PA.8 (TIM1_Channel1), PE.11 (TIM1_Channel2), PA.10 (TIM1_Channel3),
       PE.14 (TIM1_Channel4), PB.13 (TIM1_Channel1N), PB.14 (TIM1_Channel2N) &
       PB.15 (TIM1_Channel3N) in output, push-pull, alternate function mode */

    /* Common configuration for all channels */
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;

    /* Channel 1 output */
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Channel 2 complementary output */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void PWM_INIT()
{
    uwPeriod = (SystemCoreClock / 17570 ) - 1;

    //    uwPulse1 = (5 * (uwPeriod - 1)) / 10; -> 50%
    uwPulse1 = uwPeriod - 1;
    uwPulse2 = uwPeriod - 1;

    /*##-1- Configure the TIM peripheral #######################################*/
    /*----------------------------------------------------------------------------
     Generate 7 PWM signals with 4 different duty cycles:
     TIM1 input clock (TIM1CLK) is set to 2 * APB2 clock (PCLK2), since APB2
      prescaler is different from 1.
      TIM1CLK = 2 * PCLK2
      PCLK2 = HCLK / 2
      => TIM1CLK = 2 * (HCLK / 2) = HCLK = SystemCoreClock
     TIM1CLK = SystemCoreClock, Prescaler = 0, TIM1 counter clock = SystemCoreClock
     SystemCoreClock is set to 168 MHz for STM32F4xx devices

     The objective is to generate 7 PWM signal at 17.57 KHz:
       - TIM1_Period = (SystemCoreClock / 17570) - 1
     The channel 1 and channel 1N duty cycle is set to 50%
     The channel 2 and channel 2N duty cycle is set to 37.5%
     The channel 3 and channel 3N duty cycle is set to 25%
     The channel 4 duty cycle is set to 12.5%
     The Timer pulse is calculated as follows:
       - ChannelxPulse = DutyCycle * (TIM1_Period - 1) / 100

     Note:
      SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f4xx.c file.
      Each time the core clock (HCLK) changes, user had to call SystemCoreClockUpdate()
      function to update SystemCoreClock variable value. Otherwise, any configuration
      based on this variable will be incorrect.
    ----------------------------------------------------------------------- */

    /* Initialize TIMx peripheral as follow:
         + Prescaler = 0
         + Period = uwPeriod  (to have an output frequency equal to 17.57 KHz)
         + ClockDivision = 0
         + Counter direction = Up
    */
    TimHandle.Instance = TIM4;

    TimHandle.Init.Period            = uwPeriod;
    TimHandle.Init.Prescaler         = 0;
    TimHandle.Init.ClockDivision     = 0;
    TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    TimHandle.Init.RepetitionCounter = 0;

    if(HAL_TIM_PWM_Init(&TimHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }

    /*##-2- Configure the PWM channels #########################################*/
    /* Common configuration for all channels */
    sConfig.OCMode      = TIM_OCMODE_PWM2;
    sConfig.OCFastMode  = TIM_OCFAST_DISABLE;
    sConfig.OCPolarity  = TIM_OCPOLARITY_LOW;
    sConfig.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfig.OCIdleState = TIM_OCIDLESTATE_SET;
    sConfig.OCNIdleState= TIM_OCNIDLESTATE_RESET;

    /* Set the pulse value for channel 1 */
    sConfig.Pulse = uwPulse1;

    if(HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_1) != HAL_OK)
    {
        /* Configuration Error */
        Error_Handler();
    }

    /* Set the pulse value for channel 2 */
    sConfig.Pulse = uwPulse2;
    if(HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_2) != HAL_OK)
    {
        /* Configuration Error */
        Error_Handler();
    }

    /*##-3- Start PWM signals generation #######################################*/
    /* Start channel 1 */
    if(HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_1) != HAL_OK)
    {
        /* Starting Error */
        Error_Handler();
    }

    /* Start channel 2 */
    if(HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_2) != HAL_OK)
    {
        /* Starting Error */
        Error_Handler();
    }

    HAL_NVIC_SetPriority(TIM4_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);

    pressRunning = true;
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
	greenLed.init();
	redLed.init();
	blueLed.init();
	orangeLed.init();
    infoButton.setPriority(2, 1);
	infoButton.init();
	infoButton.setPressedListener(handleInfoButtonInterrupt, nullptr);

    blueLed.on();
    PWM_INIT();
//    TimerInit();
//	initTimer(500);

	// Infinite loop
	while (1) {}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	printf("Period elapsed\n");
    greenLed.toggle();
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
	redLed.on();
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

