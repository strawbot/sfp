// functions declarations for spsf rx state machine  Robert Chapman III  Feb 16, 2012

#ifndef SFP_RX_SM_H
#define SFP_RX_SM_H

#define bytesToReceive(link)	(link->sfpBytesToRx)

#endif

void switchSfpFrames(void);
bool sfpLengthOk(Byte length, linkInfo_t *link);
bool checkRxSps(Byte sps, linkInfo_t *link);
void processSfpFrame(linkInfo_t *link);
void Hunting(Byte length, linkInfo_t *link);
void Syncing(Byte sync, linkInfo_t *link);
void Receiving(Byte data, linkInfo_t *link);
void Dumping(linkInfo_t *link);
void initSfpRxSM(linkInfo_t *);
bool sfpRxSm(linkInfo_t *link);