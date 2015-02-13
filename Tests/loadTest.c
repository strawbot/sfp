// Load test for SFP  Robert Chapman III  June 14, 2012

/* see how long it takes to send a set of packets
	receiver will note the time of the first frame
	 - count packets? increment #s
	 - time of first packet sent - time of last packet sent / number of bytes sent
	 - request stats from other end to figure out how many packets made it.
 */
#include "printers.h"
#include "bktypes.h"
#include "timeout.h"
#include "machines.h"
#include "pids.h"
#include "sfpStats.h"
#include "sfpLink.h"
#include "packets.h"
#include "routing.h"

static Timeout testLength_to, sendwait_to;
static Long packetsInGroup = 1000, framesOut;
// TEST_FRAME for peer, NWK_LOAD_FR for network
static Byte testPacket[MAX_PACKET_LENGTH] = {NWK_LOAD_FR, 0};
static Byte testdest;

void throughPutTest(void);
bool testResults(Byte *packet, Byte);
void loadTest(Long n, Byte to);

bool testResults(Byte *packet, Byte length)
{
	statsPacket *p = (statsPacket *)packet;
	Long n = bytesToLong(p->good_frame) - 1;
	Long ms = (Long)sinceTimeout(&testLength_to);

	(void)length;
	print("\n");
	printDec(n), print("packets sent in ");
	if (ms > 10000)
		printDec(ms/1000), print("secs at ");
	else
		printDec(ms), print("msecs at ");
	printDec( ( n * sizeof(testPacket) * 8 ) / ms ), print("Kb/S\n");
	return true;
}

sfpLink_t *loadLink;

void loadTest(Long n, Byte to)
{
	packetsInGroup = n;
	testdest = to;
	loadLink = routeToLink(to);
	if (loadLink)
		activateOnce(throughPutTest);
	else
		print("\nLink unavailable.\n");
}

void throughPutTest(void)
{
	static enum {INIT,LOADING} state = INIT;
	static whoPacket_t clear = {CLEAR_STATS}, get = {GET_STATS}; 
	switch(state)
	{
		case INIT:
			clear.who.to = testdest;
			clear.who.from = whoami();
			if (sendNormalPacketLink((Byte *)&clear, sizeof(clear), loadLink))
			{
				whoPacket_t *wp = (whoPacket_t *)testPacket;
				print ("\nstarting throughput test...");
				flush();
				setPacketHandler(STATS, testResults);
				state = LOADING;
				framesOut = packetsInGroup;
				setTimeout(0, &testLength_to);
				setTimeout(1 TO_SEC, &sendwait_to);
				get.who.to = testdest;
				get.who.from = whoami();
				wp->who.to = testdest;
				wp->who.from = whoami();
			}
			break;
		case LOADING:
			if (framesOut)
			{
				bool flag;

				if (loadLink->enableSps)
					flag = sendSecurePacketLink(testPacket, sizeof(testPacket), loadLink);
				else
					flag = sendNormalPacketLink(testPacket, sizeof(testPacket), loadLink);
				if (flag)
				{
					framesOut--;
					setTimeout(1 TO_SEC, &sendwait_to);
				}

				if (checkTimeout(&sendwait_to))
				{
					print ("\n timedout - ");
					printDec((Long)sinceTimeout(&testLength_to));
					print(" ms; giving up throughput test\n");
					state = INIT;
					return;
				}
			}
			else if (sendNormalPacketLink((Byte *)&get, sizeof(get), loadLink))
			{
				state = INIT;
				print (" done. getting results...");
				flush();
				return;
			}
			break;
	}
	activate(throughPutTest);
}
