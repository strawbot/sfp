// SFP Frame pool  Robert Chapman III  Feb 21, 2015
/*
 * Using both ends of a queue, the pool allows machines and interrupts to share a pool
 * of frames without requiring protection. This works by allowing machines to take the
 * pool to empty but interrupts can only take it down to 1 frame left. For interrupts
 * interrupting interrupts, protection must be used on the interrupt side.
*/

#include "stats.h"
#include "framePool.h"
#include "printers.h"

static QUEUE(MAX_FRAMES, poolq);
static Byte frames[MAX_FRAMES][(MAX_SFP_SIZE + 3) & ~0x3];

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

// machine access
sfpFrame * getFrame(void)
{
    if (framePoolLeft() > 0)
        return (sfpFrame *)pullq(poolq);
	FramePoolEmpty();
	return NULL;
}

void returnFrame(void * frame)
{
	stuffq((Cell)frame, poolq);
}

// interrupt access
sfpFrame * igetFrame(void)
{
    if (framePoolLeft() > 1)
        return (sfpFrame *)popq(poolq);
	FramePoolEmpty();
	return NULL;
}

void ireturnFrame(void * frame)
{
	pushq((Cell)frame, poolq);
}

// interrupts interrupting interrupts
sfpFrame * iigetFrame(void)
{
	sfpFrame * frame;

	safe(frame = igetFrame());
	return frame;
}

void iireturnFrame(void * frame)
{
	safe(ireturnFrame(frame));
}

// CLI
void listFrames(void)
{
	Long n = MAX_FRAMES;

	while (n--)
        printHex((Cell)&frames[n][0]);
}
