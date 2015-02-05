// functions declarations for spsf rx state machine  Robert Chapman III  Feb 16, 2012

#ifndef SFSP_RX_SM_H
#define SFSP_RX_SM_H

#define bytesToReceive(link)	(link->sfspBytesToRx)

#endif

void switchSfspFrames(void);
bool sfspLengthOk(Byte length, linkInfo_t *link);
bool checkRxSps(Byte sps, linkInfo_t *link);
void processSfspFrame(linkInfo_t *link);
void Hunting(Byte length, linkInfo_t *link);
void Syncing(Byte sync, linkInfo_t *link);
void Receiving(Byte data, linkInfo_t *link);
void Dumping(linkInfo_t *link);
void initSfspRxSM(linkInfo_t *);
bool sfspRxSm(linkInfo_t *link);