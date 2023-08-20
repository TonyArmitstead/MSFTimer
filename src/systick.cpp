/*
 * systick.cpp
 *
 * Holds the ticker IRQ handler and the MSF sampler code.
 *
 */

#include <algorithm>
#include "stm32f10x.h"
#include "systick.h"
#include "msf.h"
#include "samplebuffer.h"

/*!
 * Holds MSF sampler the state machine state
 */
volatile static enum MSF_SAMPLER_STATE {
	MSF_IDLE,               /*!< Pauses the sampling */
	MSF_START,              /*!< Starts sampling */
    MSF_ZSEC_WAIT_FOR_LOW,  /*!< Waiting for the 0s low period to start */
	MSF_ZSEC_LOW_PERIOD,    /*!< Whilst in a potential 0s low period */
	MSF_ZSEC_HIGH_PERIOD,   /*!< Whilst in a potential 0s high period */
	MSF_SEC_SAMPLING        /*!< Sampling data bits */
} msfSampleState = MSF_IDLE;
/*!
 * The MSF sample buffer
 */
static struct MSF_SAMPLE_BUFFER sampleBuffer;
/*!
 * Holds the system tick counter which is a counter
 * incremented every 10ms
 */
volatile static uint32_t tickCount = 0;

/*!
 * Stores a period sample into the sampleBuffer
 * @param period the period value (number of 10ms slots) to store
 * @return the next sampler state. This will normally be MSF_SEC_SAMPLING
 *         but if the sample buffer gets full then we will reset the 
 *         sampling state machine ready for the next sample.
 */
static enum MSF_SAMPLER_STATE storeMSFPeriod(
	uint8_t period
) {
	enum MSF_SAMPLER_STATE nextState = MSF_SEC_SAMPLING;
	/* Is there space in the period buffer? */
	if (!sampleBuffer.isFull()) {
		/* yes, so store */
		sampleBuffer.store(period);
	} else {
		/* no, so release ownership of the buffer */
		sampleBuffer.setOwner(MSF_SAMPLE_BUFFER::MSF_NOONE);
		/* and restart the state machine */
		nextState = MSF_START;
	}
	return nextState;
}

/*!
 * The MSF sample state machine
 *
 *  +0   +100 +200 +300 +400 +500 +600 +700 +800 +900 +1000  ms
 *   +----+----+----+----+----+----+----+----+----+----+
 * The zero secs marker looks like:
 *                             _________________________
 * 0 |________________________|
 *
 * Following that the rest of the secs look like:
 *          ____ ____ __________________________________
 * x |_____|_Ax_|_Bx_|
 *
 */
