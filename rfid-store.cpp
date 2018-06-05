/* C/C++ Includes */

#include <stdint.h>

/* Application Includes */

#include "uid.h"
#include "rfid.h"
#include "rfid-nv.h"
#include "rfid-store.h"

static const uint8_t NO_FREE_INDEX = 0xFF;
static const uint8_t NO_MATCH = 0xFF;

static const uint8_t TOTAL_RFID_COUNT = RFID_SLOT_COUNT * RFID_HISTORY_COUNT;

/* Private Variables */

/* Private Functions */

static uint8_t _slot(uint8_t total_idx) { return total_idx/RFID_HISTORY_COUNT; }
static uint8_t _index(uint8_t total_idx) { return total_idx % RFID_HISTORY_COUNT; }
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
	UID uid;
	for (uint8_t i=0; i<RFID_HISTORY_COUNT; i++)
	{
		if (!uid_is_valid(get_uid_local(direct_index(slot, i))))
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

static uint8_t get_match_index(UID& uid_to_match, uint8_t slot)
{
	for (uint8_t i=0; i<RFID_HISTORY_COUNT; i++)
	{
		if (uid_match(get_uid_local(slot,i), uid_to_match))
		{
			return i;
		}
	}

	return NO_MATCH;
}

/* Public Functions */

bool rfid_store_check_uid(uint8_t slot, uint8_t index)
{
	return uid_is_valid(get_uid_local(slot,index));
}

void rfid_store_save_uid(UID& to_save, uint8_t slot)
{
	if(uid_is_valid(to_save))
	{
		uint8_t index = find_or_make_free_index(slot);
		rfid_nv_set_uid(to_save, direct_index(slot, index));
	}
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
	return success;
}

bool rfid_store_match_in_slot(UID& uid_to_match, uint8_t slot)
{
	bool any_match = false;
	const uint8_t start = slot * RFID_HISTORY_COUNT;
	const uint8_t end = start + RFID_HISTORY_COUNT;

	for (uint8_t i=start; i<end; i++)
	{
		any_match |= uid_match(get_uid_local(i), uid_to_match);
	}

	return any_match;
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
	uint8_t match_index;
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
		rfid_nv_clear_uid(i);
	}
}
