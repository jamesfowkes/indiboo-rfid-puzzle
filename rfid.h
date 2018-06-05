#ifndef _RFID_H_
#define _RFID_H_

#define RFID_SLOT_COUNT 8
#define RFID_HISTORY_COUNT 3

void rfid_setup(bool& rfid_update_flag);
void rfid_tick();
void rfid_print_current_uid();
void rfid_print_saved_uid();

void rfid_save_all_current_uids();
void rfid_save_current_uid(uint8_t slot);

#endif
