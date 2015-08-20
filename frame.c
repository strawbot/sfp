// SFP Frame level interface  Robert Chapman III  Feb 20, 2015

#include <string.h>

#include "sfp.h"
#include "frame.h"

// build a frame with the proper structure
void addSfpFrame(sfpFrame *frame, Byte length) // add frame header and trailer
{
	frame->length = length + FRAME_OVERHEAD; // sync + checksum
	frame->sync = sfpSync(frame->length);
	addChecksum(frame);
}

void buildSfpFrame(Byte length, Byte *data, Byte pid, sfpFrame *frame)
{
	frame->pid = pid;
	memcpy(frame->payload, data, length);
	addSfpFrame(frame, length + PID_LENGTH);
}

// take checksum structure as argument; calculate with local variables, then put in structure
void calculateCheckSum(checkSum_t *cs, Byte length, Byte *data)
{
    Byte sum = 0, sumsum = 0;

	while(length--)
		sumsum += sum += *data++; // derived from fletcher checksum; sum and sum of sums
	cs->sum = sum;
	cs->sumsum = sumsum;
}

void addChecksum(sfpFrame *frame)
{
	Byte cslength = frame->length + LENGTH_LENGTH - CHECKSUM_LENGTH;
	checkSum_t *cs = (checkSum_t *)(&frame->length + cslength);

	calculateCheckSum(cs, cslength, &frame->length);
}