static void msfSampler(void) {
    const uint32_t NOISE_REJECT_PERIOD = 5;
	/*!
	 * The ticker time associated with the falling edge of a
	 * start of a zero second marker
	 */
	static uint32_t zeroSecStartTime;
	/*! The ticker time a 1->0 transition was observed */
	static uint32_t lowTransitionTime;
	/*! The ticker time a 0->1 transition was observed */
	static uint32_t highTransitionTime;
	/*! The previous sample level */
	static uint8_t lastMSFLevel = 1;
	/*! The current sample level */
	uint8_t msfLevel = msfSample();
	/*! The level transition identified at this sample */
	enum {none, high, low} transitionType = none;
	/*! The ticker period the previous level was seen for */
	uint32_t period = 0;
	/*
	 * Figure out any level transition.
     * Reject small periods as noise, so we effectively
     * replace _____    __________________
     *              |__|                  |________________
     * with    ___________________________
     *                                    |________________
     * and replace   __                    _________________
     *         _____|  |__________________|
     * with                                _________________
     *         ___________________________|
	 */
	if (msfLevel == 1) {
		if (lastMSFLevel == 0) {
		    /* 0 -> 1 transition */
            period = tickCount-lowTransitionTime;
		    if (period > NOISE_REJECT_PERIOD) {
		        highTransitionTime = tickCount;
		        transitionType = high;
		    } else {
                if (msfSampleState == MSF_SEC_SAMPLING) {
                    sampleBuffer.unstore();
                }
		    }
		}
	} else {
		if (lastMSFLevel == 1) {
            /* 1 -> 0 */
            period = tickCount-highTransitionTime;
            if (period > NOISE_REJECT_PERIOD) {
                lowTransitionTime = tickCount;
                transitionType = low;
            } else {
                if (msfSampleState == MSF_SEC_SAMPLING) {
                    sampleBuffer.unstore();
                }
            }
		}
	}
	/*
	 * Process any transition with the state machine
	 */
	switch (msfSampleState) {
		case MSF_IDLE:
			break;
		case MSF_START:
		case MSF_ZSEC_WAIT_FOR_LOW:
			if (transitionType == low) {
				zeroSecStartTime = tickCount;
				msfSampleState = MSF_ZSEC_LOW_PERIOD;
			}
			break;
		case MSF_ZSEC_LOW_PERIOD:
			if (transitionType == high) {
				if (msfPeriodLengthMatch(period, SYSTICK_ONESEC/2)) {
					msfSampleState = MSF_ZSEC_HIGH_PERIOD;
				} else {
					msfSampleState = MSF_ZSEC_WAIT_FOR_LOW;
				}
			}
			break;
		case MSF_ZSEC_HIGH_PERIOD:
			if (transitionType == low) {
				if (msfPeriodLengthMatch(period, SYSTICK_ONESEC/2)) {
					if ((sampleBuffer.getOwner() == MSF_SAMPLE_BUFFER::MSF_SAMPLER) ||
						(sampleBuffer.getOwner() == MSF_SAMPLE_BUFFER::MSF_NOONE)) {
						sampleBuffer.setOwner(MSF_SAMPLE_BUFFER::MSF_SAMPLER);
						sampleBuffer.setEmpty();
						msfSampleState = MSF_SEC_SAMPLING;
					} else {
						msfSampleState = MSF_START;
					}
				} else {
					zeroSecStartTime = tickCount;
					msfSampleState = MSF_ZSEC_LOW_PERIOD;
				}
			}
			break;
		case MSF_SEC_SAMPLING:
			if (transitionType == low) {
				if (sampleBuffer.getOwner() == MSF_SAMPLE_BUFFER::MSF_SAMPLER) {
                    msfSampleState = storeMSFPeriod(period);
				}
			} else if (transitionType == high) {
				if (msfPeriodLengthMatch(period, SYSTICK_ONESEC/2)) {
					zeroSecStartTime = lowTransitionTime;
					if (sampleBuffer.getOwner() == MSF_SAMPLE_BUFFER::MSF_SAMPLER) {
						sampleBuffer.setStartTime(zeroSecStartTime);
						sampleBuffer.setOwner(MSF_SAMPLE_BUFFER::MSF_NOONE);
					}
					msfSampleState = MSF_ZSEC_HIGH_PERIOD;
				}
				else {
					if (sampleBuffer.getOwner() == MSF_SAMPLE_BUFFER::MSF_SAMPLER) {
                        msfSampleState = storeMSFPeriod(period);
					}
				}
			}
			break;
		default:
			msfSampleState = MSF_START;
			break;
	}
	lastMSFLevel = msfLevel;
}

/*!
 * The Systick Interrupt Handler, should be invoked every 10ms
 */
extern "C"
void SysTick_Handler(void) {
	++tickCount;
	msfSampler();
}

/*!
 * Initialises the system tick IRQ rate and prepares the MSF sampling
 * state machine and its sample buffer.
 */
void SysTick_init(void) {
	sampleBuffer.setOwner(MSF_SAMPLE_BUFFER::MSF_NOONE);
	sampleBuffer.setEmpty();
	SysTick_Config(SystemCoreClock / 100); /* Generate interrupt each 10 ms */
	msfSampleState = MSF_START;
}

/*!
 * Attempts to get ownership of the MSF sample buffer. If successful it is
 * important that it is returned to the sampler by calling
 * SysTick_releaseMSFSample(). Once the caller gets a non-zero pointer value
 * the sample buffer is deemed owned by a client and no further sampling
 * is performed. A client typically has just less then 500ms to decode the
 * buffer and return it to the sampler if we are to successfully sample the
 * next minutes data. If this schedule is missed we will not be able to sample
 * the next minute and will get the following one.
 * @returns 0 if the buffer is not currently available, or a pointer to
 *          the sample buffer available for decoding.
 */
struct MSF_SAMPLE_BUFFER* SysTick_getMSFSample(void) {
	struct MSF_SAMPLE_BUFFER* pSample = 0;
	if ((sampleBuffer.getOwner() == MSF_SAMPLE_BUFFER::MSF_NOONE) &&
		!sampleBuffer.isEmpty()) {
		sampleBuffer.setOwner(MSF_SAMPLE_BUFFER::MSF_PROCESSER);
		pSample = &sampleBuffer;
	} else if (sampleBuffer.getOwner() == MSF_SAMPLE_BUFFER::MSF_PROCESSER) {
		sampleBuffer.setEmpty();
		sampleBuffer.setOwner(MSF_SAMPLE_BUFFER::MSF_NOONE);
	}
	return pSample;
}

/*!
 * Returns ownership of the sample buffer to the sampler.
 */
void SysTick_releaseMSFSample(void) {
	sampleBuffer.setEmpty();
	sampleBuffer.setOwner(MSF_SAMPLE_BUFFER::MSF_NOONE);
}

/*!
 * Returns the current system tick count value. This is a 32 bit value
 * incremented every 10ms.
 */
uint32_t SysTick_readTicks(void) {
	return tickCount;
}
