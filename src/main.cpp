#include "main.h"
#include <cstdio>
#include <cstdint>
#include "stm32f4xx_hal_iwdg.h"
#include <stdio.h>
#include <stdlib.h>


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

uint32_t uwPeriod = 0;
/* Pulses value */
uint32_t xPulse, yPulse = 0;

/* Timer handler declaration */
TIM_HandleTypeDef    TimHandle;

int xPosition = -21;
int yPosition = -21;

/* Timer Output Compare Configuration Structure declaration */
TIM_OC_InitTypeDef sConfig;


// Best + delay 10
float kP = 50;
float kI = 2;
float kD = 500;
int delay = 10;

PIDController pidY(kP, kI, kD);
PIDController pidX(kP, kI, kD);


void setSpeed(uint8_t dir, float speed)
{
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
//    printf("Speed: %d, Reset: %d, Pulse: %d, max: %d \n", (int)speed, reset, pulse, uwPeriod);
}

int speed = 0;

int pos[] = {
        150, 60,
        170, 60,
        190, 60,
        130, 60,
        210, 100,
        210, 120,
        210, 140,
        210, 80,
        110, 100,
        110, 120,
        110, 140,
        110, 80,
        150, 160,
        170, 160,
        190, 160,
        130, 160,
        10, 120,
        10, 140,
        10, 80,
        10, 100,
        10, 200,
        10, 150,
        10, 130,
        10, 110,
};

int* posBuff = pos;
int posLen = 48;

int targetX = posBuff[0];
int targetY = posBuff[1];
int posIndex = 2;
bool end = false;

void handleInfoButtonInterrupt(void*) {
    pidY.reset();
    pidX.reset();
    targetX = posBuff[posIndex];
    targetY = posBuff[posIndex + 1];
    posIndex += 2;

    pressRunning = !pressRunning;
}

void set_new_target() {
    if (posIndex < posLen) {
        pidY.reset();
        pidX.reset();
        targetX = posBuff[posIndex];
        targetY = posBuff[posIndex + 1];
        posIndex += 2;
    } else {
        targetX = 0;
        targetY = 0;
        end = true;
    }
}

bool canMove = false;

void punch_hole() {
//    printf("Punching.. Can I? %d\n", HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_11));
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
    HAL_Delay(20);
//    printf("Resetting..\n");
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
    set_new_target();
    HAL_Delay(20);
}

extern void head_up() {
    canMove = true;
}

extern void left_border() {
    setSpeed(0, 50);
}

extern void top_border() {
    setSpeed(1, 50);
}

extern void sysTickHookMain() {
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim) {
    GPIO_InitTypeDef GPIO_InitStruct;

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* TIM1 Peripheral clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();

    /* Enable GPIO Port Clocks */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*##-2- Configure I/Os #####################################################*/
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;

    /* Channel 1 output */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void init_pwm() {
    // Set PWM dir pins as output
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_Init;
    GPIO_Init.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init.Pull = GPIO_NOPULL;
    GPIO_Init.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_Init);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_SET);
    //

    uwPeriod = (SystemCoreClock / 17570) - 1;

    //    xPulse = uwPeriod / 2  - 1; -> 50%
    xPulse = 0;
    yPulse = 0;

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
    sConfig.Pulse = xPulse;

    if(HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_1) != HAL_OK)
    {
        /* Configuration Error */
        Error_Handler();
    }

    /* Set the pulse value for channel 2 */
    sConfig.Pulse = yPulse;
    if(HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_2) != HAL_OK)
    {
        /* Configuration Error */
        Error_Handler();
    }

    /*##-3- Start PWM signals generation #######################################*/
    /* Start channel 1 & 2 */
//    if(HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_1) != HAL_OK ||
//        HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_2) != HAL_OK)
//    {
//        /* Starting Error */
//        Error_Handler();
//    }

//    HAL_NVIC_SetPriority(TIM4_IRQn, 3, 0);
//    HAL_NVIC_EnableIRQ(TIM4_IRQn);
    pressRunning = true;
}

void init_hal_and_leds() {
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
    infoButton.setPriority(3, 0);
    infoButton.init();
    infoButton.setPressedListener(handleInfoButtonInterrupt, nullptr);

    blueLed.on();
}

void init_safety_and_encoders() {
    GPIO_InitTypeDef gpioInitStruct;

    // Safety borders
    gpioInitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
    gpioInitStruct.Mode = GPIO_MODE_IT_RISING;
    gpioInitStruct.Pull = GPIO_NOPULL;
    gpioInitStruct.Speed = GPIO_SPEED_HIGH;

    HAL_GPIO_Init(GPIOC, &gpioInitStruct);

    // Configure encoder interrupts
    gpioInitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    // Enc X and enc Y -> we care only about the border between 10 and 00
    gpioInitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_6;
    HAL_GPIO_Init(GPIOC, &gpioInitStruct);


    // Configure interrupt priorities
    HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

int main(void) {
    init_hal_and_leds();
    init_safety_and_encoders();
    init_pwm();

    // Punch & Head_UP
    GPIO_InitTypeDef GPIO_Init;
    GPIO_Init.Pin = GPIO_PIN_2;
    GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init.Pull = GPIO_NOPULL;
    GPIO_Init.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_11;
    GPIO_Init.Mode = GPIO_MODE_IT_RISING;
    GPIO_Init.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_Init);


    setSpeed(0, -100);
    setSpeed(1, -100);

    // Min/max clamping
    pidX.setOutputRange(-100, 100);
    pidY.setOutputRange(-100, 100);

    while (xPosition < -20 || yPosition < -20) { HAL_Delay(10); }

    int lastX = xPosition;
    int lastY = yPosition;

    while (true) {
        int currentX = xPosition - targetX;
        int currentY = yPosition - targetY;
        uint32_t lastTick = HAL_GetTick();
        bool shouldPunch = false;
        while(canMove){
            uint32_t tick = HAL_GetTick();
            currentX = xPosition - targetX;
            currentY = yPosition - targetY;

            int outputX=(int)pidX.getOutput(currentX,0);
            int outputY=(int)pidY.getOutput(currentY,0);

            if (delay > 0) {
                HAL_Delay(delay);
            }

            setSpeed(0, outputX);
            setSpeed(1, outputY);

            if (lastX != xPosition || lastY != yPosition) {
                lastTick = tick;
            }

            // If there were no position changes for 150ms the motor won't be moving anymore -> can punch
            if (tick - lastTick > 100) {
//                printf("Tick: %d, last: %d\n", tick, lastTick);
                setSpeed(0, 0);
                setSpeed(1, 0);
                shouldPunch = !end;
                break;
            }

            lastX = xPosition;
            lastY = yPosition;
        }

        if (shouldPunch) {
            canMove = false;
            punch_hole();
        }
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

