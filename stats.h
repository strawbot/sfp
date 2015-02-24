#ifndef _STATS_H_
#define _STATS_H_

#define FOR_EACH_STAT(F) \
	F(FramePoolEmpty) \
	F(FrameProcessed) \
	F(IgnoreFrame) \
	F(NoDest) \
	F(PacketProcessed) \
	F(PacketSizeBad) \
	F(ReRouted) \
	F(UnDelivered) \
	F(UnknownPacket) \
	F(UnknownPid) \
	F(UnRouted)

#define DECLARE_STAT(stat) void stat();

#endif

FOR_EACH_STAT(DECLARE_STAT)

void initSfpStats(void);
void showSfpStats(void);
