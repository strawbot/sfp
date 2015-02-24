#include "bktypes.h"
#include "sfp.h"

typedef struct {
	Byte length;
	Byte packet[MAX_PACKET_LENGTH];
}   ;

typedef enum  {PU_NON_BLOCK, PU_BLOCK} puRequest;
pu_t *grabPu(puRequest type);
bool shipPu(puRequest type, pu_t *pu);
void returnPu(pu_t *pu);