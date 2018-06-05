/* Arduino Library Includes */

#include "EEPROMex.h"

/* Application Includes */

#include "uid.h"
#include "rfid.h"
#include "rfid-nv.h"

static const uint8_t TOTAL_RFID_COUNT = RFID_SLOT_COUNT * RFID_HISTORY_COUNT;


/* Private Variables */

static int EEPROM_ADDRESS_UID[TOTAL_RFID_COUNT];

/*
 * Public Functions
 */

void rfid_nv_setup()
{
	for (uint8_t i=0; i<TOTAL_RFID_COUNT; i++)
	{
		EEPROM_ADDRESS_UID[i] = EEPROM.getAddress(sizeof(UID));
	}
}

void rfid_nv_get_uid(UID& uid, uint8_t direct_index)
{
	EEPROM.readBlock(EEPROM_ADDRESS_UID[direct_index], &uid);
}

void rfid_nv_set_uid(UID& uid, uint8_t direct_idx)
{
	EEPROM.writeBlock(EEPROM_ADDRESS_UID[direct_idx], uid);
}

void rfid_nv_clear_uid(uint8_t direct_idx)
{
	static const UID BLANK_UID = {0,0,0,0,0,0,0,0,0,0,0};
	EEPROM.writeBlock(EEPROM_ADDRESS_UID[direct_idx], BLANK_UID);
}
