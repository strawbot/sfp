// SFP statistics  Robert Chapman III  Feb 16, 2012

// TODO: roll stats into links
#include "bktypes.h"
#include "timeout.h"
#include "machines.h"
#include "pids.h"
#include "sfpStats.h"
#include "sfpLink.h"
#include "packets.h"
#include "routing.h"
#include "node.h"

#include <stdlib.h>

static Long noDest; // packet that have no routeable destination
extern Long reRoutes;
extern Long badLink;
bool sendStats(Byte *, Byte);
bool clearStatsPacket(Byte *, Byte);

void clearStats(void)
{
	int i;

	for (i=0; sfpNode->links[i] != NULL; i++)
	{
		linkStats_t *stats = sfpNode->links[i]->stats;
		
		stats->long_frame = 0;
		stats->short_frame = 0;
		stats->tossed = 0;
		stats->good_frame = 0;
		stats->bad_checksum = 0;
		stats->timeouts = 0;
		stats->resends = 0;
		stats->rx_overflow = 0;
		stats->sent_frames = 0;
		stats->unknown_packets = 0;
		stats->bytesIn = 0;
		stats->bytesOut = 0;
		stats->unrouted = 0;
		stats->syncError = 0;
		stats->undeliverdPacket = 0;
		stats->spsacked = 0;
		stats->spsfailed = 0;
		stats->spssent = 0;
		INIT_BQ(stats->badLengthq);
	}
	badLink = 0;
	noDest = 0;
	reRoutes = 0;
	setPacketHandler(CLEAR_STATS, clearStatsPacket);
	setPacketHandler(GET_STATS, sendStats);
}

void BadSync(linkStats_t *stats)
{
	stats->syncError++;
}
void LongFrame(Byte length, linkStats_t *stats)
{
	pushbq(length, stats->badLengthq);
	stats->long_frame++;
}

void ShortFrame(Byte length, linkStats_t *stats)
{
	pushbq(length, stats->badLengthq);
	stats->short_frame++;
}

void DumpFrame(linkStats_t *stats)
{
	stats->tossed++;
}

void GoodFrame(linkStats_t *stats)
{
	stats->good_frame++;
}

void BadCheckSum(linkStats_t *stats)
{
	stats->bad_checksum++;
}

void Resent(linkStats_t *stats)
{
	stats->resends++;
}

void Gaveup(linkStats_t *stats)
{
	stats->timeouts++;
}

void Overflow(linkStats_t *stats)
{
	stats->rx_overflow++;
}

void SendFrame(linkStats_t *stats)
{
	stats->sent_frames++;
}

void UnknownPacket(linkStats_t *stats)
{
	stats->unknown_packets++;
}

void UnRouted(linkStats_t *stats)
{
	stats->unrouted++;
}

void UnDelivered(linkStats_t *stats)
{
	stats->undeliverdPacket++;
}

void SpsFailed(linkStats_t *stats)
{
	stats->spsfailed++;
}

void SpsAcked(linkStats_t *stats)
{
	stats->spsacked++;
}

void SpsSent(linkStats_t *stats)
{
	stats->spssent++;
}

void NoDest(void)
{
	noDest++;
}

void BadLink(void)
{
	badLink++;
}

// send stats over SFP
Byte statsPackage[WHO_PACKET_OVERHEAD + sizeof(long)*SFP_STATS] = {STATS, 0}; // pid, #stats, stats->..

Byte packStats(linkStats_t *stats);
Byte packStats(linkStats_t *stats)
{
	statsPacket *p = (statsPacket *)statsPackage;
	
	p->pid = STATS;
	longToBytes(stats->long_frame, &p->long_frame[0]);
 	longToBytes(stats->short_frame, &p->short_frame[0]);
 	longToBytes(stats->tossed, &p->tossed[0]);
 	longToBytes(stats->good_frame, &p->good_frame[0]);
 	longToBytes(stats->bad_checksum, &p->bad_checksum[0]);
 	longToBytes(stats->timeouts, &p->timeouts[0]);
	longToBytes(stats->resends, &p->resends[0]);
 	longToBytes(stats->rx_overflow, &p->rx_overflow[0]);
 	longToBytes(stats->sent_frames, &p->sent_frames[0]);
 	longToBytes(stats->unknown_packets, &p->unknown_packets[0]);
 	longToBytes(stats->unrouted, &p->unrouted[0]);
	return (Byte)(WHO_PACKET_OVERHEAD + sizeof(long)*SFP_STATS);
}

// Packet handlers
bool sendStats(Byte *packet, Byte x)
{
	whoPacket_t *p = (whoPacket_t *)packet;
	statsPacket *sp = (statsPacket *)statsPackage;
	Byte length;
	sfpLink_t *link;

	(void)x;
	sp->who.to = p->who.from;
	sp->who.from = whoami();
	link = routeToLink(sp->who.to);
	if (link == NULL)
		return 0;
	length = packStats(link->stats);
	return sendNpTo(statsPackage, length, sp->who.to);
}

bool clearStatsPacket(Byte *junk1, Byte junk2)
{
	(void)junk1;
	(void)junk2;
	clearStats();
	return true;
}
