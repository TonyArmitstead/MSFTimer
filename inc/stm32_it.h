/**
  ******************************************************************************
  * @file    stm32_it.h
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   This file contains the headers of the interrupt handlers.
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_IT_H
#define __STM32_IT_H

/* Includes ------------------------------------------------------------------*/
#include "platform_config.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

extern "C" void NMI_Handler(void);
extern "C" void HardFault_Handler(void);
extern "C" void MemManage_Handler(void);
extern "C" void BusFault_Handler(void);
extern "C" void UsageFault_Handler(void);
extern "C" void SVC_Handler(void);
extern "C" void DebugMon_Handler(void);
extern "C" void PendSV_Handler(void);
extern "C" void SysTick_Handler(void);
extern "C" void USB_LP_CAN1_RX0_IRQHandler(void);

#if defined (USE_STM32L152_EVAL) || (USE_STM32373C_EVAL)
void USART2_IRQHandler(void);
#else
extern "C" void USART1_IRQHandler(void);
#endif /* USE_STM32L152_EVAL */
#endif /* __STM32_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
