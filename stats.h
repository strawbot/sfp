#include "link.h"

#ifndef _STATS_H_
#define _STATS_H_

#define DECLARE_NODE_STAT(stat) void stat(void);
#define DECLARE_LINK_STAT(stat) void stat(sfpLink_t *link);

#endif

FOR_EACH_NODE_STAT(DECLARE_NODE_STAT)
FOR_EACH_LINK_STAT(DECLARE_LINK_STAT)

void LongFrame(Long length, sfpLink_t * link);
void ShortFrame(Long length, sfpLink_t * link);

void initSfpStats(void);
void showSfpStats(void);

void showLinkStatus(sfpLink_t * link);
void showNodeStatus(void);
