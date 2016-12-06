// Initialize a link  Robert Chapman III  Jun 11, 2015

// Principle is to keep code from crashing and inform programmer of fault
// by installing calls which won't crash if not replaced and will inform user

#include "link.h"
#include "printers.h"

static bool voidSfpRx(sfpLink_t * link)
{
	static bool called = false;
	
	if (called == false) {
		called = true;
		print("\nError: '"), print(link->name), print("'.sfpRx is undefined");
	}
	return false;
}

static bool voidSfpTx(sfpLink_t * link)
{
	static bool called = false;
	
	if (called == false) {
		called = true;
		print("\nError: '"), print(link->name), print("'.sfpTx is undefined");
	}
	return false;
}

static Byte voidSfpGet(sfpLink_t * link)
{
	static bool called = false;
	
	if (called == false) {
		called = true;
		print("\nError: '"), print(link->name), print("'.sfpGet is undefined");
	}
	return 0;
}

static void voidSfpPut(Long n, sfpLink_t * link)
{
	static bool called = false;
	
	if (called == false) {
		called = true;
		print("\nError: '"), print(link->name), print("'.sfpPut is undefined");
	}
	(void)link;
	(void)n;
}

static void voidServiceTx(sfpLink_t * link)
{
	static bool called = false;
	
	if (called == false) {
		called = true;
		print("\nError: '"), print(link->name), print("'.serviceTx is undefined");
	}
}

void initLink(sfpLink_t * link, char * name)
{
	link->name = name;
	link->sfpRx = voidSfpRx;
	link->sfpGet = voidSfpGet;
	link->sfpTx = voidSfpTx;
	link->sfpPut = voidSfpPut;
	link->serviceTx = voidServiceTx;
	link->listFrames = false;
	link->rxq = NULL;
	link->txq = NULL;
}
