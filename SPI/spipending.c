// SPI Pending transmission detector  Robert Chapman III  Jun 5, 2012

#include "bktypes.h"
#include "byteq.h"
#include "spipending.h"

// SPI can only be switched when there is no pending transmits or receives
// queue is empty if nothing pending
Long spiBytesSent, spiBytesRcvd; // used to track transmits and receives pending

void spiSent(void) // called for each transmission
{
	spiBytesSent++;
}

void spiRcvd(void) // called for each reception
{
	spiBytesRcvd++;
}

Long spiPending(void) // called to see if there are any pending transactions
{	
	return (spiBytesSent - spiBytesRcvd);
}

void initSpiPending(void)
{
	spiBytesSent = spiBytesRcvd = 0;
}