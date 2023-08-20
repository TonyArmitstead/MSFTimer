/*
 * msf.cpp
 *
 * Code to decode MSF sample data into a date.time value
 *
 */
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "msf.h"
#include "systick.h"
#include "msg.h"

#define A(X) (ABits[(X)-1])
#define B(X) (BBits[(X)-1])

/*!
 * System ticker time values for the 100..900ms intervals
 */
const int ms100 = 100*SYSTICK_ONESEC/1000;
const int ms200 = 200*SYSTICK_ONESEC/1000;
const int ms300 = 300*SYSTICK_ONESEC/1000;
const int ms700 = 700*SYSTICK_ONESEC/1000;
const int ms800 = 800*SYSTICK_ONESEC/1000;
const int ms900 = 900*SYSTICK_ONESEC/1000;

/*!
 * Indicates if the MSF receiver module has been enabled
 * @return true if enabled, false if not
 */
bool isMSFReceiverEnabled(void) {
	if (GPIOB->ODR & (1 << 1))
		return false;
	return true;
}

/*!
 * Enables the MSF receiver module
 */
void enableMSFReceiver(void) {
	GPIOB->BRR = (1 << 1);
}

/*!
 * Disables the MSF receiver module
 */
void disableMSFReceiver(void) {
	GPIOB->BSRR = (1 << 1);
}

/*!
 * Configures the STM32 IO to interface to the MSF receiver module.
 * PB0 = MSF receiver data output (input to us)
 * PB1 = MSF receiver enable (output from us)
 * PB2 = Output from us toggled each time we sample the MSF data
 */
void configureMSFIO(void) {
	/* Enable GPIOB Clock */
	RCC->APB2ENR |= (1UL << 3);
	/*
	 * PortB0 = Data      (Input)
	 *    1000 = Input with pull up/pull down
	 * PortB1 = /ENABLE   (Output, 0 to enable, 1 to disable)
	 *    0010 = Output max speed 2MHz, push/pull output
	 * PortB2 = /ENABLE   (Output, 0 to enable, 1 to disable)
	 *    0010 = Output max speed 2MHz, push/pull output
	 */
	GPIOB->CRL = (GPIOB->CRL & 0xFFFFF000) | 0x00000228;
}

/*!
 * Returns current MSF bit level
 * \return 0/1
 */
int msfSample(void) {
	/*
	 * Toggle the sample indicator to generate a signal
	 * output at 1/2 our sampling frequency.
	 */
	GPIOB->ODR = GPIOB->ODR ^ (1 << 2);
	/* The input is inverted */
	return (GPIOB->IDR & 1) ? 0 : 1;
}

/*!
 * Prints out the bit periods found in a MSF sample buffer
 * \param pSampleBuffer points to sample buffer containing the bit periods
 */
static void showMSFBitPeriods(
	const struct MSF_SAMPLE_BUFFER* pSampleBuffer,
    CMsg& decodeMsg
) {
	int level = 0;
    char messageBuff[128];
	if (pSampleBuffer->isEmpty()) {
		decodeMsg.append("MSF Bit Period buffer is empty");
	} else {
		decodeMsg.append("MSF Bit Periods: ");
		const uint8_t* pRPtr = pSampleBuffer->sampleData;
		uint32_t sampleIdx = 0;
		while (pRPtr != pSampleBuffer->pWPtr) {
		    snprintf(
		        messageBuff, sizeof(messageBuff),
	            "%u:%d:%u", sampleIdx++, level, *pRPtr);
	        decodeMsg.append(messageBuff, "|");
			level = 1 - level;
			++pRPtr;
		}
	}
}

/*!
 * Indicates if the period length matches the required length within
 * an allowed delta. Right now this does a test within absolute delta
 * limits - not relative ones.
 * \param length period length to test
 * \param matchLength the length we are looking to match against
 * \return true if lengths match, false if not
 */
bool msfPeriodLengthMatch(
	unsigned length,
	unsigned matchLength
) {
	const unsigned LONG_MATCH_DELTA = 5;
	const unsigned SHORT_MATCH_DELTA = 5;
	unsigned minMatch = (matchLength >= SHORT_MATCH_DELTA) ?
							(matchLength - SHORT_MATCH_DELTA) : 0;
	unsigned maxMatch = matchLength + LONG_MATCH_DELTA;
	return ((minMatch <= length) && (length <= maxMatch));
}

