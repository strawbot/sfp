// ping throughput test  Robert Chapman III  May 31, 2012

/*
  use ping and pingback to run a throughput test.  Send ping to target and wait
  for pingback before sending another ping. do this for many cycles. timeout if
  no response. report number of successful pings and any errors. calculate the
  response time.
*/

#include "printers.h"
#include "bktypes.h"
#include "timeout.h"
#include "machines.h"
#include "pids.h"
#include "sfpStats.h"
#include "sfsp.h"
#include "packets.h"
#include "sfspTxSm.h"

bool pingBackResponse(Byte *, Byte);
void pingTest(char *name, linkInfo_t *l, int p, int s);
void runPingTest(void);

static enum {WAIT_FOR_PINGBACK, WAIT_TO_PING, GOT_PINGBACK} pingback;

static Long responseTime, responses, no_timeouts, minrs, maxrs;
static Timeout lengthTo;
static Byte pidFrame[] = {PING};
static Long giveup = 150 TO_MSECS, pings, service;
static linkInfo_t *link;

bool pingBackResponse(Byte *packet, Byte length)
{
	(void)packet;
	(void)length;
	pingback = GOT_PINGBACK;
	return true;
}

void pingTest(char *name, linkInfo_t *l, int p, int s)
{
	responseTime=0, responses=0, no_timeouts=0, minrs=1000, maxrs=0;
	pingback = WAIT_TO_PING;

	pings = (Long)p;
	service = (Long)s;
	link = l;
	
	setPacketHandler(PING_BACK, pingBackResponse);
	
	print ("\nPinging "), print (name), print(" "), printDec(pings), print("times:\n");
	setTimeout(0, &lengthTo);

	activateOnce(runPingTest);
}

void runPingTest(void)
{
	if (pings)
	{
		switch (pingback)
		{
			case WAIT_TO_PING:
				{
					bool result;
					
					if (service)
						result = sendSecurePacketLink(pidFrame, sizeof(pidFrame), link);
					else
						result = sendNormalPacketLink(pidFrame, sizeof(pidFrame), link);
			
					if (result)
					{
						setTimeout(0, &lengthTo);
						pingback = WAIT_FOR_PINGBACK; // state change before sending packet
					}
				}
				break;
			case WAIT_FOR_PINGBACK:
				break;
			case GOT_PINGBACK:
				{
					Long t = (Long)sinceTimeout(&lengthTo);
					
					responses++;
					pings--;
					responseTime += t;
					if (t > maxrs) maxrs = t;
					if (t < minrs) minrs = t;
					pingback = WAIT_TO_PING;
				}
				break;
		}

		if (sinceTimeout(&lengthTo) > giveup) // always check timeout
		{
			setTimeout(0, &lengthTo);
			no_timeouts++;
			pings--;
			pingback = WAIT_TO_PING;
		}

		activate(runPingTest);
		return;
	}

	flush();

	if (no_timeouts)
		print(" "), printDec(no_timeouts), print (" timeouts. ");

	if (responses)
	{
		if (responseTime < responses)
			print ("Average response time "), printDec((responseTime*1000)/responses), print("us\n");
		else
		{
			print("Average response time "), printDec(responseTime/responses);
			print("ms  minimum "), printDec(minrs), print("ms  maximum ");
			printDec(maxrs), print("ms\n");
		}
	}
	else
		print ("No responses\n");
}

/* Augments:
 o continuous with start and stop
 o PING accepted as PINGBACK during testing to allow for echos
 o multiple links running simultaneously
 o use link sink with link source for tests
 o remove support for SPS
*/
