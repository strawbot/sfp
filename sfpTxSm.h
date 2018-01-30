
#ifndef SFP_TM_SM_H
#define SFP_TM_SM_H

#include "ttypes.h"

// tx flag bits
#define SEND_POLL_BIT		0x01
#define SEND_ACK_BIT		0x02
#define SEND_SPS_BIT		0x04
#define RCVD_ACK_BIT		0x08

// Macros
#define bytesToSend(link)		(link->sfpBytesToTx != 0)

#define testPollSend(link)		checkBit(SEND_POLL_BIT, link->txFlags)
#define testAckSend(link)		checkBit(SEND_ACK_BIT, link->txFlags)
#define testSpsSend(link)		checkBit(SEND_SPS_BIT, link->txFlags)
#define testAckReceived(link)	checkBit(RCVD_ACK_BIT, link->txFlags)

#define setPollSend(link)		safe(setBit(SEND_POLL_BIT, link->txFlags))
#define setAckSend(link)		safe(setBit(SEND_ACK_BIT, link->txFlags))
#define setSpsSend(link)		safe(setBit(SEND_SPS_BIT, link->txFlags))
#define setAckReceived(link)	safe(setBit(RCVD_ACK_BIT, link->txFlags))

#define clearPollSend(link)		safe(clearBit(SEND_POLL_BIT, link->txFlags))
#define clearAckSend(link)		safe(clearBit(SEND_ACK_BIT, link->txFlags))
#define clearSpsSend(link)		safe(clearBit(SEND_SPS_BIT, link->txFlags))
#define clearAckReceived(link)	safe(clearBit(RCVD_ACK_BIT, link->txFlags))

#endif

// Declarations
void spsAcknowledged(sfpLink_t *link);
void spsReceived(sfpLink_t *link);
void transmitSfpByte(sfpLink_t *link);
void serviceTx(sfpLink_t *link);
void serviceTxq(sfpLink_t *link);
void serviceMasterTx(sfpLink_t *link);
void initSfpTxSM(sfpLink_t *link, Qtype * npsq, Qtype * spsq);
void sfpTxSm(sfpLink_t *link);
void resetTransmitter(sfpLink_t *link);

/*
 when sending sps, check sps state machine for any_sps, only_sps. If true then
 can send with an sps bit set. This is set upon building the frame by oring it in
 with the PID along with the ACK_BIT.
 When the frame is sent, two timeouts are started: resend and giveup.
 If resend timeout occurs, and no ack has been received, then the sps frame bit is set
 and the resend timeout starts again.
 If the giveup timeout occurs, then the sps send bit is cleared, the rcvd ack bit is
 cleared, the sps state machine is moved to a received next one state (or anysps?)
 Perhaps the any_sps state could be used to establish a sps state at the other end. If
 in the anysps state, then an empty sps frame will be sent to establish other end
 sps correctness.  If sps can't get through then no frames can be sent and none
 are lost.  The giveup timeout could be used to establish a end to end coherent sps
 state.  If the sps frame length is used as a status to indicate that the sps frame
 was successfully sent, then it could be used to not lose a frame if the link goes
 down. When the link comes up, if there is a non zero length in the sps frame then
 it will attempt to be sent again. Now this is an iffy zone because the other end
 could have recieved the frame before the link went down.  This would also imply that
 the empty sps frame would have to be kept separate from the sps frame buffer.
 
 One bit indicates ack packets - SPS. A second bit indicates 1 or 0 SPS packet. The
 other 6 bits are left for PIDs. This gives 64 pids which can all be SPS'd. When the
 ack bit is a zero, the sps bit could serve as a MPS (multi packet service) this is
 an efficient large window protocol of up to 64 frames which can be used for sending
 large blocks of data efficiently. This is an extension of the SPS adapted for large
 blocks of data to make it more efficient.  The 6 bits would serve as a packet sequence
 number. Normal pids would serve as transactions confirmations: received up to n. or 
 missing/request packet j. 
 */
