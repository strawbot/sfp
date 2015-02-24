// Stats  Robert Chapman III  Feb 21, 2015

#include "bktypes.h"

static Long UnRoutedStat = 0;

void UnRouted(void)
{
	UnRoutedStat++;
}

static Long ReRoutedStat = 0;

void ReRouted(void)
{
	ReRoutedStat++;
}

static Long FrameProcessedStat = 0;

void FrameProcessed(void)
{
	FrameProcessedStat++;
}

static Long PacketProcessedStat = 0;

void PacketProcessed(void)
{
	PacketProcessedStat++;
}

static Long UnDeliveredStat = 0;

void UnDelivered(void)
{
	UnDeliveredStat++;
}

static Long UnknownPidStat = 0;

void UnknownPid(void)
{
	UnknownPidStat++;
}

static Long UnknownPacketStat = 0;

void UnknownPacket(void)
{
	UnknownPacketStat++;
}

static Long NoDestStat = 0;

void NoDest(void)
{
	NoDestStat++;
}

static Long IgnoreFrameStat = 0;

void IgnoreFrame(void)
{
	IgnoreFrameStat++;
}

static Long FramePoolEmptyStat = 0;

void FramePoolEmpty(void)
{
	FramePoolEmptyStat++;
}

static Long PacketSizeBadStat = 0;

void PacketSizeBad(void)
{
	PacketSizeBadStat++;
}

void initSfpStats(void)
{
	UnRoutedStat = 0;
	FrameProcessedStat = 0;
	PacketProcessedStat = 0;
	UnDeliveredStat = 0;
	UnknownPidStat = 0;
	UnknownPacketStat = 0;
	NoDestStat = 0;
	IgnoreFrameStat = 0;
	FramePoolEmptyStat = 0;
	PacketSizeBadStat = 0;
}

void showSfpStats(void)
{
	if (UnRoutedStat) print("UnRouted "), printDec(UnRoutedStat);
	if (FrameProcessedStat) print("FrameProcessed "), printDec(FrameProcessedStat);
	if (PacketProcessedStat) print("PacketProcessed "), printDec(PacketProcessedStat);
	if (UnDeliveredStat) print("UnDelivered "), printDec(UnDeliveredStat);
	if (UnknownPidStat) print("UnknownPid "), printDec(UnknownPidStat);
	if (UnknownPacketStat) print("UnknownPacket "), printDec(UnknownPacketStat);
	if (NoDestStat) print("NoDest "), printDec(NoDestStat);
	if (IgnoreFrameStat) print("IgnoreFrame "), printDec(IgnoreFrameStat);
	if (FramePoolEmptyStat) print("FramePoolEmpty "), printDec(FramePoolEmptyStat);
	if (PacketSizeBadStat) print("PacketSizeBad "), printDec(PacketSizeBadStat);
}