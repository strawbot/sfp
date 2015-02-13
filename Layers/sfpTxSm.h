
#ifndef SFP_TM_SM_H
#define SFP_TM_SM_H

// tx flag bits
#define SEND_POLL_BIT	0x01
#define SEND_ACK_BIT	0x02
#define SEND_NPS_BIT	0x04
#define SEND_SPS_BIT	0x08
#define RCVD_ACK_BIT	0x10
#define RESEND_BIT		0x20
#define GIVEUP_BIT		0x40

#define TX_WORK (SEND_POLL_BIT|SEND_ACK_BIT|SEND_NPS_BIT|SEND_SPS_BIT|RCVD_ACK_BIT|RESEND_BIT|GIVEUP_BIT)

// Macros
#define bytesToSend(link)		(link->sfpBytesToTx != 0)
#define workToDo(link)			(link->txFlags & TX_WORK)

#define testPollSend(link)		checkBit(SEND_POLL_BIT, link->txFlags)
#define testAckSend(link)		checkBit(SEND_ACK_BIT, link->txFlags)
#define testSpsSend(link)		checkBit(SEND_SPS_BIT, link->txFlags)
#define testNpsSend(link)		checkBit(SEND_NPS_BIT, link->txFlags)
#define testAckReceived(link)	checkBit(RCVD_ACK_BIT, link->txFlags)
#define testResend(link)		checkBit(RESEND_BIT, link->txFlags)
#define testGiveup(link)		checkBit(GIVEUP_BIT, link->txFlags)

#define setPollSend(link)		safe(setBit(SEND_POLL_BIT, link->txFlags))
#define setAckSend(link)		safe(setBit(SEND_ACK_BIT, link->txFlags))
#define setSpsSend(link)		safe(setBit(SEND_SPS_BIT, link->txFlags))
#define setNpsSend(link)		safe(setBit(SEND_NPS_BIT, link->txFlags))
#define setAckReceived(link)	safe(setBit(RCVD_ACK_BIT, link->txFlags))
#define setResend(link)			safe(setBit(RESEND_BIT, link->txFlags))
#define setGiveup(link)			safe(setBit(GIVEUP_BIT, link->txFlags))

#define clearPollSend(link)		safe(clearBit(SEND_POLL_BIT, link->txFlags))
#define clearAckSend(link)		safe(clearBit(SEND_ACK_BIT, link->txFlags))
#define clearSpsSend(link)		safe(clearBit(SEND_SPS_BIT, link->txFlags))
#define clearNpsSend(link)		safe(clearBit(SEND_NPS_BIT, link->txFlags))
#define clearAckReceived(link)	safe(clearBit(RCVD_ACK_BIT, link->txFlags))
#define clearResend(link)		safe(clearBit(RESEND_BIT, link->txFlags))
#define clearGiveup(link)		safe(clearBit(GIVEUP_BIT, link->txFlags))

// test and set a bit flag; result indicates if successful
#define requestNps(result, link)	safe(testSetBit(SEND_NPS_BIT, link->txFlags, result))
#define requestSps(result, link)	safe(testSetBit(SEND_SPS_BIT, link->txFlags, result))

// Declarations
void initSfpTxSM(linkInfo_t *);
void serviceTimeouts(void);
void sfpTxSM(void);
void sfpTxTimeouts(void);

// Link as a parameter
void serviceTx(linkInfo_t *link);
void transmitSfpByte(linkInfo_t *link);
bool transmitFrame(Byte *frame, linkInfo_t *link);
void spsAcknowledgedLink(linkInfo_t *link);
void switchNpsFramesLink(linkInfo_t *link);
bool giveupSpsLink(linkInfo_t *link);
void resendSpsLink(linkInfo_t *link);
void sfpTxSm(linkInfo_t *link);

#endif
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