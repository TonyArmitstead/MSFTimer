/*!
 * @file    usb_endp.c
 */

#include <algorithm>
#include <cstring>
#include "usb_lib.h"
#include "usb_mem.h"
#include "usb_desc.h"
#include "hw_config.h"
#include "usb_istr.h"
#include "usb_pwr.h"
#include "usb_endp.h"

#define USB_TX_BUFF_SIZE   2048
#define USB_RX_BUFF_SIZE   1024
/* Interval between sending IN packets in frame number (1 frame = 1ms) */
#define VCOMPORT_IN_FRAME_INTERVAL 5
/* USB Receive buffer - data received _from_ the USB port */
static uint8_t  USB_RxBuffer[USB_TX_BUFF_SIZE];
static uint32_t USB_RxWrIdx = 0;
static uint32_t USB_RxRdIdx = 0;
static uint32_t USB_RxLength = 0;
static uint8_t  USB_RxHoldingBuffer[VIRTUAL_COM_PORT_DATA_SIZE];
/* USB Transmit buffer - data to be sent _to_ the USB port */
static uint8_t  USB_TxBuffer[USB_TX_BUFF_SIZE];
static uint32_t USB_TxWrIdx = 0;
static uint32_t USB_TxRdIdx = 0;
static uint32_t USB_TxLength = 0;
static uint8_t  USB_TxHoldingBuffer[VIRTUAL_COM_PORT_DATA_SIZE];

/*!
 * Function Name  : USBAsynchSend.
 * Description    : sends a block of data to USB end point 1
 */
static void USBAsynchSend(void) {
    if (USB_TxLength != 0) {
        uint32_t txSize = std::min(USB_TxLength,
                                   (uint32_t)VIRTUAL_COM_PORT_DATA_SIZE);
        uint32_t b1Size = std::min(txSize,
                                   (uint32_t)(USB_TX_BUFF_SIZE-USB_TxRdIdx));
        uint32_t b2Size = txSize - b1Size;
        if (b1Size > 0) {
            std::memcpy(USB_TxHoldingBuffer, USB_TxBuffer+USB_TxRdIdx, b1Size);
            if (b2Size > 0) {
                std::memcpy(USB_TxHoldingBuffer+b1Size, USB_TxBuffer, b2Size);
                USB_TxRdIdx = b2Size;
            } else {
                USB_TxRdIdx += b1Size;
                if (USB_TxRdIdx == USB_TX_BUFF_SIZE) {
                    USB_TxRdIdx = 0;
                }
            }
            UserToPMABufferCopy(USB_TxHoldingBuffer, ENDP1_TXADDR, txSize);
            USB_TxLength -= txSize;
            SetEPTxCount(ENDP1, txSize);
            SetEPTxValid(ENDP1);
        }
    }
}

/*!
 * Function Name  : USBSendSerial
 * Description    : Sends a block of data to the USB serial port buffer
 * Input          : pBuffer - pointer to data to send
 *  				length - number of bytes to send
 * Return         : The number of bytes sent [0..length]
 */
extern "C"
uint32_t USBPutSerial(const uint8_t *pBuffer, uint32_t length) {
	uint32_t availLen = std::min(length, USB_TX_BUFF_SIZE-USB_TxLength);
	uint32_t b1Size = std::min(availLen, USB_TX_BUFF_SIZE-USB_TxWrIdx);
	uint32_t b2Size = availLen - b1Size;
	if (b1Size > 0) {
		std::memcpy(USB_TxBuffer+USB_TxWrIdx, pBuffer, b1Size);
		if (b2Size > 0) {
			std::memcpy(USB_TxBuffer, pBuffer+b1Size, b2Size);
			USB_TxWrIdx = b2Size;
		} else {
			USB_TxWrIdx += b1Size;
			if (USB_TxWrIdx == USB_TX_BUFF_SIZE) {
				USB_TxWrIdx = 0;
			}
		}
	    NVIC_DisableIRQ (USB_LP_CAN1_RX0_IRQn);
	    USB_TxLength += availLen;
	    NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
	}
	return availLen;
}

/*!
 * Function Name  : USBGetSerial
 * Description    : Gets a block of data from the USB serial port buffer
 * Input          : pBuffer - pointer to receive data
 * 				    length - size of receive buffer
 * Return         : The number of bytes read [0..length]
 */
extern "C"
uint32_t USBGetSerial(uint8_t *pBuffer, uint32_t bufferLength) {
	uint32_t availLen = std::min(bufferLength, USB_RxLength);
	uint32_t b1Size = std::min(availLen, USB_RX_BUFF_SIZE-USB_RxRdIdx);
	uint32_t b2Size = availLen - b1Size;
	if (b1Size > 0) {
		std::memcpy(pBuffer, USB_RxBuffer+USB_RxRdIdx, b1Size);
		if (b2Size > 0) {
			std::memcpy(pBuffer+b1Size, USB_RxBuffer, b2Size);
			USB_RxRdIdx = b2Size;
		} else {
			USB_RxRdIdx += b1Size;
			if (USB_RxRdIdx == USB_RX_BUFF_SIZE) {
				USB_RxRdIdx = 0;
			}
		}
	    NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
	    USB_RxLength -= availLen;
	    NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
	}
	return availLen;
}

/*!
 * Function Name  : EP1_IN_Callback
 * Description    : USB End point 1 IN processing - host PC wants data _from_
 * 					us
 */
void EP1_IN_Callback(void) {
	USBAsynchSend();
}

/*!
 * Function Name  : EP3_OUT_Callback
 * Description    : USB End point 3 OUT processing - data has arrived
 *                  for us _from_ the host PC.
 */
void EP3_OUT_Callback(void) {
	/* Extract received data */
	uint32_t rxCount = USB_SIL_Read(EP3_OUT, USB_RxHoldingBuffer);
	/* Enable the receive of data on EP3 */
	SetEPRxValid(ENDP3);
	/* Now move into USB_RxBuffer */
	uint32_t availLen = std::min(rxCount,
						    USB_RX_BUFF_SIZE-USB_RxLength);
	uint32_t b1Size = std::min(availLen, USB_RX_BUFF_SIZE-USB_RxWrIdx);
	uint32_t b2Size = availLen - b1Size;
	if (b1Size > 0) {
		std::memcpy(USB_RxBuffer+USB_RxWrIdx, USB_RxHoldingBuffer, b1Size);
		if (b2Size > 0) {
			std::memcpy(USB_RxBuffer, USB_RxHoldingBuffer+b1Size, b2Size);
			USB_RxWrIdx = b2Size;
		} else {
			USB_RxWrIdx += b1Size;
			if (USB_RxWrIdx == USB_TX_BUFF_SIZE) {
				USB_RxWrIdx = 0;
			}
		}
	    USB_RxLength += availLen;
	}
}

/*!
 * Function Name  : SOF_Callback / INTR_SOFINTR_Callback
 * Description    :
 */
void SOF_Callback(void) {
	static uint32_t FrameCount = 0;
	if (USBDeviceState == CONFIGURED) {
		if (FrameCount++ == VCOMPORT_IN_FRAME_INTERVAL) {
			/* Reset the frame counter */
			FrameCount = 0;
			/* Check the data to be sent through IN pipe */
			USBAsynchSend();
		}
	}
}


