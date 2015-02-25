// Stats  Robert Chapman III  Feb 21, 2015

#include "stats.h"
#include "node.h"

#define SET_STAT(stat) \
static Long stat##Stat = 0; \
void stat(void) { stat##Stat++; }

#define SET_LINK_STAT(stat) \
void stat(sfpLink_t *link) { link->stat++; }

FOR_EACH_STAT(SET_STAT)
FOR_EACH_LINK_STAT(SET_LINK_STAT)

void LongFrame(Long length, sfpLink_t * link)
{
	link->LongFrame++;
	(void)length;
}

void ShortFrame(Long length, sfpLink_t * link)
{
	link->ShortFrame++;
	(void)length;
}

void initSfpStats(void)
{
    Long i;
#define ZERO_STAT(stat) stat##Stat = 0;
#define ZERO_LINK_STAT(stat) link->stat = 0;

	FOR_EACH_STAT(ZERO_STAT)

	for (i = 0; i < NUM_LINKS; i++) {
        sfpLink_t *link = nodeLink(i);
        if (link) {
            FOR_EACH_LINK_STAT(ZERO_LINK_STAT)
        }
	}
}

void showSfpStats(void)
{
    Long i;
#define PRINT_STAT(stat) if (stat##Stat) print("\n"#stat" "), printDec(stat##Stat);
#define PRINT_LINK_STAT(stat) if (link->stat) print("\n"#stat" "), printDec(link->stat);

	FOR_EACH_STAT(PRINT_STAT)

	for (i = 0; i < NUM_LINKS; i++) {
        sfpLink_t *link = nodeLink(i);
        if (link) {
            print("\nLink#"), printDec(i);
            FOR_EACH_LINK_STAT(PRINT_LINK_STAT)
        }
    }
}
