/**
 ******************************************************************************
 * @file    hw_config.c
 * @author  MCD Application Team
 * @version V4.0.0
 * @date    21-January-2013
 * @brief   Hardware Configuration & Setup
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include "stm32_it.h"
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "hw_config.h"
#include "usb_pwr.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ErrorStatus HSEStartUpStatus;
USART_InitTypeDef USART_InitStructure;
EXTI_InitTypeDef EXTI_InitStructure;

static void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len);
/* Extern variables ----------------------------------------------------------*/

extern LINE_CODING linecoding;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
 * Function Name  : Set_System
 * Description    : Configures Main system clocks & power
 * Input          : None.
 * Return         : None.
 *******************************************************************************/
void Set_System(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	/*!< At this stage the microcontroller clock setting is already configured,
	 this is done through SystemInit() function which is called from startup
	 file (startup_stm32f10x_xx.s) before to branch to application main.
	 To reconfigure the default setting of SystemInit() function, refer to
	 system_stm32f10x.c file
	 */
	/* Enable USB_DISCONNECT GPIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DISCONNECT, ENABLE);

	/* Configure USB pull-up pin */
	GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(USB_DISCONNECT, &GPIO_InitStructure);
	/* Configure the EXTI line 18 connected internally to the USB IP */
	EXTI_ClearITPendingBit(EXTI_Line18);
	EXTI_InitStructure.EXTI_Line = EXTI_Line18;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}

/*******************************************************************************
 * Function Name  : Set_USBClock
 * Description    : Configures USB Clock input (48MHz)
 * Input          : None.
 * Return         : None.
 *******************************************************************************/
void Set_USBClock(void) {
	/* Select USBCLK source */
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);

	/* Enable the USB clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}

/*******************************************************************************
 * Function Name  : Enter_LowPowerMode
 * Description    : Power-off system clocks and power while entering suspend mode
 * Input          : None.
 * Return         : None.
 *******************************************************************************/
void Enter_LowPowerMode(void) {
	/* Set the device state to suspend */
	USBDeviceState = SUSPENDED;
}

/*******************************************************************************
 * Function Name  : Leave_LowPowerMode
 * Description    : Restores system clocks and power while exiting suspend mode
 * Input          : None.
 * Return         : None.
 *******************************************************************************/
void Leave_LowPowerMode(void) {
	DEVICE_INFO *pInfo = &Device_Info;

	/* Set the device state to the correct state */
	if (pInfo->Current_Configuration != 0) {
		/* Device configured */
		USBDeviceState = CONFIGURED;
	} else {
		USBDeviceState = ATTACHED;
	}
	/*Enable SystemCoreClock*/
	SystemInit();
}

/*******************************************************************************
 * Function Name  : USB_Interrupts_Config
 * Description    : Configures the USB interrupts
 * Input          : None.
 * Return         : None.
 *******************************************************************************/
void USB_Interrupts_Config(void) {
	NVIC_InitTypeDef NVIC_InitStructure;
	wInterrupt_Mask = IMR_MSK;

	/* 2 bit for pre-emption priority, 2 bits for subpriority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	/* Enable the USB Wake-up interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USBWakeUp_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
	/* Enable USART Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EVAL_COM1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
 * Function Name  : USB_Cable_Config
 * Description    : Software Connection/Disconnection of USB Cable
 * Input          : None.
 * Return         : Status
 *******************************************************************************/
void USB_Cable_Config(FunctionalState NewState) {
	if (NewState != DISABLE) {
		GPIO_ResetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
	} else {
		GPIO_SetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
	}
}

/*******************************************************************************
 * Function Name  : Get_SerialNum.
 * Description    : Create the serial number string descriptor.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Get_SerialNum(void) {
	uint32_t Device_Serial0, Device_Serial1, Device_Serial2;

	Device_Serial0 = *(uint32_t*) ID1;
	Device_Serial1 = *(uint32_t*) ID2;
	Device_Serial2 = *(uint32_t*) ID3;

	Device_Serial0 += Device_Serial2;

	if (Device_Serial0 != 0) {
		IntToUnicode(Device_Serial0, &Virtual_Com_Port_StringSerial[2], 8);
		IntToUnicode(Device_Serial1, &Virtual_Com_Port_StringSerial[18], 4);
	}
}

/*******************************************************************************
 * Function Name  : HexToChar.
 * Description    : Convert Hex 32Bits value into char.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
static void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len) {
	uint8_t idx = 0;

	for (idx = 0; idx < len; idx++) {
		if (((value >> 28)) < 0xA) {
			pbuf[2 * idx] = (value >> 28) + '0';
		} else {
			pbuf[2 * idx] = (value >> 28) + 'A' - 10;
		}

		value = value << 4;

		pbuf[2 * idx + 1] = 0;
	}
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
