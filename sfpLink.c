// SFP Link  Robert Chapman  Feb 23, 2012
#include "bktypes.h"
#include "timeout.h"
#include "sfpStats.h"
#include "sfpLink.h"

#include <stdlib.h>
#include <string.h>

// external
void initSfpRxSM(sfpLink_t *);
void initSfpTxSM(sfpLink_t *);

void initSfp(sfpLink_t *link) //! initialize SFP state machines
{
	initSfpRxSM(link);
	initSfpTxSM(link);
	link->rxErrFunction = NULL; // default is to do nothing
	link->linkOwner = SFP_LINK;
}

void rxLinkError(sfpLink_t *link) // called in each rx error
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

void buildSfpFrame(Byte length, Byte *data, Byte pid, Byte *f)
{
	sfpFrame *frame = (sfpFrame *)f;
	Byte *dp = frame->payload, check1=0, check2=0;

	frame->length = length + MIN_SFP_LENGTH; // packet length + sync + pid + checksum
	frame->sync = sfpSync(frame->length);
	frame->pid = pid;
	while(length--)
		*dp++ = *data++;
	// include front length in checksum calc but not checksum; hence -1
	calculateFletcherCheckSum(&check1, &check2, frame->length-1, &frame->length);
	*dp++ = check1;
	*dp = check2;
}

