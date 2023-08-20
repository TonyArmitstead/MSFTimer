/*
 * usb_endp.h
 *
 *  Created on: 9 Jan 2015
 *      Author: tony
 */

#ifndef USB_ENDP_H_
#define USB_ENDP_H_

#if defined __cplusplus
extern "C" {
#endif

uint32_t USBPutSerial(const uint8_t *ptrBuffer, uint32_t Send_length);
uint32_t USBGetSerial(uint8_t *ptrBuffer, uint32_t bufferLength);

#if defined __cplusplus
}
#endif

#endif /* USB_ENDP_H_ */
