// functions declarations for spsf rx state machine  Robert Chapman III  Feb 16, 2012

#ifndef SFP_RX_SM_H
#define SFP_RX_SM_H

#define bytesToReceive(link)	(link->sfpBytesToRx)

#endif

bool sfpRxSm(sfpLink_t *link);
void initSfpRxSM(sfpLink_t *, Qtype * frameInq);
