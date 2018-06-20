#ifndef _RFID_STORE_H_
#define _RFID_STORE_H_

static const uint8_t NO_MATCH = 0xFF;

uint8_t rfid_store_save_uid(UID& to_save, uint8_t slot);
bool rfid_store_get_uid(UID& uid, uint8_t slot, uint8_t index);
void rfid_store_get_uids(UID (&uids)[RFID_HISTORY_COUNT], uint8_t slot);

bool rfid_store_check_uid(uint8_t slot, uint8_t index);

bool rfid_store_match_saved(UID& uid_to_match);
bool rfid_store_match_saved(UID& uid_to_match, uint8_t& slot, uint8_t& index);
uint8_t rfid_store_match_in_slot(UID& uid_to_match, uint8_t slot);

void rfid_store_clear_slot_history(uint8_t slot);
void rfid_store_clear_all();

#endif