/*!
 * Calculates an error score for how well two period match two target periods.
 * If the match is exact [(p1 == m1) and (p2 == m2) _and_ the two bit periods
 * add up to 1000ms] the returned value is 0 - the minimum error (maximum match).
 * As the p values move away from the m values the error score will increase.
 * @param p1 the test period 1
 * @param p2 the test period 2
 * @param m1 the target period 1
 * @param m2 the target period 2
 */
static uint32_t msfPeriodMatch2Periods(
    int p1,
    int p2,
    int m1,
    int m2
) {
    int pcMatch = 0;
    pcMatch += abs(1000 - 1000*p1/m1);
    pcMatch += abs(1000 - 1000*p2/m2);
    pcMatch += 10*abs(100 - p1 - p2);
    return (uint32_t)(pcMatch/3);
}

/*!
 * Calculates an error score for how well two period match 300ms and 700ms.
 * If the match is exact [(p300 == 300ms) and (p700 == 700ms) _and_ the two bit
 * periods add up to 1000ms] the returned value is 0 - the minimum error
 * (maximum match). As the p values move away from the 300/700 ms values the
 * error score will increase.
 * @param p300 the 300ms test period
 * @param p700 the 700ms test period
 */
static uint32_t msfPeriodMatch_300_700(
    uint8_t p300,
    uint8_t p700
) {
    return msfPeriodMatch2Periods(p300, p700, ms300, ms700);
}

/*!
 * Calculates an error score for how well two period match 100ms and 900ms.
 * If the match is exact [(p100 == 100ms) and (p900 == 900ms) _and_ the two bit
 * periods add up to 1000ms] the returned value is 0 - the minimum error
 * (maximum match). As the p values move away from the 100/900 ms values the
 * error score will increase.
 * @param p100 the 100ms test period
 * @param p900 the 900ms test period
 */
static uint32_t msfPeriodMatch_100_900(
    uint8_t p100,
    uint8_t p900
) {
    return msfPeriodMatch2Periods(p100, p900, ms100, ms900);
}

/*!
 * Calculates an error score for how well two period match 200ms and 800ms.
 * If the match is exact [(p200 == 200ms) and (p800 == 800ms) _and_ the two bit
 * periods add up to 1000ms] the returned value is 0 - the minimum error
 * (maximum match). As the p values move away from the 200/800 ms values the
 * error score will increase.
 * @param p200 the 200ms test period
 * @param p800 the 800ms test period
 */
static uint32_t msfPeriodMatch_200_800(
    uint8_t p200,
    uint8_t p800
) {
    return msfPeriodMatch2Periods(p200, p800, ms200, ms800);
}

/*!
 * Calculates an error score for how well four period match 100,100,100,700ms.
 * If the match is exact [(p100_1 == 100ms) and (p100_2 == 100ms) and
 * (p100_3=100ms) _and_ p100_1+p100_2+p100_3+p700 = 1000ms] the returned value
 * is 0 - the minimum error (maximum match). As the values move away from the
 * 100/100/100/700 ms values the error score will increase.
 * @param p100_1 the 100ms test period
 * @param p100_2 the 100ms test period
 * @param p100_3 the 100ms test period
 * @param p700 the 700ms test period
 */
static uint32_t msfPeriodMatch_100_100_100_700(
    int p100_1,
    int p100_2,
    int p100_3,
    int p700
) {
    int pcMatch = 0;
    pcMatch += abs(1000 - 1000*p100_1/ms100);
    pcMatch += abs(1000 - 1000*p100_2/ms100);
    pcMatch += abs(1000 - 1000*p100_3/ms100);
    pcMatch += abs(1000 - 1000*p700/ms700);
    pcMatch += 10*abs(100 - p100_1 - p100_2 - p100_3 - p700);
    return (uint32_t)(pcMatch/5);
}

/*!
 * Indicates if a test value is an acceptable error and is the minimum error
 * in a set of 3.
 * @param test the value to test for being the minimum acceptable error
 * @param e1 one of the 3 comparator error values
 * @param e2 one of the 3 comparator error values
 * @param e3 one of the 3 comparator error values
 * @return true if test is an acceptable error value and is smaller than
 *         any in the set {e1,e2,e3}
 */
static bool isBestError(
	uint32_t test,
	uint32_t e1,
	uint32_t e2,
	uint32_t e3
) {
    bool rCode = false;
    if (test < 300) {
        if (test < e1) {
            if (test < e2) {
                if (test < e3) {
                    rCode = true;
                }
            }
        }
    }
    return rCode;
}

