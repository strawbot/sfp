// SFP status queries  Robert Chapman III  Apr 26, 2012

#include <mqx.h>
#include <fio.h>
#include <shell.h>
#include <string.h>
#include <stdlib.h>

// sfp status
#include "bktypes.h"
#include "trapBpErrors.h"

void listm(void);
Long getTime(void);
Long fletcher32(Byte *data, Long len);
void pingTestCmd(char *option, int number, int service);
void loadTestCmd(int number, char *option);
void listStats(void);
void linkStatus(void);
void clearStats(void);
void dumpFrames(void);
void guidHandlerStatus(void);
void modbusSfpStatus(void);
void guidDumps(void); // dump out guid packets
void sfpShell(void);
void printPuStats(void);
void showNxStats(void);
void showVersion(void);
void sfpConnect(void);
void altSfpStatus(void);
void i2cAsGpio(void);
void i2cAsI2c(void);
void toggleI2cClock(int clocks, int delay);
void clkHi(void);
void clkLo(void);
void i2cStatus(void);
void i2cWrite(int n);
void i2sWrite(int n);
void i2dWrite(int n);

int_32 Shell_sfp( int_32 argc, char_ptr argv[] );
int_32 Shell_sfp( int_32 argc, char_ptr argv[] )
{
	boolean                       print_usage, shorthelp = FALSE;
	int_32                        return_code = SHELL_EXIT_SUCCESS;
	
	print_usage = Shell_check_help_request(argc, argv, &shorthelp );
	
	if (!print_usage)
	{
   		if (--argc)
   		{
			char *option = *++argv;
			int number=1;
			
			if (0 == strcmp("frames", option))
				listStats();
			else if (0 == strcmp("links", option)) 
				linkStatus();
			else if (0 == strcmp("clear", option)) 
				clearStats();
			else if (0 == strcmp("dump", option)) 
				dumpFrames();
 			else if ( (0 == strcmp("ping", option) ) || (0 == strcmp("sping", option) ) )
 			{
				int service = (0 == strcmp("sping", option));

				if (argc)
				{
					--argc;
 					option = *++argv;
 				}
 				else
 					option = "";
				if (argc)
 					number = atoi(*++argv);
 				
 				pingTestCmd(option, number, service);
			}
			else if (0 == strcmp("load", option)) 
 			{
 				number = 10;
				if (argc)
				{
					--argc;
 					option = *++argv;
 				}
 				else
 					option = "";
				if (argc)
 					number = atoi(*++argv);
				loadTestCmd(number, option);
			}
			else if (0 == strcmp("status", option))
			{
				showVersion();
				printf("\nTime: %d:", getTime());
				listm();
			}	
			else if (0 == strcmp("checksum", option))
			{
				Byte *address;
				Long length;
				 
				address = (Byte *)strtoul(*++argv, NULL, 0);
				length = (Long)atoi(*++argv);
				printf ("Checksum for %X length %u  Result: %d", address, length, fletcher32(address, length));
			}
			else if (0 == strcmp("guids", option))
				guidHandlerStatus();
			else if (0 == strcmp("modbus", option))
				modbusSfpStatus();
			else if (0 == strcmp("gdump", option))
				guidDumps();
			else if (0 == strcmp("shell", option))
				sfpShell();
			else if (0 == strcmp("connect", option))
				sfpConnect();
			else if (0 == strcmp("pu", option))
				printPuStats();
			else if (0 == strcmp("nx", option))
				showNxStats();
			else if (0 == strcmp("version", option))
				showVersion();
			else if (0 == strcmp("connect", option))
				sfpConnect();
			else if (0 == strcmp("alt", option))
				altSfpStatus();
			else if (0 == strcmp("dmatrap", option))
				setDmaTrap();
			else if (0 == strcmp("dmadump", option))
				dumpDmaBuffer();
/*			else if (0 == strcmp("i2c", option))
				i2cAsI2c();
			else if (0 == strcmp("clklo", option))
				clkLo();
			else if (0 == strcmp("clkhi", option))
				clkHi();
			else if (0 == strcmp("gpio", option))
				i2cAsGpio();
			else if (0 == strcmp("clocks", option))
 				toggleI2cClock(atoi(*++argv), atoi(*++argv));
			else if (0 == strcmp("i2cs", option))
 				i2cStatus();
			else if (0 == strcmp("i2ccw", option))
 				i2cWrite(atoi(*++argv));
			else if (0 == strcmp("i2csw", option))
 				i2sWrite(atoi(*++argv));
			else if (0 == strcmp("i2cdw", option))
 				i2dWrite(atoi(*++argv));
*/  		}
   		printf("\n");
	}
	else  
	{
		if (shorthelp)
			printf("%s [options]\n", argv[0]);
		else
		{
			printf("Usage:%s [frames, links, clear, dump, ping]\n", argv[0]);
			printf(" sfp frames - display frame level statistics\n");
			printf(" sfp links - display link status for each link\n");
			printf(" sfp clear - reset frame statistics to zero\n");
			printf(" sfp dump - dump the contents of the input and output frame buffers\n");
			printf(" sfp ping [link] [n] - find out average response time to ping the link n times\n");
			printf(" sfp sping [link] [n] - find avg resp time to securely ping the link n times\n");
			printf(" sfp load - run load test using 10 packets\n");
			printf(" sfp status - list all machines running\n");
			printf(" sfp checksum address length - get checksum for length bytes at address\n");
			printf(" sfp guids - list stats for guid handler\n");
			printf(" sfp gdump - dump contents of guid packets\n");
			printf(" sfp modbus - list stats for modbus service\n");
			printf(" sfp shell - start a debug shell; exit with ESC\n");
			printf(" sfp connect - start a debug shell using SFP over eng port; exit with ESC\n");
			printf(" sfp pu - print stats for packet unit service\n");
			printf(" sfp nx - print stats for nx file transfer\n");
			printf(" sfp version - print version\n");
			printf(" sfp alt - print ALT status\n");
			printf(" sfp dmatrap - trap and stop on link error over bpuart link\n");
			printf(" sfp dmadump - dump results of last error\n");
/*			printf(" sfp gpio - switch I2C bus to just GPIO\n");
			printf(" sfp i2c - switch I2C bus to I2C\n");
			printf(" sfp clocks n d - drive I2C bus with n clocks and d delay for half period\n");
			printf(" sfp i2cs - print out values in status and control registers\n");
			printf(" sfp i2ccw value - write value to control register\n");
			printf(" sfp i2csw value - write value to status register\n");
			printf(" sfp i2cdw value - write value to data register\n");
*/		}
	}
	return return_code;
}
