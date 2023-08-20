/*
 * stm3210b_lctech.cpp
 */

/* Includes ------------------------------------------------------------------*/
#include "stm3210b_lctech.h"

/** @addtogroup Utilities
 * @{
 */

/** @addtogroup STM32_EVAL
 * @{
 */

/** @addtogroup STM3210B_EVAL
 * @{
 */

/** @defgroup STM3210B_EVAL_LOW_LEVEL
 * @brief This file provides firmware functions to manage Leds, push-buttons,
 *        COM ports, SD card on SPI, serial flash (sFLASH), serial EEPROM (sEE)
 *        and temperature sensor (LM75) available on STM3210B-EVAL evaluation
 *        board from STMicroelectronics.
 * @{
 */

/** @defgroup STM3210B_EVAL_LOW_LEVEL_Private_TypesDefinitions
 * @{
 */
/**
 * @}
 */

/** @defgroup STM3210B_EVAL_LOW_LEVEL_Private_Defines
 * @{
 */
/**
 * @}
 */

/** @defgroup STM3210B_EVAL_LOW_LEVEL_Private_Macros
 * @{
 */
/**
 * @}
 */

/** @defgroup STM3210B_EVAL_LOW_LEVEL_Private_Variables
 * @{
 */
USART_TypeDef* COM_USART[COMn] = { EVAL_COM1, EVAL_COM2 };

GPIO_TypeDef* COM_TX_PORT[COMn] = { EVAL_COM1_TX_GPIO_PORT,
		EVAL_COM2_TX_GPIO_PORT };

GPIO_TypeDef* COM_RX_PORT[COMn] = { EVAL_COM1_RX_GPIO_PORT,
		EVAL_COM2_RX_GPIO_PORT };

const uint32_t COM_USART_CLK[COMn] = { EVAL_COM1_CLK, EVAL_COM2_CLK };

const uint32_t COM_TX_PORT_CLK[COMn] = { EVAL_COM1_TX_GPIO_CLK,
		EVAL_COM2_TX_GPIO_CLK };

const uint32_t COM_RX_PORT_CLK[COMn] = { EVAL_COM1_RX_GPIO_CLK,
		EVAL_COM2_RX_GPIO_CLK };

const uint16_t COM_TX_PIN[COMn] = { EVAL_COM1_TX_PIN, EVAL_COM2_TX_PIN };

const uint16_t COM_RX_PIN[COMn] = { EVAL_COM1_RX_PIN, EVAL_COM2_RX_PIN };

/**
 * @}
 */

/** @defgroup STM3210B_EVAL_LOW_LEVEL_Private_FunctionPrototypes
 * @{
 */

/**
 * @}
 */

/** @defgroup STM3210B_EVAL_LOW_LEVEL_Private_Functions
 * @{
 */

/**
 * @brief  Configures LED GPIO.
 * @param  Led: Specifies the Led to be configured.
 *   This parameter can be one of following parameters:
 *     @arg LED1
 *     @arg LED2
 *     @arg LED3
 *     @arg LED4
 * @retval None
 */
void STM_EVAL_LEDInit(Led_TypeDef Led) {
}

/**
 * @brief  Turns selected LED On.
 * @param  Led: Specifies the Led to be set on.
 *   This parameter can be one of following parameters:
 *     @arg LED1
 *     @arg LED2
 *     @arg LED3
 *     @arg LED4
 * @retval None
 */
void STM_EVAL_LEDOn(Led_TypeDef Led) {
}

/**
 * @brief  Turns selected LED Off.
 * @param  Led: Specifies the Led to be set off.
 *   This parameter can be one of following parameters:
 *     @arg LED1
 *     @arg LED2
 *     @arg LED3
 *     @arg LED4
 * @retval None
 */
void STM_EVAL_LEDOff(Led_TypeDef Led) {
}

/**
 * @brief  Toggles the selected LED.
 * @param  Led: Specifies the Led to be toggled.
 *   This parameter can be one of following parameters:
 *     @arg LED1
 *     @arg LED2
 *     @arg LED3
 *     @arg LED4
 * @retval None
 */
