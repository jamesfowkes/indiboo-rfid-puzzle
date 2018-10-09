#include <EEPROMex.h>

#include "ensb_eeprom.h"

static int s_eeprom_addresses[NUM_EEPROM_LOCATIONS];

void eeprom_setup()
{
	s_eeprom_addresses[eIP_ADDRESS] = EEPROM.getAddress(sizeof(uint8_t) * 4);
	s_eeprom_addresses[eMAC_ADDRESS] = EEPROM.getAddress(sizeof(uint8_t) * 6);
}

void eeprom_get(eEEPROM_CONTENTS eToGet, void * dst)
{
	int addr = s_eeprom_addresses[eToGet];

	switch(eToGet)
	{
	case eIP_ADDRESS:
		EEPROM.readBlock(addr, (uint8_t*)dst, 4);
		break;
	case eMAC_ADDRESS:
		EEPROM.readBlock(addr, (uint8_t*)dst, 6);
		break;
	default:
		break;
	}
}

void eeprom_set(eEEPROM_CONTENTS eToGet, const void * src)
{
	int addr = s_eeprom_addresses[eToGet];

	switch(eToGet)
	{
	case eIP_ADDRESS:
		EEPROM.updateBlock(addr, (uint8_t*)src, 4);
		break;
	case eMAC_ADDRESS:
		EEPROM.updateBlock(addr, (uint8_t*)src, 6);
		break;
	default:
		break;
	}
}
