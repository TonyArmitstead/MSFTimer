/*
 * msf.h
 *
 *  Created on: 30 Oct 2014
 *      Author: tony
 */

#ifndef MSF_H_
#define MSF_H_
#include <stddef.h>
#include "msg.h"
#include "samplebuffer.h"
/*!
 * The format of a decoded MSF date/time record
 */
struct MSF_DATE_TIME {
	uint32_t ticksAtTime;
	int		DUT1;
	uint8_t	year;
	uint8_t	month;
	uint8_t	day;
	uint8_t	dayOfWeek;
	uint8_t hour;
	uint8_t min;
	bool    BST;
};
bool isMSFReceiverEnabled(void);
void enableMSFReceiver(void);
void disableMSFReceiver(void);
void configureMSFIO(void);
int msfSample(void);
bool msfPeriodLengthMatch(
	unsigned length,
	unsigned matchLength
);
bool decodeMSFSampleBuffer(
	struct MSF_SAMPLE_BUFFER* pSampleBuffer,
	struct MSF_DATE_TIME &dateTime,
	CMsg& decodeMsg
);
void formatMSFDateTime(
	const struct MSF_DATE_TIME& dateTime,
	CMsg& output
);
#endif /* MSF_H_ */
