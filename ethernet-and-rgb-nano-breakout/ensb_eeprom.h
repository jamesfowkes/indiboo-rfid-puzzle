#ifndef _ENSB_EEPROM_H_
#define _ENSB_EEPROM_H_

typedef enum _eeprom_contents
{
	eIP_ADDRESS,
	eMAC_ADDRESS,
	NUM_EEPROM_LOCATIONS
} eEEPROM_CONTENTS;

void eeprom_setup();

void eeprom_get(eEEPROM_CONTENTS eToGet, void * dst);
void eeprom_set(eEEPROM_CONTENTS eToGet, const void * src);

#endif
