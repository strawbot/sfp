// functions declarations for spsf rx state machine  Robert Chapman III  Feb 16, 2012

#ifndef SFP_RX_SM_H
#define SFP_RX_SM_H
#include "tea.h"

#define bytesToReceive(link)	(link->sfpBytesToRx)

#endif

extern Event RxFrame;
void checkDataTimeout(sfpLink_t *link);
bool sfpRxSm(sfpLink_t *link);
void initSfpRxSM(sfpLink_t *, Qtype * frameInq);
