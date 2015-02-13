// SPI commands for remote shell of IO  Robert Chapman III  Apr 26, 2012

#include <mqx.h>
#include <fio.h>
#include <shell.h>

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :    Shell_slota, Shell_slotb
* Returned Value   :    int_32 error code
* Comments         :    command line access to slot a card. 
*
*END*---------------------------------------------------------------------*/

#include "bktypes.h"
#include "timeout.h"
#include "machines.h"
#include "pids.h"
#include "sfpStats.h"
#include "sfpLink.h"
#include "sfpTxSm.h"
#include "packets.h"
#include "links.h"
#include "routing.h"

#include <string.h>

void sendText(int_32 argc, char_ptr argv[], Byte to);
void sendText(int_32 argc, char_ptr argv[], Byte to)
{
	Byte eval[80], l;
	evaluatePacket_t *p = (evaluatePacket_t *)eval;
	char string[100];
		
	while(--argc)
	{
		if (0 == strcmp("esc", *argv)) // check to see if escape needs to be sent
		{
			strcat(string, "\x1b"); // encode for ascii escape
			argv++;
		}
		else
			strcat(string, *++argv);
		strcat(string, " ");
	}

	l = (Byte)strlen(string);
	memcpy(p->payload, string, l);
	p->pid = EVAL;
	p->who.to = to;
	p->who.from = whoami();
	p->payload[l] = 0; // null termination
	l += sizeof(evaluatePacket_t) + 1;
	sendNpTo(eval, l, to);
}

int_32 Shell_slota( int_32 argc, char_ptr argv[] );
int_32 Shell_slota( int_32 argc, char_ptr argv[] )
{
	boolean print_usage, shorthelp = FALSE;
	int_32 return_code = SHELL_EXIT_SUCCESS;

	print_usage = Shell_check_help_request(argc, argv, &shorthelp );
	
	if (!print_usage)  
		sendText(argc, argv, SLOTA);
	else  
	{
	   if (shorthelp)
		   printf("%s\n", argv[0]);
	   else
		   printf("Usage: %s remote commands\n", argv[0]);
	}
	return return_code;
}

int_32 Shell_slotb( int_32 argc, char_ptr argv[] );
int_32 Shell_slotb( int_32 argc, char_ptr argv[] )
{
   boolean                       print_usage, shorthelp = FALSE;
   int_32                        return_code = SHELL_EXIT_SUCCESS;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  
		sendText(argc, argv, SLOTB);
   else  
   {
	   if (shorthelp)
		   printf("%s\n", argv[0]);
	   else
		   printf("Usage: %s remote commands\n", argv[0]);
   }
   return return_code;
}
