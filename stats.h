#include "link.h"

#ifndef _STATS_H_
#define _STATS_H_

#define DECLARE_STAT(stat) void stat();
#define DECLARE_LINK_STAT(stat) void stat(sfpLink_t *link);

#endif

FOR_EACH_STAT(DECLARE_STAT)
FOR_EACH_LINK_STAT(DECLARE_LINK_STAT)

void LongFrame(Long length, sfpLink_t * link);
void ShortFrame(Long length, sfpLink_t * link);

void initSfpStats(void);
void showSfpStats(void);

