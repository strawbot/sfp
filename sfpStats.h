// Declarations for SFP stats  Robert Chapman III  Feb 17, 2012
#include "byteq.h"

#ifndef SFPSTATS_H
#define SFPSTATS_H
#define SFP_STATS 11 // number of stats in packet payload

typedef struct {
	// byte flow
	Long bytesIn;
	Long bytesOut;
	Long rxError;
	Long txError;
	// rx
	Long good_frame;
	Long long_frame;
	Long short_frame;
	Long syncError;
	NEW_BQ(16, badLengthq); // keep track of last 16 bad length bytes
	Long tossed;
	Long bad_checksum;
	Long timeouts;
	Long rx_overflow;
	Long unknown_packets;
	Long unrouted;
	Long undeliverdPacket;
	// tx
	Long sent_frames;
	Long spssent;
	Long spsacked;
	Long resends;
	Long spsfailed;
} linkStats_t;

#endif

void LongFrame(Byte length, linkStats_t *stats);
void ShortFrame(Byte length, linkStats_t *stats);
void DumpFrame(linkStats_t *stats);
void GoodFrame(linkStats_t *stats);
void BadCheckSum(linkStats_t *stats);
void Gaveup(linkStats_t *stats);
void Resent(linkStats_t *stats);
void Overflow(linkStats_t *stats);
void SendFrame(linkStats_t *stats);
void UnknownPacket(linkStats_t *stats);
void UnRouted(linkStats_t *stats);
void BadSync(linkStats_t *stats);
void UnDelivered(linkStats_t *stats);
void SpsFailed(linkStats_t *stats);
void SpsAcked(linkStats_t *stats);
void SpsSent(linkStats_t *stats);
void clearStats(void);
void BadLink(void);
void NoDest(void);
