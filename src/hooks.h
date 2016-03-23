#ifdef __cplusplus
extern "C" {
#endif

void EXTI0_IRQHandler(void);
void SysTick_Handler(void);

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line);
#endif


#ifdef __cplusplus
}
#endif
