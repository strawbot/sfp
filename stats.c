// Stats  Robert Chapman III  Feb 21, 2015

#include "bktypes.h"

#define SET_STAT(stat) \
static Long stat##Stat = 0; \
void stat(void) { stat##Stat++; }

FOR_EACH_STAT(SET_STAT)

void initSfpStats(void)
{
#define ZERO_STAT(stat) stat##Stat = 0;
	FOR_EACH_STAT(ZERO_STAT)
}

void showSfpStats(void)
{
#define PRINT_STAT(stat) if (stat##Stat) print(#stat" "), printDec(stat##Stat);
	FOR_EACH_STAT(PRINT_STAT)
}