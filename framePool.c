// SFP Frame pool  Robert Chapman III  Feb 21, 2015

#include "sfp.h"
#include "stats.h"
#include "framePool.h"

static QUEUE(MAX_FRAMES, poolq);
static Byte frames[MAX_FRAMES][(MAX_SFP_SIZE + 3) & ~0x3];

bool returnFrame(void *frame)
{
	pushq((Cell)frame, poolq);
	return true;
}

sfpFrame *getFrame(void)
{
    if (queryq(poolq))
        return (sfpFrame *)pullq(poolq);
	FramePoolEmpty();
	return NULL;
}

void initFramePool(void)
{
	Long n = MAX_FRAMES;

    zeroq(poolq);
	while (n--)
		pushq((Cell)&frames[n][0], poolq);
}

Long framePoolLeft(void)
{
	return queryq(poolq);
}

void listFrames(void)
{
	Long n = MAX_FRAMES;

	while (n--)
		printHex(&frames[n][0]);
}