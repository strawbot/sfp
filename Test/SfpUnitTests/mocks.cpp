// Mocks

extern "C" {
#include <stdio.h>
#include "printers.h"
#include "mocks.h"

void print(const char * message)
{
    printf("%s", message);
}

void printHex(unsigned int hex)
{
    printf("%8X ", hex);
}

void printnHex(unsigned int n, unsigned int hex)
{
    if (n)
        printf("%*X ", n, hex);
    else
        printf("%X ", hex);
}

void printDec(unsigned int dec)
{
    printf("%d ", dec);
}

#include "framepool.c"

bool framePoolFull()
{
    return queryq(poolq) == MAX_FRAMES;
}

#include "stats.c"

Long getFramePoolEmpty() { return FramePoolEmptyStat; }
Long getNoDest() { return NoDestStat; }
Long getPacketSizeBad() { return PacketSizeBadStat; }

#include "services.c"

#include "node.c"

#include "machines.c"

#include "timeout.c"

static Long time;
Long getTime()
{
    return time;
}
}
