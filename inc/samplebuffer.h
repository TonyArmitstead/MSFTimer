/*
 * samplebuffer.h
 *
 *  Created on: 1 Feb 2015
 *      Author: tony
 */

#ifndef INC_SAMPLEBUFFER_H_
#define INC_SAMPLEBUFFER_H_

#include <stddef.h>

/*!
 * The number of period samples. For a normal sample set
 * there can be up to 4 periods per second and there can be
 * upto 60 seconds (normally 59 second entries + 1 for those
 * 'special' times a leap second can be added on).
 */
const size_t MSF_SAMPLE_BYTE_COUNT = 60*4;
/*!
 * MSF data sample buffer
 */
struct MSF_SAMPLE_BUFFER {
    /*! Who owns the buffer */
    enum SAMPLE_OWNER {
        MSF_NOONE,      /*!< Buffer is free for use */
        MSF_SAMPLER,    /*!< Owned by the IRQ sampler code */
        MSF_PROCESSER   /*!< Owned by the client processing code */
    } sampleOwner;
    /*! The ticker time associated with 0 secs */
    uint32_t sampleStartTime;
    /*!
     * Used as a write pointer. Holds the index of the next entry in
     * sampleData that is to be written.
     */
    uint8_t* pRPtr;
    /*!
     * Used as a read pointer. Holds the index of the next entry in
     * sampleData that is to be read.
     */
    uint8_t* pWPtr;
    /*! The period samples */
    uint8_t sampleData[MSF_SAMPLE_BYTE_COUNT];

    bool isEmpty(void) const { return pWPtr == sampleData; }
    bool isFull(void) const { return pWPtr >= sampleData+sizeof(sampleData); }
    void setEmpty(void) { pWPtr = sampleData; }
    void store(uint8_t period) { *pWPtr++ = period; }
    uint8_t unstore() {
        if (pWPtr > sampleData)
            return *--pWPtr;
        return 0;
    }
    void setOwner(enum SAMPLE_OWNER owner) { sampleOwner = owner; }
    enum SAMPLE_OWNER getOwner(void) const { return sampleOwner; }
    void setStartTime(uint32_t time) { sampleStartTime = time; }
    void resetRead(void) { this->pRPtr = this->sampleData; }
    size_t getReadOffset() { return this->pRPtr - this->sampleData; }
    bool readNext(
        uint8_t& data
    ) {
        if (this->pWPtr > this->pRPtr) {
            data = *this->pRPtr++;
            return true;
        }
        data = 0;
        return false;
    }
    bool readPeek(
        int offset,
        uint8_t& data
    ) {
        if (this->pWPtr > this->pRPtr + offset) {
            data = *(this->pRPtr + offset++);
            return true;
        }
        data = 0;
        return false;
    }
    void readSkip(
        int offset
    ) {
        if (this->pRPtr + offset > this->pWPtr) {
            this->pRPtr = this->pWPtr;
        } else if (this->pRPtr + offset < this->sampleData) {
            this->pRPtr = this->sampleData;
        } else {
            this->pRPtr += offset;
        }
    }
};

#endif /* INC_SAMPLEBUFFER_H_ */
