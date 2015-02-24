// Async serial port  Robert Chapman III  Aug 12, 2012
#include <string.h>
//#include "AS1.h"

#include "timbre.h"
//#include "asyncPort.h"
#include "sfpStats.h"
#include "sfpLink.h"
#include "sfpTxSm.h"
#include "sfpRxSm.h"
#include "sfpStats.h"
//#include "links.h"

// serial link servicing
// receive
#define ASYNCQ_SIZE (2 * MAX_SFP_FRAME)

#if ASYNCQ_SIZE > MAX_BQ_SIZE
	#error "async rx byte queue needs to be bigger"
#else
	BQUEUE(ASYNCQ_SIZE, asyncrxq);
#endif

void serviceAsyncLinkMachine(void);
void killAsyncLink(void);

void initAsyncLink(void)
{
	zerobq(asyncrxq);
	activateOnce(serviceAsyncLinkMachine);
}

void killAsyncLink(void)
{
	deactivate(serviceAsyncLinkMachine);
}

bool asyncRx(void)
{
	return (bool)(qbq(asyncrxq));
}

Byte asyncGet(void)
{
	return pullbq(asyncrxq);
}

void serviceAsyncRx(void)
{
    AS1_TComData c;

	// TODO: read more than 1 byte at a time?
	while(!fullbq(asyncrxq))
	{
		if (ERR_OK == AS1_RecvChar(&c)) // non-blocking
		{
			pushbq((Byte)c, asyncrxq);
			(asyncLink.stats->bytesIn)++;
		}
		else
			return;
	}
}

// transmit
bool asyncTx(void) // check for room in output buffer
{
	return (bool) (AS1_GetCharsInTxBuf() < AS1_OUT_BUF_SIZE);
}

void asyncPut(Long chr)
{
	if(asyncTx() == 0U) // must read before send to have bit cleared!
		asyncLink.stats->txError++;
	else
	{
		if (ERR_OK == AS1_SendChar((AS1_TComData)chr))
			(asyncLink.stats->bytesOut)++;
		else
			asyncLink.stats->txError++;
	}
}

void serviceAsyncTx(void)
{
	sfpLink_t *link = &asyncLink; // pass link as a pointer
	
	if (bytesToSend(link)) // needs a pointer
		if (asyncTx())
			transmitSfpByte(link);
}

// Task
void serviceAsyncLinkMachine(void) // interface same as mqx
{
	if (asyncLink.linkOwner == SFP_LINK)
	{
		serviceAsyncTx();
		sfpRxSm(&asyncLink);
		sfpTxSm(&asyncLink);
	}
	activate(serviceAsyncLinkMachine);
}

void setBaud57600(void);
void setBaud57600(void)
{
	AS1_SetBaudRateMode(AS1_BM_57600BAUD);
}