/*!
 * Extracts the A,B bit sets from the bit periods data set
 * \param pSampleBuffer the bit period data set we work on
 * \param ABits array of A bit values which we assign
 * \param BBits array of B bit values which we assign
 * \param secsCount assigned the number of seconds entries we
 *        extracted. If we fail, this is set to the seconds
 *        entry that we failed at. Note that if there is no
 *        data in the sample buffer we return false with
 *        secsCount set to 0.
 * \return true if full set of A/B bits assigned, false if failed
 *
 *  +0   +100 +200 +300 +400 +500 +600 +700 +800 +900 +1000  ms
 *   +----+----+----+----+----+----+----+----+----+----+
 *         ____ ____ ___________________________________
 * x|_____|_Ax_|_Bx_|
 *
 * If the A/B bit period is 0 the bit value is 1
 * If the A/B bit period is 1 the bit value is 0
 * The period data started after the first second marker.
 * For each sampled sec period x, we will get one of:
 *   0:300, 1:700                => Ax = 1, Bx = 1
 *   0:200, 1:800,               => Ax = 1, Bx = 0
 *   0:100, 1:100, 0:100, 1:700  => Ax = 0, Bx = 1
 *   0:100, 1:900                => Ax = 0, Bx = 0
 * any other pattern is invalid.
 * We can see from the above that we should get 2 or 4 periods
 * per second from which we can deduce the A/B bit values.
 */
static bool extractABBits(
	struct MSF_SAMPLE_BUFFER* pSampleBuffer,
	uint8_t ABits[],
	uint8_t BBits[],
	size_t& secsCount,
    CMsg& decodeMsg
) {
    char messageBuff[128];
    bool rCode = true;
	secsCount = 0;
	if (pSampleBuffer->isEmpty()) {
        decodeMsg.append("extractABBits failed: sample buffer is empty", 0);
		rCode = false;
	} else {
		bool allDone = false;
		pSampleBuffer->resetRead();
		//   v(pRPtr)          v(pSampleBuffer->pWPtr)
		// x x x x x x x x x x
		// while we have at least 2 samples to process ...
		while (!allDone && rCode) {
            // Inspect first 2 periods
			size_t startOffset = pSampleBuffer->getReadOffset();
            uint8_t bitP0;
            uint8_t bitP1;
            pSampleBuffer->readNext(bitP0);
            allDone = !pSampleBuffer->readNext(bitP1);
            if (!allDone) {
				uint32_t err_300_700 = msfPeriodMatch_300_700(bitP0, bitP1);
				uint32_t err_200_800 = msfPeriodMatch_200_800(bitP0, bitP1);
				uint32_t err_100_900 = msfPeriodMatch_100_900(bitP0, bitP1);
				uint32_t err_100_100_100_700 = 100;
	            uint8_t bitP2=0;
	            uint8_t bitP3=0;
				if (pSampleBuffer->readPeek(0, bitP2)) {
					if (pSampleBuffer->readPeek(1, bitP3)) {
						err_100_100_100_700 = msfPeriodMatch_100_100_100_700(
								bitP0, bitP1, bitP2, bitP3);
					}
				}
				if (isBestError(err_300_700,
								err_200_800, err_100_900, err_100_100_100_700)) {
					ABits[secsCount] = 1;
					BBits[secsCount] = 1;
					secsCount += 1;
				} else if (isBestError(err_200_800,
								  err_300_700, err_100_900, err_100_100_100_700)) {
					ABits[secsCount] = 1;
					BBits[secsCount] = 0;
					secsCount += 1;
				} else if (isBestError(err_100_900,
								  err_300_700, err_200_800, err_100_100_100_700)) {
					ABits[secsCount] = 0;
					BBits[secsCount] = 0;
					secsCount += 1;
				} else if (isBestError(err_100_100_100_700,
									   err_300_700, err_200_800, err_100_900)) {

					ABits[secsCount] = 0;
					BBits[secsCount] = 1;
					pSampleBuffer->readSkip(2);
					secsCount += 1;
				} else {
					snprintf(
						messageBuff, sizeof(messageBuff),
						"extractABBits failed: @%u {%u,%u,%u,%u} %u,%u,%u,%u",
						startOffset,
						bitP0, bitP1, bitP2, bitP3,
		                err_300_700, err_200_800,
		                err_100_900, err_100_100_100_700
					);
					decodeMsg.append(messageBuff, 0);
					rCode = false;
				}
            }
		}
	}
	if (rCode == false) {
	    showMSFBitPeriods(pSampleBuffer, decodeMsg);
	}
	return rCode;
}

