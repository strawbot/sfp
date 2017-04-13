#ifndef SMALLNETWORK_H
#define SMALLNETWORK_H

extern "C" {
#include "mocks.h"
#include "link.h"
}

#define NUM_LINKS 2				// number of links in this node
#define QLINK_SIZE (MAX_FRAME_LENGTH-3)

extern sfpNode_t nodes[4];
extern struct qlink{
    sfpLink_t link;
    NEW_BQ(QLINK_SIZE, byteq);
} links[6];

void initNetworkStats();
void initRoutes();
void runNodes(Long i);
void selectNode(Long n);

void sendPacket(Byte pid, Byte to);
void networkStats();
Long nodeStat(Long node, Long (*stat)(sfpLink_t *link));
Long nodeStat(Long node, Long (*stat)());

void setClockRatio(Long n);
extern Long pings;
bool pingBack(Byte * packet, Byte length);

#endif // SMALLNETWORK_H
