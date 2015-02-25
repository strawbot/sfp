// SFP Frame level interface  Robert Chapman III  Feb 20, 2015

#include <string.h>

#include "sfp.h"
#include "frame.h"

// build a frame with the proper structure
void buildSfpFrame(Byte length, Byte *data, Byte pid, sfpFrame *frame)
{
	checkSum_t *cs = (checkSum_t *)&frame->payload[length];
	Byte sum=0, sumsum=0;

	frame->length = length + MIN_FRAME_LENGTH; // sync + pid + checksum
	frame->sync = sfpSync(frame->length);
	frame->pid = pid;
	memcpy(frame->payload, data, length);
	calculateFletcherCheckSum(&sum, &sumsum, frame->length - CHECKSUM_LENGTH + LENGTH_LENGTH, &frame->length);
	cs->sum = sum;
	cs->sumsum = sumsum;
}

void calculateFletcherCheckSum(Byte *sum, Byte *sumsum, Byte length, Byte *data)
{
	while(length--)
		*sumsum += *sum += *data++;
}

