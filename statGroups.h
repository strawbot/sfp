#ifndef _STAT_GROUP_H_
#define _STAT_GROUP_H_

#define FOR_EACH_NODE_STAT(F) \
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

#define FOR_EACH_LINK_STAT(F) \
    F(BadSync) \
    F(GoodFrame) \
    F(BadCheckSum) \
    F(RxTimeout) \
    F(BadLength) \
    F(SpsAcked) \
    F(SpsSent) \
    F(SendFrame) \
    F(PollFrame) \
    F(BytesOut) \
    F(BytesIn) \
    F(UnexpectedAck) \
    F(SpsqUnderflow) \
    F(SpsTimeout)

#endif
