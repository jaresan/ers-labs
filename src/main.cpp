#include "main.h"
#include <cstdio>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include "LED.h"
#include "PWMController.h"

static void SystemClock_Config(void);

static void Error_Handler(void);

int xPosition = -21;
int yPosition = -21;

LED redLed(GPIO_PIN_14);
LED blueLed(GPIO_PIN_15);

// Best + delay 10
float kP = 50;
float kI = 2;
float kD = 500;
int delay = 10;
PIDController pidY(kP, kI, kD);
PIDController pidX(kP, kI, kD);

PWMController pwm = PWMController();

int defaultInput[] = {
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
};

int pikachu[] = {440,180,220,110,170,250,190,200,370,280,120,150,360,270,290,220,270,170,180,150,150,180,330,110,200,210,130,180,280,280,370,260,200,230,210,220,340,170,180,120,110,180,330,220,170,170,280,290,160,340,190,230,350,200,370,300,310,270,310,110,210,170,120,160,380,340,320,290,230,160,170,280,260,200,250,220,250,230,390,130,270,240,360,150,320,300,370,270,350,220,340,230,250,280,150,130,180,160,110,160,280,100,410,170,420,150,340,200,380,200,210,210,350,280,380,220,290,210,440,170,160,210,250,200,280,240,360,160,430,170,400,140,370,120,320,160,160,220,180,220,100,180,160,170,370,250,360,210,290,280,350,230,270,100,240,100,160,200,260,240,310,280,330,170,430,160,290,230,330,250,330,210,380,240,400,180,170,290,380,230,270,290,170,260,380,320,310,230,170,300,350,210,420,180,260,280,270,200,220,160,200,200,200,220,340,210,160,130,420,170,270,280,100,170,280,200,300,260,380,170,250,100,160,320,110,170,250,210,350,270,170,180,120,180,260,100,230,110,170,270,160,240,420,160,180,210,170,120,260,290,210,110,160,230,320,110,430,180,340,120,360,220,360,120,380,210,290,250,190,120,300,230,260,190,140,140,170,190,320,240,370,180,370,190,290,240,290,200,130,170,190,220,310,160,130,140,280,190,160,310,350,120,340,260,380,310,190,210,200,120,160,330,390,180,300,100,180,270,380,330,410,180,380,130,200,170,370,170,140,180,340,220,370,290,410,140,270,190,120,170,290,100};

int* punchesBuffer = pikachu; // or defaultInput;
int punchesLen = sizeof(pikachu) / sizeof(int); // or sizeof(defaultInput);

int targetX = punchesBuffer[0];
int targetY = punchesBuffer[1];
int posIndex = 2;
bool end = false;

void set_new_target() {
    if (posIndex < punchesLen) {
        pidY.reset();
        pidX.reset();
        targetX = punchesBuffer[posIndex];
        targetY = punchesBuffer[posIndex + 1];
        posIndex += 2;
    } else {
        targetX = 0;
        targetY = 0;
        end = true;
    }
}

bool canMove = false;

void punch_hole() {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
    set_new_target();
}

extern void head_up() {
    canMove = true;
}

extern void left_border() {
    pwm.setSpeed(0, 50);
}

extern void top_border() {
    pwm.setSpeed(1, 50);
}

void init_hal() {
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
    // 00 01 11 10 | 00 01 11 10 | 00 01 11 10 | 00 01 11 10
    //             ^ here, when moving right, pos++; when moving left pos--
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

void init_puncher() {
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
}

int main(void) {
    init_hal();
    redLed.init();
    pwm.init();
    init_safety_and_encoders();
    init_puncher();

    blueLed.init();
    blueLed.on();

    // Go top left to search for 0,0
    pwm.setSpeed(0, -50);
    pwm.setSpeed(1, -50);

    // Min/max clamping
    pidX.setOutputRange(-100, 100);
    pidY.setOutputRange(-100, 100);

    // Wait until starting position found
    while (xPosition < -20 || yPosition < -20) { HAL_Delay(10); }

    int lastX = xPosition;
    int lastY = yPosition;

    while (true) {
        uint32_t lastTick = HAL_GetTick();
        bool shouldPunch = false;
        while (canMove) {
            uint32_t tick = HAL_GetTick();
            int currentX = xPosition - targetX;
            int currentY = yPosition - targetY;

            int outputX = (int) pidX.getOutput(currentX, 0);
            int outputY = (int) pidY.getOutput(currentY, 0);

            if (delay > 0) {
                HAL_Delay(delay);
            }

            pwm.setSpeed(0, outputX);
            pwm.setSpeed(1, outputY);

            if (lastX != xPosition || lastY != yPosition) {
                lastTick = tick;
            }

            // If there were no position changes for 100ms the motor isn't moving anymore -> can punch
            if (tick - lastTick > 100) {
                pwm.setSpeed(0, 0);
                pwm.setSpeed(1, 0);

                if (end) {
                    return 1;
                } else {
                    shouldPunch = true;
                    break;
                }
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
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        // Initialization Error
        Error_Handler();
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
        clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 |
                                   RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
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
    redLed.on();
    /* User may add here some code to deal with this error */
    while (1) {}
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

