// SFSP Link  Robert Chapman  Feb 23, 2012
#include "bktypes.h"
#include "timeout.h"
#include "sfpStats.h"
#include "sfsp.h"

#include <stdlib.h>
#include <string.h>

// external
void initSfspRxSM(linkInfo_t *);
void initSfspTxSM(linkInfo_t *);

void initSfsp(linkInfo_t *link) //! initialize SFSP state machines
{
	initSfspRxSM(link);
	initSfspTxSM(link);
	link->rxErrFunction = NULL; // default is to do nothing
	link->linkOwner = SFP_LINK;
}

void rxLinkError(linkInfo_t *link) // called in each rx error
{
	if (link->rxErrFunction)
		link->rxErrFunction();
}

// build a frame with the proper structure
void calculateFletcherCheckSum(Byte *c1, Byte *c2, Byte length, Byte *data)
{
	while(length--)
		*c2 += *c1 += *data++;
}

void buildSfspFrame(Byte length, Byte *data, Byte pid, Byte *f)
{
	sfspFrame *frame = (sfspFrame *)f;
	Byte *dp = frame->payload, check1=0, check2=0;

	frame->length = length + MIN_SFSP_LENGTH; // packet length + sync + pid + checksum
	frame->sync = sfspSync(frame->length);
	frame->pid = pid;
	while(length--)
		*dp++ = *data++;
	// include front length in checksum calc but not checksum; hence -1
	calculateFletcherCheckSum(&check1, &check2, frame->length-1, &frame->length);
	*dp++ = check1;
	*dp = check2;
}

