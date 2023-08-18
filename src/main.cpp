/*!
 * @file    main.cpp
 */

#include <stddef.h>
#include <stdio.h>
#include <cstring>
#include "hw_config.h"
#include "usb_desc.h"
#include "systick.h"
#include "stm32f10x.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include "usb_endp.h"
#include "msf.h"
#include "samplebuffer.h"

#pragma import(__use_no_semihosting)

/*!
 * Holds a count of the total number of good MSF time reads.
 * We get 1 read per min, so 32 bits should be good for 8000 years!
 */
static uint32_t goodCount = 0;
/*!
 * Holds a count of the total number of bad MSF time reads
 * We get 1 read per min, so 32 bits should be good for 8000 years!
 */
static uint32_t badCount = 0;
/*!
 * We holds read totals for the last 10, 60 and 1440 minutes. For such short
 * periods, 16 bit count values are sufficient (max value of 1440!).
 */
typedef struct _STATS_TOTAL {
    uint16_t goodCount;
    uint16_t badCount;
} STATS_TOTAL;
/*!
 * We holds read totals for the last 1, 10 and 60 minutes. For such short
 * periods, 8 bit count values are sufficient (max value of 60!).
 */
typedef struct _STATS_RECORD {
    uint8_t goodCount;
    uint8_t badCount;
} STATS_RECORD;
/*!
 * Stats values for the last 10 minutes. These are updated every minute.
 */
static STATS_TOTAL S10; /*! Current last 10 mins stats values */
static size_t S10Idx;
static const size_t S10EntryCount = 10;
static STATS_RECORD S10History[S10EntryCount];
/*!
 * Stats values for the last 60 minutes. These are updated every 10 minutes.
 */
static STATS_TOTAL S60; /*! Current last 60 mins stats values */
static size_t S60Idx;
static const size_t S60EntryCount = 6;
static STATS_RECORD S60History[S60EntryCount];
/*!
 * Stats values for the last 1440 minutes.  These are updated every 60 minutes.
 */
static STATS_TOTAL S1440; /*! Current last 1440 mins stats values */
static size_t S1440Idx;
static const size_t S1440EntryCount = 24;
static STATS_RECORD S1440History[S1440EntryCount];

/*!
 * Initialise the stats variables and stores
 */
static void statsInit() {
    goodCount = 0;
    badCount = 0;
    S10Idx = 0;
    S10.goodCount = 0;
    S10.badCount = 0;
    std::memset(S10History, 0, sizeof(S10History));
    S60Idx = 0;
    S60.goodCount = 0;
    S60.badCount = 0;
    std::memset(S60History, 0, sizeof(S60History));
    S1440Idx = 0;
    S1440.goodCount = 0;
    S1440.badCount = 0;
    std::memset(S1440History, 0, sizeof(S1440History));
}

/*!
 * Updates the stats values with the most recent read success
 * @param wasGood true if the last read was a good one
 */
static void statsUpdate(bool wasGood) {
    if (wasGood) {
        ++goodCount;
        S10.goodCount += 1;
        S10.goodCount -= S10History[S10Idx].goodCount;
        S10.badCount -= S10History[S10Idx].badCount;
        S10History[S10Idx].goodCount = 1;
        S10History[S10Idx].badCount = 0;
    } else {
        ++badCount;
        S10.badCount += 1;
        S10.goodCount -= S10History[S10Idx].goodCount;
        S10.badCount -= S10History[S10Idx].badCount;
        S10History[S10Idx].badCount = 1;
        S10History[S10Idx].goodCount = 0;
    }
    if (++S10Idx == S10EntryCount) {
        S10Idx = 0;
        S60.goodCount += S10.goodCount;
        S60.goodCount -= S60History[S60Idx].goodCount;
        S60.badCount += S10.badCount;
        S60.badCount -= S60History[S60Idx].badCount;
        S60History[S60Idx].goodCount = S10.goodCount;
        S60History[S60Idx].badCount = S10.badCount;
        if (++S60Idx == S60EntryCount) {
            S60Idx = 0;
            S1440.goodCount += S60.goodCount;
            S1440.goodCount -= S1440History[S1440Idx].goodCount;
            S1440.badCount += S60.badCount;
            S1440.badCount -= S1440History[S1440Idx].badCount;
            S1440History[S1440Idx].goodCount = S60.goodCount;
            S1440History[S1440Idx].badCount = S60.badCount;
            if (++S1440Idx == S1440EntryCount) {
                S1440Idx = 0;
            }
        }
    }
}

/*!
 * Services data we _receive_ via USB i.e. data sent from the host PC to
 * us. We dont currently have a use for this, so we just echo what we
 * receive back to the host - at least this can be used to verify we are
 * operational.
 */
static void serviceUSB() {
    char tempBuff[128];
    if (USBDeviceState == CONFIGURED) {
        size_t messageLen = USBGetSerial((uint8_t *)tempBuff,
                                  (uint32_t)sizeof(tempBuff));
        if (messageLen > 0) {
            USBPutSerial((uint8_t *)tempBuff,
                          (uint32_t)messageLen);
        }
    }
}

static void addStatsUpdate(
    CMsg& msg
) {
    char tempBuff[128];
    snprintf(
        tempBuff, sizeof(tempBuff),
        "%u,%u,%u,%u,%u,%u,%u,%u",
        goodCount, badCount,
        S10.goodCount, S10.badCount,
        S60.goodCount, S60.badCount,
        S1440.goodCount, S1440.badCount);
    msg.append(tempBuff, "|");
}

/*!
 * Our main processing loop
 */
int main(void) {
    CMsg decodeMsg;
    const char* cdcMessage;
    size_t cdcMessageLength;

    statsInit();
    SysTick_init();
	Set_System();
	Set_USBClock();
	USB_Interrupts_Config();
	USB_Init();
	disableMSFReceiver();
	configureMSFIO();
	enableMSFReceiver();


	while (1) {
		struct MSF_SAMPLE_BUFFER* pSampleBuffer = 0;
		while ((pSampleBuffer=SysTick_getMSFSample()) == 0) {
		    serviceUSB();
		}
		struct MSF_DATE_TIME dateTime;
		decodeMsg.clear();
		bool decodeOK = decodeMSFSampleBuffer(
			pSampleBuffer, dateTime, decodeMsg);
		SysTick_releaseMSFSample();
		if (decodeOK) {
            formatMSFDateTime(dateTime, decodeMsg);
            statsUpdate(true);
            addStatsUpdate(decodeMsg);
			cdcMessage = decodeMsg.getMsg(&cdcMessageLength);
		} else {
            statsUpdate(false);
            addStatsUpdate(decodeMsg);
            cdcMessage = decodeMsg.getErrorMsg(&cdcMessageLength);
		}
		if (cdcMessageLength > 0) {
			if (USBDeviceState == CONFIGURED) {
				USBPutSerial((uint8_t *)cdcMessage,
							  (uint32_t)cdcMessageLength);
			}
		}
	}
}

extern "C" void _sys_exit() {
    while (1) {
    }
}


#ifdef USE_FULL_ASSERT
/*******************************************************************************
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line number
 *                  where the assert_param error has occurred.
 * Input          : - file: pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 *******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{}
}
#endif

