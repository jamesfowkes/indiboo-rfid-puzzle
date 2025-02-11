/* C/C++ Includes */
#include <stdint.h>

/* Arduino/AVR Includes */
#include <avr/wdt.h>

/* Application Includes */

#include "uid.h"
#include "rfid.h"
#include "rfid-nv.h"
#include "rfid-store.h"

static const uint8_t NO_FREE_INDEX = 0xFF;

static const uint8_t TOTAL_RFID_COUNT = RFID_SLOT_COUNT * RFID_HISTORY_COUNT;

/* Private Variables */

/* Private Functions */

static uint8_t direct_index(uint8_t slot, uint8_t index) { return (slot * RFID_HISTORY_COUNT) + index; }

static UID& get_uid_local(uint8_t direct_idx)
{
	static UID s_local;
	rfid_nv_get_uid(s_local, direct_idx);
	return s_local;
}

static UID& get_uid_local(uint8_t slot, uint8_t index)
{
	return get_uid_local(direct_index(slot, index));
}

static uint8_t find_free_index(uint8_t slot)
{
	for (uint8_t i=0; i<RFID_HISTORY_COUNT; i++)
	{
		if (!uid_is_valid(get_uid_local(slot, i)))
		{
			return i;
		}
	}
	return NO_FREE_INDEX;
}

static uint8_t find_or_make_free_index(uint8_t slot)
{
	uint8_t free_index = find_free_index(slot);
	if (free_index != NO_FREE_INDEX) { return free_index; }
	
	for (uint8_t index=0; index<RFID_HISTORY_COUNT-1; index++)
	{
		rfid_nv_set_uid(get_uid_local(slot, index+1), direct_index(slot, index));
	}

	rfid_nv_clear_uid(direct_index(slot, RFID_HISTORY_COUNT-1));
	return RFID_HISTORY_COUNT-1;
}

/* Public Functions */

bool rfid_store_check_uid(uint8_t slot, uint8_t index)
{
	return uid_is_valid(get_uid_local(slot,index));
}

uint8_t rfid_store_save_uid(UID& to_save, uint8_t slot)
{
	uint8_t index = 0xFF;
	if(uid_is_valid(to_save) && (rfid_store_match_in_slot(to_save, slot) == NO_MATCH))
	{
		index = find_or_make_free_index(slot);
		rfid_nv_set_uid(to_save, direct_index(slot, index));
	}
	return index;
}

void rfid_store_get_uids(UID (&uids)[RFID_HISTORY_COUNT], uint8_t slot)
{
	for (uint8_t index=0; index<RFID_HISTORY_COUNT; index++)
	{
		rfid_store_get_uid(uids[index], slot, index);
	}
}

bool rfid_store_get_uid(UID& uid, uint8_t slot, uint8_t index)
{
	bool success = uid_is_valid(get_uid_local(slot,index));
	if (success)
	{
		uid_copy(uid, get_uid_local(slot,index));
	}
	else
	{
		uid_clear(uid);
	}
	return success;
}

uint8_t rfid_store_match_in_slot(UID& uid_to_match, uint8_t slot)
{
	uint8_t match = NO_MATCH;
	const uint8_t start = direct_index(slot, 0);
	const uint8_t end = direct_index(slot, RFID_HISTORY_COUNT-1);

	for (uint8_t i=start; i<=end; i++)
	{
		if (uid_is_valid(get_uid_local(i)) && uid_match(get_uid_local(i), uid_to_match))
		{
			match = i - start;
			break;
		}
	}

	return match;
}

bool rfid_store_match_saved(UID& uid_to_match)
{
	for (uint8_t i=0; i<TOTAL_RFID_COUNT; i++)
	{
		if (uid_match(get_uid_local(i), uid_to_match))
		{
			return true;
		}
	}
	return false;
}

bool rfid_store_match_saved(UID& uid_to_match, uint8_t& slot, uint8_t& index)
{
	for (uint8_t i=0; i<TOTAL_RFID_COUNT; i++)
	{
		if (uid_match(get_uid_local(i), uid_to_match))
		{
			slot = i / RFID_HISTORY_COUNT;
			index = i - (slot * RFID_HISTORY_COUNT);
			return true;
		}
	}

	return false;
}

void rfid_store_clear_slot_history(uint8_t slot)
{
	for (uint8_t index=0; index<RFID_HISTORY_COUNT; index++)
	{
		rfid_nv_clear_uid(direct_index(slot, index));
	}
}

void rfid_store_clear_all()
{
	for (uint8_t i=0; i<TOTAL_RFID_COUNT; i++)
	{
		wdt_reset();
		rfid_nv_clear_uid(i);
	}
}
