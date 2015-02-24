// functions declarations for spsf rx state machine  Robert Chapman III  Feb 16, 2012

#ifndef SFP_RX_SM_H
#define SFP_RX_SM_H

#define bytesToReceive(link)	(link->sfpBytesToRx)

#endif

void switchSfpFrames(void);
bool sfpLengthOk(Byte length, sfpLink_t *link);
bool checkRxSps(Byte sps, sfpLink_t *link);
void processSfpFrame(sfpLink_t *link);
void Hunting(Byte length, sfpLink_t *link);
void Syncing(Byte sync, sfpLink_t *link);
void Receiving(Byte data, sfpLink_t *link);
void Dumping(sfpLink_t *link);
void initSfpRxSM(sfpLink_t *);
bool sfpRxSm(sfpLink_t *link);