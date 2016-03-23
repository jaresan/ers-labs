#include "hooks.h"
#include "main.h"

void SysTick_Handler(void) {
	PulseLED::tickInterruptHandler();
	sysTickHookMain();
}

void EXTI0_IRQHandler(void) {
	infoButton.pressedInterruptHandler();
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
	printf("Wrong parameters value: file %s on line %d\r\n", file, line);
	for(;;);
}
#endif