/*!
 * Turns an array of A bits and a matching array of B bits into a text string
 * for display.
 * @param ABits and array of A bits - one entry per bit
 * @param BBits and array of B bits - one entry per bit
 * @param secsCount the number of entries in ABits[] and BBits[]
 * @param decodeMsg - the message into which the text string is appended
 */
static void showABBitSet(
	const uint8_t ABits[],
	const uint8_t BBits[],
	size_t secsCount,
    CMsg& decodeMsg
) {
    char messageBuff[128];
    snprintf(
        messageBuff, sizeof(messageBuff),
        "Extracted AB bits for %u secs\r", secsCount);
    decodeMsg.append(messageBuff, "\r");
    /*
     *            1         2         3         4         5         6
     *   123456789012345678901234567890123456789012345678901234567890
     * A=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
     * B=bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb
     */
    decodeMsg.append("           1         2         3         4         5         6\r", 0);
    decodeMsg.append("  123456789012345678901234567890123456789012345678901234567890\r", 0);
    decodeMsg.append("A=", 0);
    for (size_t idx=0; idx < secsCount; ++idx) {
        messageBuff[idx]='0' + (ABits[idx] & 1);
    }
    messageBuff[secsCount]='\r';
    messageBuff[secsCount+1]='\0';
    decodeMsg.append(messageBuff, 0);
    decodeMsg.append("B=", 0);
    for (size_t idx=0; idx < secsCount; ++idx) {
        messageBuff[idx]='0' + (BBits[idx] & 1);
    }
    decodeMsg.append(messageBuff, 0);
}

/*!
 * Converts two nibbles in BCD to a decimal value
 * @param bcdHexValue containing h4:l4 ((h4 << 4) | l4)
 * @returns decimal value as 10*h4 + l4
 */
static inline uint8_t bcdDecimalValue(
	uint8_t bcdHexValue
) {
	return 10*(bcdHexValue >> 4) + (bcdHexValue & 0x0F);
}

/*!
 * Builds a value from a contiguous set of extracted bits
 * @param bitSet an array containing one entry per bit [x]=0|1
 * @param startBit index of starting bit
 * @param bitCount number of bits (Should be [1..8])
 * @return unsigned 8 bit value
 */
static uint8_t buildValueFromBitset(
	const uint8_t bitSet[],
	size_t startBit,
	size_t bitCount
) {
	uint8_t value = 0;
	for (size_t idx = 0; idx < bitCount; ++idx) {
		value = (value << 1) | bitSet[startBit-1+idx];
	}
	return value;
}

/*!
 * Counts the number of consecutive 1 bits within a bit set
 * @param bitSet an array containing one entry per bit [x]=0|1
 * @param startBit index of starting bit
 * @param bitCount number of bits to scan
 * @return the number of consecutive 1 bits found [0..bitCount]
 */
static uint8_t countLeading1Bits(
	const uint8_t bitSet[],
	size_t startBit,
	size_t bitCount
) {
	uint8_t count = 0;
	while (bitCount-- > 0) {
		if (bitSet[startBit++ - 1] == 1)
			++count;
		else
			break;
	}
	return count;
}

/*!
 * Calculates the parity (xor of bits)
 * @param bitSet an array containing one entry per bit [x]=0|1
 * @param startBit index of starting bit + 1 i.e. index using 1.. based
 *        indexing (done so so that calls to us match the MSF spec for
 *        bit indexing (sorry!))
 * @param bitCount number of bits [1..]
 * @return the xor of each bit bitSet[startBit-1 .. startBit-1+bitCount-1]
 */
static uint8_t calcParity(
	const uint8_t bitSet[],
	size_t startBit,
	size_t bitCount
) {
    startBit -= 1;
	uint8_t parity = bitSet[startBit++];
	while (--bitCount > 0) {
		parity ^= bitSet[startBit++];
	}
	return parity;
}

/*!
 * Decodes an A/B bit set into a MSF_DATE_TIME struct
 * @param ABits array of 0/1 values, one for each bit
 * @param BBits array of 0/1 values, one for each bit
 * @param msfDateTime assigned the decided date time value
 * @param decodeMsg where we return any error message should
 *        we fail to decode
 * @return true if decoded OK, false if not
 */
