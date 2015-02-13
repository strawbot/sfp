// SFP status queries  Robert Chapman III  Apr 26, 2012

// sfp status
#include "printers.h"
#include "timbre.h"
#include "pids.h"
#include "sfpStats.h"
#include "sfpLink.h"
#include "packets.h"
#include "routing.h"
#include "sfpStatus.h"
#include "node.h"

#include <stdlib.h>

#define SHOW_STAT(stat) if (link->stats->stat) print( "\n " #stat ": "), printDec(link->stats->stat)
#define SHOW_ERROR(stat) if (stat) print( "\n " #stat ": "), printDec(stat)

void iterateFunction(void (*function)(linkInfo_t *));
void iterateFunction(void (*function)(linkInfo_t *))
{
	int i = 0;
	linkInfo_t *link;

	while ((link = sfpNode->links[i++]) != NULL)
	{
		print("\n");
		print(link->name), print(":");
		function(link);
	}
}

// TODO: these should belong to node
extern Long reRoutes, noDest; // number of packets rerouted
Long badLink;

void listStatsLink(linkInfo_t *link);
void listStatsLink(linkInfo_t *link)
{
	Long n;
	
	// byte flow
	SHOW_STAT( bytesIn );
	SHOW_STAT( bytesOut );
	SHOW_STAT( rxError );
	SHOW_STAT( txError );
	// rx
	SHOW_STAT( good_frame );
	SHOW_STAT( long_frame );
	SHOW_STAT( short_frame );
	SHOW_STAT( syncError );
	n = link->stats->short_frame + link->stats->long_frame;
	if (n)
	{
		Byte d, *blq = link->stats->badLengthq;

		print("\n  "), printDec(n), print("bad lengths: ");
		if (n > sizebq(blq))
		{
			n = sizebq(blq);
			wrappedbq(blq); // set remove to insert pointer
		}
		while (n--)
		{
			d = pullbq(blq);
			printHex2(d);
			pushbq(d, blq);
		}
	}
	SHOW_STAT( tossed );
	SHOW_STAT( bad_checksum );
	SHOW_STAT( timeouts );
	SHOW_STAT( rx_overflow );
	SHOW_STAT( unknown_packets);
	SHOW_STAT( unrouted );
	SHOW_STAT( undeliverdPacket );
	// tx
	SHOW_STAT( sent_frames );
	SHOW_STAT( spssent );
	SHOW_STAT( spsacked );
	SHOW_STAT( resends );
	SHOW_STAT( spsfailed );
}

void listStats(void)
{
	iterateFunction(listStatsLink);
	print("\nLink issues:");
	SHOW_ERROR( reRoutes );
	SHOW_ERROR( noDest );
	SHOW_ERROR( badLink );
	print("\n");
}

void spsFails(linkInfo_t *link)
{
	SHOW_STAT( spsfailed );
}

char *inFrameStates[]= {"FRAME_EMPTY", "FRAME_FULL", "FRAME_QUEUED", "FRAME_PROCESSED", "FRAME_REQUEUED"};

void linkStatusLink(linkInfo_t *link);
void linkStatusLink(linkInfo_t *link)
{
	Long flags;

	print("\n Bytes to receive: "), printDec(link->sfpBytesToRx);
	print("\n sfp Rx State: "), printDec(link->sfpRxState);
	print("\n rx Sps "), printDec(link->rxSps);
	print("\n frameIn "), printDec(link->frameIn[0]);
	print(inFrameStates[link->inFrameState]);

	print("\n Bytes To Tx "), printDec(link->sfpBytesToTx);
	if (link->sfpTx != NULL)
		print("\n can something be sent? "), printDec(link->sfpTx());
	print("\n sfp Tx State "), printDec(link->sfpTxState);
	print("\n Link owner: "), printDec( link->linkOwner);
	print("\n tx Sps "), printDec( link->txSps);
	print("\n tx Flags ");
	flags = link->txFlags;
	printHex(flags);
	print("[");
	{
		char **f, *flagNames[] = {"SW","TE","GU","RE","RA","SS","SN","SA","SP"};
		Long mask = 0x100;

		f = &flagNames[0];
		while(mask)
		{
			if (mask & flags)
				print(*f);
			else
				print("  ");
			f++;
			mask >>= 1;
			if (mask)
				print("|");
		}
		print("]");
	}
	print("\n");
}

void linkStatus(void)
{
	iterateFunction(linkStatusLink);
}

extern Queue_t framewaitq;
extern Queue_t rxframeq;

void dumpFramesLink(linkInfo_t *link);
void dumpFramesLink(linkInfo_t *link)
{
	print("\n In     "), printMemLine(link->frameIn);
	print("\n OutNps1"), printMemLine(link->frameOutNps1);
	print("\n OutNps2"), printMemLine(link->frameOutNps2);
	print("\n OutSps "), printMemLine(link->frameOutSps);
	print("\n");
}

void dumpFrames(void)
{
	iterateFunction(dumpFramesLink);
	print("\n framewaitq: "), printDec(queryq(framewaitq));
	print("\n rxframeq: "), printDec(queryq(rxframeq));
	print("\n");
}

void printMemLine(Byte *bytes) // print out one line of memory
{
	Byte i;
	
	print(":");
	for (i=0; i<16; i++)
		printHex2(bytes[i]);
	print(" ");
	for (i=0; i<16; i++)
	{
		Byte b = *bytes++;

		if ((b<' ') || (b>'~'))
			b = '.';
		printChar(b);
	}
}
