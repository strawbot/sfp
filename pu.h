#include "bktypes.h"
#include "sfp.h"

// TODO: add: link pointer so it can be linked into alink linked queue?
typedef struct {
	Byte length;
	Byte packet[MAX_PACKET_LENGTH];
	Byte index;
} pu_t;

typedef enum  {PU_NON_BLOCK, PU_BLOCK} puRequest;
pu_t *grabPu(puRequest type);
bool shipPu(puRequest type, pu_t *pu);
void returnPu(pu_t *pu);