static bool decodeMSFDateTime(
	const uint8_t ABits[],
	const uint8_t BBits[],
	struct MSF_DATE_TIME& msfDateTime,
	CMsg& decodeMsg
) {
	bool rCode = true;
	/*
	 * Check bits A52..59 are 01111110
	 */
	uint8_t a52_59 = buildValueFromBitset(ABits, 52, 8);
	if (a52_59 != 0x7E) {
		decodeMsg.append("A52..59 code check fail");
		rCode =false;
	}
	/*
	 * Figure out the DUT1 value
	 */
	uint8_t oneCount = countLeading1Bits(BBits,1,8);
	if (oneCount > 0) {
		msfDateTime.DUT1 = 100*oneCount;
	} else {
		oneCount = countLeading1Bits(BBits,9,8);
		if (oneCount > 0) {
			msfDateTime.DUT1 = -100*oneCount;
		}
		else {
			msfDateTime.DUT1 = 0;
		}
	}
	/*
	 * Extract the main date/time values
	 */
	msfDateTime.year = bcdDecimalValue(buildValueFromBitset(ABits, 17, 8));
	msfDateTime.month = bcdDecimalValue(buildValueFromBitset(ABits, 25, 5));
	msfDateTime.day = bcdDecimalValue(buildValueFromBitset(ABits, 30, 6));
	msfDateTime.dayOfWeek = bcdDecimalValue(buildValueFromBitset(ABits, 36, 3));
	msfDateTime.hour = bcdDecimalValue(buildValueFromBitset(ABits, 39, 6));
	msfDateTime.min = bcdDecimalValue(buildValueFromBitset(ABits, 45, 7));
	msfDateTime.BST = (B(58) == 1);
	/*
	 * Check out the parity bits (odd parity)
	 */
	if ((calcParity(ABits, 17, 8) ^ B(54)) != 1) {
		decodeMsg.append("B54 parity error");
		rCode =false;
	}
	if ((calcParity(ABits, 25, 11) ^ B(55)) != 1) {
		decodeMsg.append("B55 parity error");
		rCode =false;
	}
	if ((calcParity(ABits, 36, 3) ^ B(56)) != 1) {
		decodeMsg.append("B56 parity error");
		rCode =false;
	}
	if ((calcParity(ABits, 39, 13) ^ B(57)) != 1) {
		decodeMsg.append("B57 parity error");
		rCode =false;
	}
	if (rCode == false) {
        showABBitSet(ABits, BBits, 60, decodeMsg);
	}
	return rCode;
}

/*!
 * Converts a MSF_DATE_TIME struct into a text form of the date/time value
 * @param msfDateTime the MSF_DATE_TIME struct
 * @param output the CMsg into which the text form is appended
 */
void formatMSFDateTime(
	const struct MSF_DATE_TIME& msfDateTime,
	CMsg& output
) {
	const static char* dayNames[8] = {
	    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "?DAY?"
	};
	uint32_t age = SysTick_readTicks() - msfDateTime.ticksAtTime;
	char str[80];
	snprintf(str, sizeof(str),
	    "%u.%02u|%s %02u/%02u/%02u|%s %02u:%02u|DUT1=%d",
		age/100, age % 100,
		dayNames[msfDateTime.dayOfWeek & 7],
		msfDateTime.day, msfDateTime.month, msfDateTime.year,
		msfDateTime.BST ? "BST" : "GMT",
		msfDateTime.hour, msfDateTime.min,
		msfDateTime.DUT1
	);
	output.append(str, 0);
}

/*!
 * Decodes a period sample buffer into a MSF_DATE_TIME struct. If the decode
 * fails, we return the reason in the decodeMsg
 * @param pSampleBuffer the bit period data set we decode
 * @param msfDateTime the MSF_DATE_TIME struct
 * @param output the CMsg into which the text form is appended
 * @return true if the decode was good, false if the decode failed - in which
 *         case decodeMsg is filled with the reason the decode failed.
 */
bool decodeMSFSampleBuffer(
	struct MSF_SAMPLE_BUFFER* pSampleBuffer,
	struct MSF_DATE_TIME &dateTime,
	CMsg& decodeMsg
) {
	bool rCode = true;
	uint8_t ABits[60];
	uint8_t BBits[60];
	size_t secsCount = 0;
	if (!extractABBits(pSampleBuffer, ABits, BBits, secsCount, decodeMsg)) {
		rCode = false;
	} else if (secsCount < 59) {
		decodeMsg.append("Did not get at least 59 seconds from sample data");
		rCode = false;
	} else {
		if (!decodeMSFDateTime(ABits, BBits, dateTime, decodeMsg)) {
		    showMSFBitPeriods(pSampleBuffer, decodeMsg);
			rCode = false;
		} else {
			dateTime.ticksAtTime = pSampleBuffer->sampleStartTime;
		}
	}
	return rCode;
}
