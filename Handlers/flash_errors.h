// Flash errors  Robert Chapman III  Aug 28, 2012

#ifndef _FLASH_ERRORS_H_
#define _FLASH_ERRORS_H_

enum {FLASH_OK = 0x00, // No error
	FLASH_NOTAVAIL = 0x01, // Desired program/erase operation is not available
	FLASH_RANGE = 0x02, // The address is out of range
	FLASH_BUSY = 0x03, // Device is busy
	FLASH_SPEED = 0x04, // This device does not work in the active speed mode
	FLASH_PROTECT = 0x05, // Flash is write protected
	FLASH_VALUE = 0x06, // Read value is not equal to written value
	FLASH_PARAM_VALUE = 0x07 // destination address or length is odd
};

#endif

Byte rangeCheck(Long start, Long end);