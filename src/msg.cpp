/*
 * msg.cpp
 *
 * Code to form a message which holds a good (ACK) or bad (NAK) pay-load along
 * with a CRC check:
 *      {ACK|NAK}{LLLL}*{msg.8}{CCCC}{CR}
 * where LLLL is the length of the msg as a 16 bit hex-ASCII value, msg.8 is
 * a string of 8 bit characters (UTF-8) with length as indicated by LLLL,
 * CCCC is a 16 bit CRC-16 value for the msg.8 characters,
 * ACK;NAK;CR are the ASCII characters of the same name. Note that the {}
 * characters are field separators and do not actually appear in the protocol.
 *
 * An example of a 'good' message:
 *
 * {ACK}00380.52|Sun 01/02/15|GMT 16:31|DUT1=-500|45,5,10,0,45,5,0,036D2{CR}
 *
 * From which "0.52|Sun 01/02/15|GMT 16:31|DUT1=-500|45,5,10,0,45,5,0,0" is
 * the message content.
 *
 * An example of a 'bad' message:
 *
 * {NAK}0379B55 parity error Extracted A:B <snip-cos-it-was-long!> 04C7A{CR}
 *
 */

#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include "msg.h"

static const char ACK = '\006';
static const char NAK = '\025';
static const char  CR = '\015';
static const size_t HEADER_LEN = 5;
static const size_t TOTAL_OVERHEAD = 10;

/*!
 * Default constructor
 */
CMsg::CMsg() {
    clear();
}

/*!
 * Removes any message content
 */
void CMsg::clear() {
    this->length = 0;
    this->message[HEADER_LEN] = '\0';
}

/*!
 * Adds content to the message with an optional field separator/delimiter
 * @param pMsg the ASCIZ string to append to the message content. Note that
 *             the terminating '\0' is NOT added to the message.
 * @param pSep if not 0, must point to an ASCIZ string used as a field
 *             separator - which means that if the message is not empty, this
 *             string will be added to the content BEFORE the pMsg.
 * @note If the message being appended would overflow the message size. it is
 *       silently truncated. Do not try to add more content than the size of
 *       the message[] array, or it will be silently lost.
 */
void CMsg::append(
	const char* pMsg,
	const char* pSep)
{
	if ((this->length != 0) && (pSep != 0)) {
		// Need to add separator
		this->append(pSep, 0);
	}
	// this->size = 20
	// this->length = 16
	// +--------------------+
	// |xxxxxxxxxxxxxxxx0---|
	// +--------------------+
	// pMsg = "abc"
	// +--------------------+
	// |xxxxxxxxxxxxxxxxabc0|
	// +--------------------+
	size_t lenAvail = this->size - this->length - TOTAL_OVERHEAD; // = 3
	if (lenAvail > 0) {
		size_t appendLength = strlen(pMsg); // = 3
		if (appendLength <= lenAvail) {
			strcat(this->message+HEADER_LEN, pMsg);
			this->length += appendLength;
		} else {
			strncat(this->message+HEADER_LEN, pMsg, lenAvail);
			this->length += lenAvail;
		}
	}
}

/*!
 * Fills in the none body message content i.e. the header and trailer
 */
void CMsg::formMsg() {
    char numStr[5];
    snprintf(numStr, sizeof(numStr),
             "%04X", this->length);
    this->message[1] = numStr[0];
    this->message[2] = numStr[1];
    this->message[3] = numStr[2];
    this->message[4] = numStr[3];
    snprintf(numStr, sizeof(numStr),
             "%04X", calcCRC());
    this->message[this->length+5] = numStr[0];
    this->message[this->length+6] = numStr[1];
    this->message[this->length+7] = numStr[2];
    this->message[this->length+8] = numStr[3];
    this->message[this->length+9] = CR;
}

/*!
 * Gets the message content framed as a 'good' message.
 * @params pTotalLength points to a value assigned the total length of the
 *         message. You _will_ need this as the message returned is not '\0'
 *         terminated.
 * @returns a pointer to the message content
 */
const char* CMsg::getMsg(size_t* pTotalLength) {
    this->message[0] = ACK;
    formMsg();
    if (pTotalLength)
        *pTotalLength = this->length + TOTAL_OVERHEAD;
    return this->message;
}

/*!
 * Gets the message content framed as a 'bad' message.
 * @params pTotalLength points to a value assigned the total length of the
 *         message. You _will_ need this as the message returned is not '\0'
 *         terminated.
 * @returns a pointer to the message content
 */
const char* CMsg::getErrorMsg(size_t* pTotalLength) {
    this->message[0] = NAK;
    formMsg();
    if (pTotalLength)
        *pTotalLength = this->length + TOTAL_OVERHEAD;
    return this->message;
}

/*!
 * Calculates the CRC-16 value for the message body
 * @returns CRC-16 of the message body
 * @note This code is adapted from code at:
 *          http://srecord.sourceforge.net/crc16-ccitt.html
 */
uint16_t CMsg::calcCRC() {
    uint16_t crc = 0xFFFF;
    size_t len = this->length;
    char* pData = this->message + HEADER_LEN;
    while (len-- ) {
        uint16_t data = (uint16_t)*pData++;
        uint16_t v = 0x80;
        for (int i=0; i<8; i++) {
            bool xor_flag = ((crc & 0x8000) != 0);
            crc = crc << 1;
            if (data & v) {
                crc = crc + 1;
            }
            if (xor_flag) {
                crc = crc ^ 0x1021;
            }
            v = v >> 1;
        }
    }
    for (int i=0; i<16; i++) {
        bool xor_flag = ((crc & 0x8000) != 0);
        crc = crc << 1;
        if (xor_flag) {
            crc = crc ^ 0x1021;
        }
    }
    return crc;
}
