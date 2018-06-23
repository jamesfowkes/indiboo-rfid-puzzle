#ifndef _RFID_H_
#define _RFID_H_

#define RFID_SLOT_COUNT 6
#define RFID_HISTORY_COUNT 3

void rfid_setup(bool& rfid_update_flag);
void rfid_tick();
void rfid_debug();

void rfid_print_current_uids();
void rfid_print_current_uid(uint8_t index);
void rfid_print_saved_uid(uint8_t index);

void rfid_save_all_current_uids();
void rfid_save_current_uid(uint8_t slot);

uint8_t rfid_get_matched(bool (&show)[RFID_SLOT_COUNT]);
uint8_t rfid_get_uid_count();

#endif