void STM_EVAL_LEDToggle(Led_TypeDef Led) {
}

/**
 * @brief  Configures Button GPIO and EXTI Line.
 * @param  Button: Specifies the Button to be configured.
 *   This parameter can be one of following parameters:
 *     @arg BUTTON_WAKEUP: Wakeup Push Button
 *     @arg BUTTON_TAMPER: Tamper Push Button
 *     @arg BUTTON_KEY: Key Push Button
 *     @arg BUTTON_RIGHT: Joystick Right Push Button
 *     @arg BUTTON_LEFT: Joystick Left Push Button
 *     @arg BUTTON_UP: Joystick Up Push Button
 *     @arg BUTTON_DOWN: Joystick Down Push Button
 *     @arg BUTTON_SEL: Joystick Sel Push Button
 * @param  Button_Mode: Specifies Button mode.
 *   This parameter can be one of following parameters:
 *     @arg BUTTON_MODE_GPIO: Button will be used as simple IO
 *     @arg BUTTON_MODE_EXTI: Button will be connected to EXTI line with interrupt
 *                     generation capability
 * @retval None
 */
void STM_EVAL_PBInit(Button_TypeDef Button, ButtonMode_TypeDef Button_Mode) {
}

/**
 * @brief  Returns the selected Button state.
 * @param  Button: Specifies the Button to be checked.
 *   This parameter can be one of following parameters:
 *     @arg BUTTON_WAKEUP: Wakeup Push Button
 *     @arg BUTTON_TAMPER: Tamper Push Button
 *     @arg BUTTON_KEY: Key Push Button
 *     @arg BUTTON_RIGHT: Joystick Right Push Button
 *     @arg BUTTON_LEFT: Joystick Left Push Button
 *     @arg BUTTON_UP: Joystick Up Push Button
 *     @arg BUTTON_DOWN: Joystick Down Push Button
 *     @arg BUTTON_SEL: Joystick Sel Push Button
 * @retval The Button GPIO pin value.
 */
uint32_t STM_EVAL_PBGetState(Button_TypeDef Button) {
	return 0;
}

/**
 * @brief  Configures COM port.
 * @param  COM: Specifies the COM port to be configured.
 *   This parameter can be one of following parameters:
 *     @arg COM1
 *     @arg COM2
 * @param  USART_InitStruct: pointer to a USART_InitTypeDef structure that
 *   contains the configuration information for the specified USART peripheral.
 * @retval None
 */
void STM_EVAL_COMInit(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct) {
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIO clock */
	RCC_APB2PeriphClockCmd(
			COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM] | RCC_APB2Periph_AFIO,
			ENABLE);

	if (COM == COM1) {
		RCC_APB2PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
	} else {
		/* Enable the USART2 Pins Software Remapping */
		GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
		RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
	}

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);

	/* Configure USART Rx as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
	GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);

	/* USART configuration */
	USART_Init(COM_USART[COM], USART_InitStruct);

	/* Enable USART */
	USART_Cmd(COM_USART[COM], ENABLE);
}

/**
 * @brief  DeInitializes the SD/SD communication.
 * @param  None
 * @retval None
 */
void SD_LowLevel_DeInit(void) {
}

/**
 * @brief  Initializes the SD_SPI and CS pins.
 * @param  None
 * @retval None
 */
void SD_LowLevel_Init(void) {
}

/**
 * @brief  DeInitializes the peripherals used by the SPI FLASH driver.
 * @param  None
 * @retval None
 */
void sFLASH_LowLevel_DeInit(void) {
}

/**
 * @brief  Initializes the peripherals used by the SPI FLASH driver.
 * @param  None
 * @retval None
 */
void sFLASH_LowLevel_Init(void) {
}

/**
 * @brief  DeInitializes the LM75_I2C.
 * @param  None
 * @retval None
 */
void LM75_LowLevel_DeInit(void) {
}

/**
 * @brief  Initializes the LM75_I2C.
 * @param  None
 * @retval None
 */
void LM75_LowLevel_Init(void) {
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

