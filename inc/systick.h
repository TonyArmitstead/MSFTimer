/*
 * systick.h
 *
 *  Created on: 31 Oct 2014
 *      Author: tony
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_

const unsigned SYSTICK_ONESEC = 100;
void SysTick_init(void);
uint32_t SysTick_readTicks(void);
bool SysTick_startSample(void);
struct MSF_SAMPLE_BUFFER* SysTick_getMSFSample(void);
void SysTick_releaseMSFSample(void);

#endif /* SYSTICK_H_ */
