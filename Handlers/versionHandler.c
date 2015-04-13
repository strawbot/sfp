// Version

/* version packet consists of 4 fixed length fields and a variable length field.
   The fields are:
    major version: 1 byte, value
    minor version: 1 byte, value
    build number: 2 bytes, value
    build date: 20 bytes, count prefixed string
    build name: n bytes, count prefixed string of name; device dependant but following
                the pattern: IO Boot, IO App, Main Boot, Main App
*/
#include "sfp.h"
#include "valueTransfer.h"
#include "services.h"
#include "version.h"

#include <string.h>

bool getVersion(Byte *packet, Byte length);
void initVersion(void);

bool getVersion(Byte *packet, Byte length)
{
	whoPacket_t *sp = (whoPacket_t *)packet;
	Byte reply[MAX_PACKET_LENGTH];
	versionPacket_t *vp = (versionPacket_t *)reply;
	char *buildDate = getBuildDate(), *buildName = getBuildName();
	
	(void)length;
	vp->pid = VERSION_NO;
	vp->who.to = sp->who.from;
	vp->who.from = sp->who.to;
	vp->major = (Byte)majorVersion();
	vp->minor = (Byte)minorVersion();
	wordToBytes((Short)buildVersion(), (Byte *)&vp->build);
	vp->dateLength = (Byte)strlen(buildDate);
	memcpy(vp->date, buildDate, strlen(buildDate));
	vp->nameLength = (Byte)strlen(buildName);
	memcpy(vp->name, buildName, strlen(buildName));
	return sendNpTo(reply, (Byte)(sizeof(versionPacket_t) + strlen(buildName)), vp->who.to);
}

void initVersion(void)
{
	setPacketHandler(GET_VERSION, getVersion);
}
