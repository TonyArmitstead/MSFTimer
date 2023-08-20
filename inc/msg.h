/*
 * msg.h
 */
#ifndef MSG_H_
#define MSG_H_

#include <stdint.h>
/*!
 * Class to build a message with the following form:
 * <ACK|NAK><length.16>*{msg.8}<CRC.16><CR>
 * where length.16 is 4 hex-ascii characters HHLL
 *       CRC.16 is 4 hex-ascii characters HHLL
 */
class CMsg {
public:
	CMsg();
	void append(const char* pMsg, const char* pSep= ", ");
	void clear();
	const char* getErrorMsg(size_t* pTotalLength = 0);
    const char* getMsg(size_t* pTotalLength = 0);
private:
    uint16_t calcCRC();
    void formMsg();

private:
	static const size_t size = 2048;
	size_t length;
	char message[size];
};

#endif /* MSG_H_ */
